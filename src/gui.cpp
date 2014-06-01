#include <csignal>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "Sound_Queue.h"
#include "apu.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "menu.hpp"
#include "gui.hpp"

namespace GUI {


// Screen size:
const unsigned width  = 256;
const unsigned height = 240;

// SDL structures:
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* gameTexture;
SDL_Texture* background;
TTF_Font* font;
u8 const* keys;
Sound_Queue* soundQueue;

// Menus:
Menu* menu;
Menu* mainMenu;
Menu* settingsMenu;
Menu* videoMenu;
Menu* controlMenu;
FileMenu* fileMenu;

bool pause = true;

// Controls settings:
SDL_Scancode CTRL_A      = SDL_SCANCODE_A;
SDL_Scancode CTRL_B      = SDL_SCANCODE_S;
SDL_Scancode CTRL_SELECT = SDL_SCANCODE_SPACE;
SDL_Scancode CTRL_START  = SDL_SCANCODE_RETURN;
SDL_Scancode CTRL_UP     = SDL_SCANCODE_UP;
SDL_Scancode CTRL_DOWN   = SDL_SCANCODE_DOWN;
SDL_Scancode CTRL_LEFT   = SDL_SCANCODE_LEFT;
SDL_Scancode CTRL_RIGHT  = SDL_SCANCODE_RIGHT;

/* Set the window size multiplier */
void set_size(int mul)
{
    SDL_SetWindowSize(window, width * mul, height * mul);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

/* Initialize GUI */
void init()
{
    // Initialize graphics system:
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    TTF_Init();

    APU::init();
    soundQueue = new Sound_Queue;
    soundQueue->init(44100);

    // Initialize graphics structures:
    window      = SDL_CreateWindow  ("LaiNES",
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     width, height, 0);

    renderer    = SDL_CreateRenderer(window, -1,
                                     SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, width, height);

    gameTexture = SDL_CreateTexture (renderer,
                                     SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                     width, height);

    font = TTF_OpenFont("res/font.ttf", fontSz);
    keys = SDL_GetKeyboardState(0);

    // Initial background:
    SDL_Surface* backSurface  = IMG_Load("res/init.png");
    background = SDL_CreateTextureFromSurface(renderer, backSurface);
    SDL_SetTextureColorMod(background, 60, 60, 60);
    SDL_FreeSurface(backSurface);

    // Menus:
    mainMenu = new Menu;
    mainMenu->add(new Entry("Load ROM", []{ menu = fileMenu; }));
    mainMenu->add(new Entry("Settings", []{ menu = settingsMenu; }));
    mainMenu->add(new Entry("Exit",     []{ exit(0); }));

    settingsMenu = new Menu;
    settingsMenu->add(new Entry("<",        []{ menu = mainMenu; }));
    settingsMenu->add(new Entry("Video",    []{ menu = videoMenu; }));
    settingsMenu->add(new Entry("Controls", []{ menu = controlMenu; }));

    videoMenu = new Menu;
    videoMenu->add(new Entry("<",       []{ menu = settingsMenu; }));
    videoMenu->add(new Entry("Size 1x", []{ set_size(1); }));
    videoMenu->add(new Entry("Size 2x", []{ set_size(2); }));
    videoMenu->add(new Entry("Size 3x", []{ set_size(3); }));

    controlMenu = new Menu;
    controlMenu->add(new Entry("<", []{ menu = settingsMenu; }));
    controlMenu->add(new ControlEntry("Up",     &CTRL_UP));
    controlMenu->add(new ControlEntry("Down",   &CTRL_DOWN));
    controlMenu->add(new ControlEntry("Left",   &CTRL_LEFT));
    controlMenu->add(new ControlEntry("Right",  &CTRL_RIGHT));
    controlMenu->add(new ControlEntry("A",      &CTRL_A));
    controlMenu->add(new ControlEntry("B",      &CTRL_B));
    controlMenu->add(new ControlEntry("Start",  &CTRL_START));
    controlMenu->add(new ControlEntry("Select", &CTRL_SELECT));

    fileMenu = new FileMenu;

    menu = mainMenu;
}

/* Render a texture on screen */
void render_texture(SDL_Texture* texture, int x, int y)
{
    int w, h;
    SDL_Rect dest;

    SDL_QueryTexture(texture, NULL, NULL, &dest.w, &dest.h);
    if (x == TEXT_CENTER)
        dest.x = width/2 - dest.w/2;
    else if (x == TEXT_RIGHT)
        dest.x = width - dest.w - 10;
    else
        dest.x = x + 10;
    dest.y = y + 5;

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
        j |= (keys[CTRL_A])      << 0;
        j |= (keys[CTRL_B])      << 1;
        j |= (keys[CTRL_SELECT]) << 2;
        j |= (keys[CTRL_START])  << 3;
        j |= (keys[CTRL_UP])     << 4;
        j |= (keys[CTRL_DOWN])   << 5;
        j |= (keys[CTRL_LEFT])   << 6;
        j |= (keys[CTRL_RIGHT])  << 7;
    }
    return j;
}

/* Send the rendered frame to the GUI */
void new_frame(u32* pixels)
{
    SDL_UpdateTexture(gameTexture, NULL, pixels, width * sizeof(u32));
}

void new_samples(const blip_sample_t* samples, size_t count)
{
    soundQueue->write(samples, count);
}

/* Render the screen */
void render()
{
    SDL_RenderClear(renderer);

    // Draw the NES screen:
    if (Cartridge::loaded())
        SDL_RenderCopy(renderer, gameTexture, NULL, NULL);
    else
        SDL_RenderCopy(renderer, background, NULL, NULL);

    // Draw the menu:
    if (pause) menu->render();

    SDL_RenderPresent(renderer);
}

/* Play/stop the game */
void toggle_pause()
{
    pause = not pause;
    menu  = mainMenu;

    if (pause)
        SDL_SetTextureColorMod(gameTexture,  40,  40,  40);
    else
        SDL_SetTextureColorMod(gameTexture, 255, 255, 255);
}

/* Prompt for a key, return the scancode */
SDL_Scancode query_key()
{
    SDL_Texture* prompt = gen_text("Press a key...", { 255, 255, 255 });
    render_texture(prompt, TEXT_CENTER, height - fontSz*4);
    SDL_RenderPresent(renderer);

    SDL_Event e;
    while (true)
    {
        SDL_PollEvent(&e);
        if (e.type == SDL_KEYDOWN)
            return e.key.keysym.scancode;
    }
}

/* Run the emulator */
void run()
{
    SDL_Event e;

    // Framerate control:
    u32 frameStart, frameTime;
    const int fps   = 60;
    const int delay = 1000.0f / fps;

    while (true)
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
