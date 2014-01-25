#include <csignal>
#include <SDL2/SDL.h>
#include "io.hpp"

namespace IO {

const unsigned width  = 256;
const unsigned height = 240;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
u32 pixels[width * height];

void init()
{
    SDL_Init(SDL_INIT_VIDEO);

    window   = SDL_CreateWindow  ("LaiNES",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  width, height, 0);

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    texture  = SDL_CreateTexture (renderer,
                                  SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                  width, height);

    signal(SIGINT, SIG_DFL);
}

void draw_pixel(unsigned x, unsigned y, u32 rgb)
{
    pixels[y*width + x] = rgb;
}

void flush_screen()
{
    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof (u32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}


}
