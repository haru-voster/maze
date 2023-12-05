#include <SDL2/SDL.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TILE_SIZE = 20;
const int FPS = 60;

// Use constants for maze dimensions
#define MAZE_ROWS 10
#define MAZE_COLUMNS 10

int world_map[MAZE_ROWS][MAZE_COLUMNS] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 0, 0, 0, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 1, 1, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
};


typedef struct {
    int x, y;
    short life;  // Not currently used
    char *name;  // Not currently used
} Man;

// Function prototypes
int processEvents(SDL_Window *window, Man *man);
void renderMaze(SDL_Renderer *renderer);
void renderRaycasting(SDL_Renderer *renderer, Man *man);

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_Window *window = SDL_CreateWindow("Maze", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (window == NULL) {
        fprintf(stderr, "Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Man man = {220, 140, 0, NULL};

    // The window is open: enter program loop (see SDL_PollEvent)
    int done = 0;

    // Event loop:
    while (!done) {
        // Check for events
        done = processEvents(window, &man);

        // Render display with raycasting
        renderMaze(renderer);
        renderRaycasting(renderer, &man);

        // Delay to achieve the desired FPS
        SDL_Delay(1000 / FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

int processEvents(SDL_Window *window, Man *man) {
    SDL_Event event;
    int done = 0;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_WINDOWEVENT_CLOSE: {
                if (window) {
                    SDL_DestroyWindow(window);
                    window = NULL;
                    done = 1;
                }
            } break;
            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        done = 1;
                        break;
                }
            } break;
            case SDL_QUIT:
                done = 1;
                break;
        }
    }

    const Uint8 *state = SDL_GetKeyboardState(NULL);

    // Update the man's position based on keyboard input
    if (state[SDL_SCANCODE_LEFT] && world_map[man->y / TILE_SIZE][(man->x - TILE_SIZE) / TILE_SIZE] != 1) {
        man->x -= TILE_SIZE;
    }
    if (state[SDL_SCANCODE_RIGHT] && world_map[man->y / TILE_SIZE][(man->x + TILE_SIZE) / TILE_SIZE] != 1) {
        man->x += TILE_SIZE;
    }
    if (state[SDL_SCANCODE_UP] && world_map[(man->y - TILE_SIZE) / TILE_SIZE][man->x / TILE_SIZE] != 1) {
        man->y -= TILE_SIZE;
    }
    if (state[SDL_SCANCODE_DOWN] && world_map[(man->y + TILE_SIZE) / TILE_SIZE][man->x / TILE_SIZE] != 1) {
        man->y += TILE_SIZE;
    }

    return done;
}

void renderMaze(SDL_Renderer *renderer) {
    // Set the color for drawing to blue
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    // Clear the entire screen to our selected color
    SDL_RenderClear(renderer);

    // Set drawing color to white
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    // Render the map
    for (int i = 0; i < MAZE_ROWS; ++i) {
        for (int j = 0; j < MAZE_COLUMNS; ++j) {
            if (world_map[i][j] == 1) {
                SDL_Rect wall = {j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                SDL_RenderFillRect(renderer, &wall);
            }
        }
    }
}

void renderRaycasting(SDL_Renderer *renderer, Man *man) {
    // Raycasting parameters
    double angle = atan2(1, 0); // Direction the player is facing (horizontal)
    double FOV = M_PI / 4.0;    // Field of view

    // Cast rays
    for (int ray = 0; ray < SCREEN_WIDTH; ++ray) {
        // Calculate ray direction
        double rayDirX = cos(angle - FOV / 2.0 + ray * FOV / SCREEN_WIDTH);
        double rayDirY = sin(angle - FOV / 2.0 + ray * FOV / SCREEN_WIDTH);

        // Initial positions
        double posX = man->x;
        double posY = man->y;

        // Step increments for ray
        double stepX = TILE_SIZE * rayDirX / SCREEN_WIDTH;
        double stepY = TILE_SIZE * rayDirY / SCREEN_WIDTH;

        // Ray casting loop
        int hit = 0;
        while (!hit) {
            posX += stepX;
            posY += stepY;

            // Check for collision with walls
            int mapX = (int)(posX / TILE_SIZE);
            int mapY = (int)(posY / TILE_SIZE);

            if (world_map[mapY][mapX] == 1) {
                hit = 1;
            }
        }

        // Calculate wall height based on distance to the wall
        double distance = sqrt((posX - man->x) * (posX - man->x) + (posY - man->y) * (posY - man->y));
        int wallHeight = SCREEN_HEIGHT / distance;

        // Draw the column on the screen
        SDL_Rect column = {ray, SCREEN_HEIGHT / 2 - wallHeight / 2, 1, wallHeight};
        SDL_RenderFillRect(renderer, &column);
    }
    // Render the man
    SDL_Rect rect = {man->x, man->y, TILE_SIZE, TILE_SIZE};
    SDL_RenderFillRect(renderer, &rect);

    // Done drawing, "present" or show to screen what we've drawn
    SDL_RenderPresent(renderer);
}
