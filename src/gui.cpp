#include <csignal>
#include <SDL2/SDL.h>
#include "cpu.hpp"
#include "gui.hpp"

namespace GUI {


// Screen size:
const unsigned width  = 256;
const unsigned height = 240;

// SDL structures:
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
u8 const* keys;

// Status:
bool pause = false;

/* Initialize SDL */
void init()
{
    SDL_Init(SDL_INIT_VIDEO);

    window   = SDL_CreateWindow  ("LaiNES",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  width, height, 0);

    renderer = SDL_CreateRenderer(window, -1,
                                  SDL_RENDERER_ACCELERATED);

    texture  = SDL_CreateTexture (renderer,
                                  SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                  width, height);

    keys = SDL_GetKeyboardState(0);
    signal(SIGINT, SIG_DFL);
}

/* Get the joypad state from SDL */
u8 get_joypad_state(int n)
{
    u8 j = 0;
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

/* Send the rendered frame to the GUI */
void new_frame(u32* pixels)
{
    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(u32));
}

/* Actually render the frame */
void render()
{
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

/* Run the emulator */
void run()
{
    SDL_Event e;

    // Framerate control:
    u32 frameStart, frameTime;
    const int fps   = 60;
    const int delay = 1000.0f / fps;

    while(true)
    {
        frameStart = SDL_GetTicks();

        // Handle events:
        while (SDL_PollEvent(&e))
            switch (e.type)
            {
                case SDL_QUIT:  return;
                case SDL_KEYDOWN:
                    if (keys[SDL_SCANCODE_ESCAPE])
                    {
                        pause = not pause;

                        if (pause)
                            SDL_SetTextureColorMod(texture,  48,  48,  48);
                        else
                            SDL_SetTextureColorMod(texture, 255, 255, 255);
                    }
            }

        if (not pause) CPU::run_frame();
        render();

        // Wait to mantain framerate:
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < delay)
            SDL_Delay((int)(delay - frameTime));
    }
}


}
