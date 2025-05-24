#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdlib.h>

#define SIZE 25
#define CELL_SIZE 20

enum Cell_state {
    EMPTY,
    SNAKE,
    APPLE,
};

enum Game_state {
    RUNNING,
    GAMEOVER,
    WIN,
    EXIT,
};

const int SCREEN_WIDTH = SIZE * CELL_SIZE;
const int SCREEN_HEIGHT = SIZE * CELL_SIZE;
const Uint32 TARGET_FRAME_TIME = 300;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

void Draw_apple(int x_center, int y_center) {
    float radius = 8;
    float distance;
    SDL_SetRenderDrawColor(renderer, 128, 128, 0, 1);
    SDL_RenderDrawLine(renderer, x_center, y_center - radius, x_center + 2, y_center - radius * 1.5);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 1);
    for(float y = y_center - radius; y <= y_center + radius; y++) {
        for(float x = x_center - radius; x <= x_center + radius; x++) {
            distance = hypotf(x - x_center, y - y_center);
            if(distance <= radius) {
                SDL_RenderDrawPointF(renderer, x, y);
            }
        }
    }
}

void Draw_snake_segment(int x_center, int y_center) {
    SDL_Rect rect = {
        .x = x_center - CELL_SIZE * 0.75 * 0.5,
        .y = y_center - CELL_SIZE * 0.75 * 0.5,
        .w = CELL_SIZE * 0.75,
        .h = CELL_SIZE * 0.75,
    };
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 1);
    SDL_RenderFillRect(renderer, &rect);
}

void Generate_apple(enum Cell_state map[SIZE][SIZE], int snake_segments) {
    int apple_rand = rand()%(SIZE * SIZE - snake_segments);
    int apple_new_index = - 1;
    while(apple_rand >= 0) {
        if(map[0][apple_new_index + 1] == EMPTY) {
            apple_rand--;
            apple_new_index++;
        } else {
            apple_new_index++;
        }
    }
    map[0][apple_new_index] = APPLE;
}

struct Snake_segment {
    int x;
    int y;
    struct Snake_segment* next;
    struct Snake_segment* prev;
};

int main(int argc, char* argv[]) {
    srand(time(NULL));
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Не удалось инициализировать SDL, %s\n", SDL_GetError());
    }
    window = SDL_CreateWindow("Snake", 500, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(window == NULL) {
        printf("Ошибка, не удалось создать окно, %s\n", SDL_GetError());
    }
    renderer = SDL_CreateRenderer(window, -1, 0);
    if(renderer == NULL) {
        printf("Ошибка, не удалось создать рендерер, %s\n", SDL_GetError());
    }
    if(TTF_Init() != 0) {
        printf("Ошибка, не удалось инициализировать шрифт %s\n", TTF_GetError());
    }
    struct Snake_segment head = {
        .x = SIZE * 0.5,
        .y = SIZE * 0.5,
        .next = NULL,
        .prev = NULL,
    };
    struct Snake_segment* tail = &head;
    int snake_direction = 4;
    int snake_segments = 1;
    enum Game_state game_state = RUNNING;
    enum Cell_state map[SIZE][SIZE];
    for(int i = 0; i < SIZE * SIZE; i++) {
        map[0][i] = EMPTY;
    }
    map[head.y][head.x] = SNAKE;
    Generate_apple(map, snake_segments);
    SDL_Rect rect = {
        .x = 0,
        .y = 0,
        .w = CELL_SIZE,
        .h = CELL_SIZE,
    };
    TTF_Font* font = TTF_OpenFont("terminus-ttf-4.49.3/TerminusTTF-4.49.3.ttf", 12);
    SDL_Color gameover_color = {15, 15, 15};
    SDL_Surface* gameover_surface = TTF_RenderText_Solid(font, "Game Over", gameover_color);
    SDL_Texture* gameover_texture = SDL_CreateTextureFromSurface(renderer, gameover_surface);
    SDL_Rect gameover_rect = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 150, 75};
    gameover_rect.x -= gameover_rect.w / 2;
    gameover_rect.y -= gameover_rect.h / 2;
    SDL_Surface* gameover_hint_surface = TTF_RenderText_Solid(font, "Press ESC to quit", gameover_color);
    SDL_Texture* gameover_hint_texture = SDL_CreateTextureFromSurface(renderer, gameover_hint_surface);
    SDL_Rect gameover_hint_rect = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + gameover_rect.h / 2 + 20, 120, 30};
    gameover_hint_rect.x -= gameover_hint_rect.w / 2;
    gameover_hint_rect.y -= gameover_hint_rect.h / 2;
    while(game_state == RUNNING) {
        Uint32 start_time = SDL_GetTicks();
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                game_state = EXIT;
            }
            if(event.type == SDL_KEYDOWN && !event.key.repeat) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: game_state = EXIT; break;
                    case SDLK_RIGHT: if(snake_direction != 2 || snake_segments < 2) {snake_direction = 0;} break;
                    case SDLK_DOWN: if(snake_direction != 3 || snake_segments < 2) {snake_direction = 1;} break;
                    case SDLK_LEFT: if(snake_direction != 0 || snake_segments < 2) {snake_direction = 2;} break;
                    case SDLK_UP: if(snake_direction != 1 || snake_segments < 2) {snake_direction = 3;} break;
                }
            }
        }
        int old_x = tail->x;
        int old_y = tail->y;
        struct Snake_segment* segment = tail;
        while(segment->prev != NULL) {
            segment->x = segment->prev->x;
            segment->y = segment->prev->y;
            segment = segment->prev;
        }
        map[old_y][old_x] = EMPTY;
        switch (snake_direction) {
            case 0: head.x++; break;
            case 1: head.y++; break;
            case 2: head.x--; break;
            case 3: head.y--; break;
        }
        if(head.x == SIZE) {
            head.x = 0;
        } else if(head.x == - 1) {
            head.x = SIZE - 1;
        } else if(head.y == SIZE) {
            head.y = 0;
        } else if(head.y == - 1) {
            head.y = SIZE - 1;
        }
        if(map[head.y][head.x] == APPLE) {
            tail->next = malloc(sizeof(struct Snake_segment));
            tail->next->prev = tail;
            tail = tail->next;
            tail->x = old_x;
            tail->y = old_y;
            tail->next = NULL;
            snake_segments++;
            map[old_y][old_x] = SNAKE;
            map[head.y][head.x] = SNAKE;
            if(snake_segments == SIZE * SIZE) {
                game_state = WIN;
            } else {
                Generate_apple(map, snake_segments);
            }
        } else if(map[head.y][head.x] == SNAKE) {
            game_state = GAMEOVER;
        } else {
            map[head.y][head.x] = SNAKE;
        }
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 1);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 1);
        for(int y = 0; y < SIZE; y++) {
            for(int x = 0; x < SIZE; x++) {
                rect.x = x * rect.w;
                rect.y = y * rect.h;
                SDL_RenderDrawRect(renderer, &rect);
                switch (map[y][x]) {
                    case SNAKE:
                        Draw_snake_segment(x * rect.w + rect.w / 2, y * rect.h + rect.h / 2);
                        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 1);
                        break;
                    case APPLE:
                        Draw_apple(x * rect.w + rect.w / 2, y * rect.h + rect.h / 2);
                        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 1);
                        break;
                }
            }
        }
        if(game_state == GAMEOVER) {
            SDL_RenderCopy(renderer, gameover_texture, NULL, &gameover_rect);
            SDL_RenderCopy(renderer, gameover_hint_texture, NULL, &gameover_hint_rect);
        }
        SDL_RenderPresent(renderer);
        switch (game_state) {
            case RUNNING:
                Uint32 end_time = SDL_GetTicks();
                Uint32 frame_time = end_time - start_time;
                if(frame_time < TARGET_FRAME_TIME) {
                    SDL_Delay(TARGET_FRAME_TIME - frame_time);
                }
                break;
            case GAMEOVER:
                while(1) {
                    SDL_WaitEvent(&event);
                    if(event.type == SDL_KEYDOWN) {
                        if(event.key.keysym.sym == SDLK_ESCAPE) {
                            break;
                        }
                    }
                }
                break;
        }
    }
    return 0;
}
