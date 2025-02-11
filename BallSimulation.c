#include </opt/homebrew/include/SDL2/SDL.h>
#include <complex.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define CIRCLE_RADIUS 400
#define FPS 60
#define GRAVITY 200.0f
#define ENERGYKEPT 0.95f
#define MIN_VELOCITY 3.0f
#define BALL_RADIUS 15
// Ball properties
typedef struct {
    float x, y;
    float vx, vy;
} Ball;
void applyCenterForce(Ball *ball, float dt){
	
	float dx = -ball->x;
	float dy = -ball->y;
		
	float dist = sqrt(dx * dx + dy * dy);
	if(dist > 0){
				
		float ax = dx / dist;
		float ay = dy / dist;
	
		ball->vy += 400 * ay * dt;
		ball->vx += 400 * ax * dt;
	}
}
void update_ball(Ball *ball, float dt, Ball balls[],bool forceOn, int ballCount) {
	
	ball->vy += GRAVITY * dt;	
	if(forceOn){
		applyCenterForce(ball, dt);
	}
	ball->x += ball->vx * dt;
    ball->y += ball->vy * dt;

	for(int i = 0; i <= ballCount; i++){
			if(&balls[i] == ball) continue;
			float dx = balls[i].x - ball->x;
			float dy = balls[i].y - ball->y;
			float colDist = sqrt(dx * dx + dy * dy);	
			if(colDist < BALL_RADIUS * 2){
				float colnx = dx / colDist;
				float colny = dy / colDist;
				float colDot = ball->vx * colnx + ball->vy * colny;
				float colOver = (2 * BALL_RADIUS) - colDist;
				ball->x -= colOver * colnx;
				ball->y -=  colOver * colny;
					
				ball->vx -= 1.25 * colDot * colnx;
				ball->vy -= 1.25 * colDot * colny;
				 // Stop small movements when velocity is too low
        		if (fabs(ball->vy) < MIN_VELOCITY) {
            		ball->vy = 0;
        		}
        		if (fabs(ball->vx) < MIN_VELOCITY) {
            		ball->vx = 0;
				}
			}
	}
	
    float dist = sqrt(ball->x * ball->x + ball->y * ball->y);
    if (dist + BALL_RADIUS > CIRCLE_RADIUS) {
        // Normalized normal vector
        float nx = ball->x / dist;
        float ny = ball->y / dist;
    	    
        // Reflect velocity vector
        float dot = ball->vx * nx + ball->vy * ny;
        ball->vx -= 2 * dot * nx;
        ball->vy -= 2 * dot * ny;

		ball->vy *= ENERGYKEPT;
		ball->vx *= ENERGYKEPT;

		float overlap = (dist + BALL_RADIUS) - CIRCLE_RADIUS;
        ball->x -= overlap * nx;
        ball->y -= overlap * ny;
		
        // Stop small movements when velocity is too low
        if (fabs(ball->vy) < MIN_VELOCITY) {
            ball->vy = 0;
        }
        if (fabs(ball->vx) < MIN_VELOCITY) {
            ball->vx = 0;
        }
    }	
}
void draw_circle(SDL_Renderer *renderer, int cx, int cy, int radius) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (int angle = 0; angle < 360; angle++) {
        float rad = angle * M_PI / 180.0f;
        int x = cx + (int)(radius * cos(rad));
        int y = cy + (int)(radius * sin(rad));
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

float sigmoid_v_to_color(float range,float x){
	float k = 0.0001;
	int result = (range/2) * (1 + tanhf(k * x));
	return result;
}
void draw_filled_circle(SDL_Renderer *renderer, int cx, int cy, int radius, Ball *ball) {
	double speed = sqrt(ball->vx * ball->vx + ball->vy * ball->vy);
	double Vmax = 200;
    // Normalize speed to 0-1 range
    double t = (Vmax > 0) ? speed / Vmax : 0;
    if (t > 1) t = 1;  // Clamp to max range

    // Interpolate from red (255, 0, 0) to green (0, 255, 0)
    float r = ((1 - t) * 255);  // Red decreases as speed increases
    float g = (t * 255);        // Green increases as speed increases
    float b = 0;                     // Keep blue at 0

    // Ensure values stay in range
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
	
		
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx * dx + dy * dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}
void render(SDL_Renderer *renderer, Ball balls[], int ballCount) {
 SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);	 
    SDL_RenderClear(renderer);
	
    // Draw circle boundary
    draw_circle(renderer, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, CIRCLE_RADIUS);

    // Draw ball
	for(int i = 0; i < ballCount; i++){

    	draw_filled_circle(renderer, WINDOW_WIDTH / 2 + (int)balls[i].x, WINDOW_HEIGHT / 2 + (int)balls[i].y, BALL_RADIUS, &balls[i]);
	}
    SDL_RenderPresent(renderer);
}


void instantiateBalls(Ball balls[], int ballCount, bool randRadius){
	for(int i = 0; i < ballCount; i++){
		balls[i].x = -400 + 10 * i;
		balls[i].y = -CIRCLE_RADIUS / 2;
	    balls[i].vx = 0;
		balls[i].vy = 10;	
	}

}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Bouncing Ball Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    int ballCount = atoi(argv[1]);
	bool randRadius = false;

	Ball balls[ballCount];	
	instantiateBalls(balls, ballCount, randRadius);

	bool running = true;
    SDL_Event event;
    Uint32 last_time = SDL_GetTicks();
	
	bool forceOn = false;
    while (running) {
      
		   	while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
	   		if(event.type == SDL_KEYDOWN){
				 if(event.key.keysym.sym == SDLK_UP){
					forceOn = true;
				 }
			}
			if(event.type == SDL_KEYUP){
				if(event.key.keysym.sym == SDLK_UP){
					forceOn = false;
				}
			}
		}
        
        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;
		for(int i = 0; i < ballCount; i++){
			update_ball(&balls[i], dt, balls, forceOn, ballCount);
		}	
        render(renderer, balls, ballCount);
		
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

