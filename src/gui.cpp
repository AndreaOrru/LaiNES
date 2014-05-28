#include <csignal>
#include <SDL2/SDL_ttf.h>
#include "cartridge.hpp"
#include "cpu.hpp"
#include "menu.hpp"
#include "gui.hpp"

namespace GUI {


// SDL structures:
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* gameTexture;
TTF_Font* font;
u8 const* keys;

// Menus:
Menu* menu;
Menu mainMenu;
Menu settingsMenu;
FileMenu* fileMenu;

bool pause = true;

/* Initialize GUI */
void init()
{
    // Initialize graphics system:
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    // Initialize graphics structures:
    window      = SDL_CreateWindow  ("LaiNES",
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     width, height, 0);

    renderer    = SDL_CreateRenderer(window, -1,
                                     SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    gameTexture = SDL_CreateTexture (renderer,
                                     SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                     width, height);

    font        = TTF_OpenFont("res/font.ttf", fontPt);

    keys = SDL_GetKeyboardState(0);
    signal(SIGINT, SIG_DFL);  // CTRL+C kills the application.

    // Menus:
    mainMenu.add("Load ROM", []{ menu = fileMenu->reset(); });
    mainMenu.add("Settings", []{ menu = settingsMenu.reset(); });
    mainMenu.add("Exit"    , []{ exit(0); });
    settingsMenu.add("<",    []{ menu = mainMenu.reset(); });
    settingsMenu.add("Controls");
    settingsMenu.add("Video");
    fileMenu = new FileMenu;
    menu = &mainMenu;
}

/* Render a texture on screen (-1 to center on an axis) */
void render_texture(SDL_Texture* texture, int x, int y)
{
    int w, h;
    SDL_Rect dest;

    SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
    dest.x = (x < 0) ? ((width  / 2) - (dest.w / 2)) : x;
    dest.y = (y < 0) ? ((height / 2) - (dest.h / 2)) : y;
    SDL_RenderCopy(renderer, texture, NULL, &dest);
}

/* Generate a texture from text */
SDL_Texture* gen_text(std::string text, SDL_Color color)
{
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_FreeSurface(surface);
    return texture;
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
    SDL_UpdateTexture(gameTexture, NULL, pixels, width * sizeof(u32));
}

/* Render the screen */
void render()
{
    SDL_RenderClear(renderer);

    // Draw the NES screen:
    if (pause)
        SDL_SetTextureColorMod(gameTexture,  40,  40,  40);
    else
        SDL_SetTextureColorMod(gameTexture, 255, 255, 255);
    SDL_RenderCopy(renderer, gameTexture, NULL, NULL);

    // Draw the menu:
    if (pause) menu->render();

    SDL_RenderPresent(renderer);
}

/* Play/stop the game */
void toggle_pause()
{
    pause = not pause;
    menu  = mainMenu.reset();
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
                case SDL_QUIT: return;
                case SDL_KEYDOWN:
                    if (keys[SDL_SCANCODE_ESCAPE] and Cartridge::loaded())
                        toggle_pause();
                    else if (pause)
                        menu->update(keys);
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
