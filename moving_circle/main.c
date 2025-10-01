#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define CIRCLE_RADIUS 20
#define SPEED 200.0f // pixels par seconde
#define FPS 165
#define FPS_HISTORY 30
#define NUM_BONUS 5

//--------------------- Gestion FPS ---------------------//
typedef struct {
    double history[FPS_HISTORY];
    int index;
} FPSCounter;

void FPSCounter_Init(FPSCounter* counter) {
    for (int i = 0; i < FPS_HISTORY; i++) counter->history[i] = 1.0 / FPS; // init à 165 FPS
    counter->index = 0;
}

int FPSCounter_Update(FPSCounter* counter, double deltaTime) {
    counter->history[counter->index] = deltaTime;
    counter->index = (counter->index + 1) % FPS_HISTORY;

    double sum = 0;
    for (int i = 0; i < FPS_HISTORY; i++) sum += counter->history[i];
    double averageDelta = sum / FPS_HISTORY;

    return (int)(0.5 + 1.0 / averageDelta); // arrondi à l'entier
}

//--------------------- Dessin ---------------------//
void drawCircle(SDL_Renderer* renderer, int cx, int cy, int r) {
    for (int w = 0; w < r * 2; w++) {
        for (int h = 0; h < r * 2; h++) {
            int dx = r - w;
            int dy = r - h;
            if ((dx*dx + dy*dy) <= (r*r)) {
                SDL_RenderDrawPoint(renderer, cx + dx, cy + dy);
            }
        }
    }
}

void drawText(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dstRect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dstRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

//--------------------- Carrés bonus ---------------------//
typedef struct {
    float x, y;
    int size;
    int collected;
} BonusSquare;

BonusSquare bonus[NUM_BONUS];

void initBonusSquares() {
    for (int i = 0; i < NUM_BONUS; i++) {
        bonus[i].size = 30;
        bonus[i].collected = 0;
        bonus[i].x = rand() % (WINDOW_WIDTH - bonus[i].size);
        bonus[i].y = rand() % (WINDOW_HEIGHT - bonus[i].size);
    }
}

int checkCollision(float cx, float cy, int radius, BonusSquare* b) {
    float closestX = (cx < b->x) ? b->x : (cx > b->x + b->size) ? b->x + b->size : cx;
    float closestY = (cy < b->y) ? b->y : (cy > b->y + b->size) ? b->y + b->size : cy;

    float dx = cx - closestX;
    float dy = cy - closestY;

    return (dx*dx + dy*dy) < (radius*radius);
}

//--------------------- Boucle principale ---------------------//
void main_menu(SDL_Window* window, SDL_Renderer* renderer) {
    float x = WINDOW_WIDTH / 2.0f;
    float y = WINDOW_HEIGHT / 2.0f;
    int score = 0;

    int running = 1;
    SDL_Event event;

    if (TTF_Init() != 0) {
        printf("Erreur TTF: %s\n", TTF_GetError());
        return;
    }

    TTF_Font* font = TTF_OpenFont("pokemon_BW2.otf", 24);
    if (!font) {
        printf("Erreur chargement police: %s\n", TTF_GetError());
        TTF_Quit();
        return;
    }

    FPSCounter fpsCounter;
    FPSCounter_Init(&fpsCounter);
    char fpsText[50], scoreText[50];
    const Uint8* keystates;

    srand(SDL_GetTicks());
    initBonusSquares();

    Uint64 lastCounter = SDL_GetPerformanceCounter(), currentCounter;
    double deltaTime;

    while (running) {
        currentCounter = SDL_GetPerformanceCounter();
        deltaTime = (currentCounter - lastCounter) / (double)SDL_GetPerformanceFrequency();
        lastCounter = currentCounter;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }

        // Déplacement
        keystates = SDL_GetKeyboardState(NULL);
        if (keystates[SDL_SCANCODE_LEFT])  x -= SPEED * deltaTime;
        if (keystates[SDL_SCANCODE_RIGHT]) x += SPEED * deltaTime;
        if (keystates[SDL_SCANCODE_UP])    y -= SPEED * deltaTime;
        if (keystates[SDL_SCANCODE_DOWN])  y += SPEED * deltaTime;

        // Limites fenêtre
        if (x < CIRCLE_RADIUS) x = CIRCLE_RADIUS;
        if (x > WINDOW_WIDTH - CIRCLE_RADIUS) x = WINDOW_WIDTH - CIRCLE_RADIUS;
        if (y < CIRCLE_RADIUS) y = CIRCLE_RADIUS;
        if (y > WINDOW_HEIGHT - CIRCLE_RADIUS) y = WINDOW_HEIGHT - CIRCLE_RADIUS;

        // Rendu
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Dessiner les carrés bonus
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (int i = 0; i < NUM_BONUS; i++) {
            if (!bonus[i].collected){
                SDL_Rect rect = { (int)bonus[i].x, (int)bonus[i].y, bonus[i].size, bonus[i].size };
                SDL_RenderFillRect(renderer, &rect);

                if (checkCollision(x, y, CIRCLE_RADIUS, &bonus[i])) {
                    score++;
                        
                    // Apparition d'un carré bonnus
                    bonus[i].size -= 5;
                    if (bonus[i].size <= 0) {
                        bonus[i].collected = 1;
                    } else {
                        bonus[i].x = rand() % (WINDOW_WIDTH - bonus[i].size);
                        bonus[i].y = rand() % (WINDOW_HEIGHT - bonus[i].size);
                    }
                }
            }
        }

        // Dessiner la sphère
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        drawCircle(renderer, (int)x, (int)y, CIRCLE_RADIUS);

        // Afficher FPS
        int fpsAverage = FPSCounter_Update(&fpsCounter, deltaTime);
        snprintf(fpsText, sizeof(fpsText), "FPS: %d", fpsAverage);
        drawText(renderer, font, fpsText, 10, 10);

        // Afficher score
        snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
        drawText(renderer, font, scoreText, 10, 40);

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    TTF_Quit();
}

//--------------------- main ---------------------//
int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Erreur SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Déplace la sphère",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    main_menu(window, renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
