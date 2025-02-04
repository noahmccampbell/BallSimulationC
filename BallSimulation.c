#include </opt/homebrew/include/SDL2/SDL.h>
#include <math.h>
#include <stdbool.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define BALL_RADIUS 10
#define CIRCLE_RADIUS 400
#define FPS 60
#define GRAVITY 200.0f
#define ENERGYKEPT 0.95f
#define MIN_VELOCITY 5.0f

// Ball properties
typedef struct {
    float x, y;
    float vx, vy;
} Ball;

void update_ball(Ball *ball, float dt, Ball balls[], int ballCount) {
	
	ball->vy += GRAVITY * dt;	
	ball->x += ball->vx * dt;
    ball->y += ball->vy * dt;

	for(int i = 0; i <= ballCount; i++){
			if(&balls[i] == ball) continue;
			float dx = balls[i].x - ball->x;
			float dy = balls[i].y - ball->y;
			float colDist = sqrt(dx * dx + dy * dy);	
			if(colDist < BALL_RADIUS*2){
				float colnx = dx / colDist;
				float colny = dy / colDist;
				float colDot = ball->vx * colnx + ball->vy * colny;
				float colOver = 2 * BALL_RADIUS - colDist;
				ball->x -= colOver * colnx;
				ball->y -=  colOver * colny;
					
				ball->vx -= 1.8 * colDot * colnx;
				ball->vy -= 1.8 * colDot * colny;
				 // Stop small movements when velocity is too low
        		//if (fabs(ball->vy) < MIN_VELOCITY) {
            	//	ball->vy = 0;
        		//}
        		//if (fabs(ball->vx) < MIN_VELOCITY) {
            	//	ball->vx = 0;
				//}
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

void draw_filled_circle(SDL_Renderer *renderer, int cx, int cy, int radius) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
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

    	draw_filled_circle(renderer, WINDOW_WIDTH / 2 + (int)balls[i].x, WINDOW_HEIGHT / 2 + (int)balls[i].y, BALL_RADIUS);
	}
    SDL_RenderPresent(renderer);
}


void instantiateBalls(Ball balls[], int ballCount){
	for(int i = 0; i < ballCount; i++){
		balls[i].x = -400 + 10 * i;
		balls[i].y = -CIRCLE_RADIUS / 2;
	    balls[i].vx = 0;
		balls[i].vy = 10;	
	}

}

int main(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Bouncing Ball Simulation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    int ballCount = 100;
	Ball balls[ballCount]; 
	instantiateBalls(balls, ballCount);

	bool running = true;
    SDL_Event event;
    Uint32 last_time = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
	   	}
        
        Uint32 current_time = SDL_GetTicks();
        float dt = (current_time - last_time) / 1000.0f;
        last_time = current_time;
		for(int i = 0; i < ballCount; i++){
			update_ball(&balls[i], dt, balls, ballCount);
		}	
        render(renderer, balls, ballCount);
		
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

