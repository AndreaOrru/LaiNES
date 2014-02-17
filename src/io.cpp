#include <csignal>
#include <SDL2/SDL.h>
#include "io.hpp"

namespace IO {

// Screen size:
const unsigned width  = 256;
const unsigned height = 240;

// SDL structures:
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
u8 const* keys;

u32 pixels[width * height];  // Video buffer.
u8 joypad_bits[2];  // Joypad shift registers.
bool strobe;        // Joypad strobe latch.

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

    keys = SDL_GetKeyboardState(0);
    signal(SIGINT, SIG_DFL);
}

u8 get_joypad_state(int n)
{
    u8 j = 0;
    SDL_PumpEvents();

    if (n == 0)
    {
        j |= (keys[SDL_SCANCODE_A])      << 0;  // A.
        j |= (keys[SDL_SCANCODE_S])      << 1;  // B.
        j |= (keys[SDL_SCANCODE_SPACE])  << 2;  // Select.
        j |= (keys[SDL_SCANCODE_RETURN]) << 3;  // Start.
        j |= (keys[SDL_SCANCODE_UP])     << 4;  // Up.
        j |= (keys[SDL_SCANCODE_DOWN])   << 5;  // Down.
        j |= (keys[SDL_SCANCODE_LEFT])   << 6;  // Left.
        j |= (keys[SDL_SCANCODE_RIGHT])  << 7;  // Right.
    }
    return j;
}

u8 read_joypad(int n)
{
    // When strobe is high, it keeps reading A:
    if (strobe)
        return 0x40 | (get_joypad_state(n) & 1);

    // Get the status of a button and shift the register:
    u8 j = 0x40 | (joypad_bits[n] & 1);
    joypad_bits[n] = 0x80 | (joypad_bits[n] >> 1);
    return j;
}

void write_joypad_strobe(bool v)
{
    // Read the joypad data on strobe's transition 1 -> 0.
    if (strobe and !v)
        for (int i = 0; i < 2; i++)
            joypad_bits[i] = get_joypad_state(i);

    strobe = v;
}

void draw_pixel(unsigned x, unsigned y, u32 rgb)
{
    pixels[y*width + x] = rgb;
}

void flush_screen()
{
    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(u32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}


}
