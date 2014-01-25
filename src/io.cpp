#include <csignal>
#include <SDL2/SDL.h>
#include "io.hpp"

namespace IO {

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
u32 pixels[256 * 240];

void init()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    SDL_CreateWindowAndRenderer(256, 240, 0, &window, &renderer);
    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                256, 240);

    signal(SIGINT, SIG_DFL);
}

void draw_pixel(unsigned x, unsigned y, u32 rgb)
{
    pixels[y*256 + x] = rgb;
}

void flush_screen()
{
    SDL_UpdateTexture(texture, NULL, pixels, 256 * sizeof (u32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}


}
