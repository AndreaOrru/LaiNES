#include <csignal>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include "Sound_Queue.h"
#include "apu.hpp"
#include "cartridge.hpp"
#include "cpu.hpp"
#include "menu.hpp"
#include "gui.hpp"
#include "config.hpp"
#include "savestate.hpp"

namespace GUI {

// SDL structures:
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* gameTexture;
SDL_Texture* background;
TTF_Font* font;
u8 const* keys;
Sound_Queue* soundQueue;
SDL_GameController* controller[] = { nullptr, nullptr };

// Menus:
Menu* menu;
Menu* mainMenu;
Menu* settingsMenu;
Menu* videoMenu;
Menu* keyboardMenu[2];
Menu* joystickMenu[2];
FileMenu* fileMenu;

bool pause = true;
bool fast_forward = false;
const int FAST_FORWARD_MULTIPLIER = 8;

/* Set the window size multiplier */
void set_size(int mul)
{
    last_window_size = mul;
    SDL_SetWindowSize(window, WIDTH * mul, HEIGHT * mul);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

/* Toggle fullscreen mode */
void set_fullscreen(bool enabled)
{
    fullscreen_mode = enabled;
    SDL_SetWindowFullscreen(window, enabled ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
}

/* Set scaling/filtering mode */
void set_scaling_mode(bool smooth)
{
    scaling_mode = smooth;
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, smooth ? "1" : "0");

    // Recreate game texture for the hint to take effect
    SDL_DestroyTexture(gameTexture);
    gameTexture = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                    WIDTH, HEIGHT);

    // Recreate background texture
    SDL_DestroyTexture(background);
    SDL_Surface* backSurface = IMG_Load("res/init.png");
    background = SDL_CreateTextureFromSurface(renderer, backSurface);
    SDL_SetTextureColorMod(background, 60, 60, 60);
    SDL_FreeSurface(backSurface);

    // Recreate all menu entry textures
    auto recreate_menu = [](Menu* m) {
        if (m) {
            for (auto entry : m->entries) {
                entry->set_label(entry->get_label());
            }
        }
    };

    recreate_menu(mainMenu);
    recreate_menu(settingsMenu);
    recreate_menu(videoMenu);
    recreate_menu(keyboardMenu[0]);
    recreate_menu(keyboardMenu[1]);
    if (controller[0]) recreate_menu(joystickMenu[0]);
    if (controller[1]) recreate_menu(joystickMenu[1]);
    recreate_menu(fileMenu);
}

/* Toggle aspect ratio stretching */
void set_aspect_stretch(bool enabled)
{
    stretch_aspect = enabled;
    // Only applies in fullscreen mode
    if (fullscreen_mode)
    {
        if (enabled)
        {
            // Disable logical size to stretch to fill screen
            SDL_RenderSetLogicalSize(renderer, 0, 0);
        }
        else
        {
            // Enable logical size for letterboxing
            SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
        }
    }
}

/* Initialize GUI */
void init()
{
    // Initialize graphics system:
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scaling_mode ? "1" : "0");

    // Set window class for Wayland/X11 to match .desktop file
    SDL_SetHint("SDL_VIDEO_WAYLAND_WMCLASS", "laines");
    SDL_SetHint("SDL_VIDEO_X11_WMCLASS", "laines");

    TTF_Init();

    for (int i = 0, ci = 0; i < SDL_NumJoysticks() && ci < 2; i++)
        if (SDL_IsGameController(i))
            controller[ci++] = SDL_GameControllerOpen(i);

    APU::init();
    soundQueue = new Sound_Queue;
    soundQueue->init(96000);

    // Initialize graphics structures:
    window      = SDL_CreateWindow  ("LaiNES",
                                     SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                     WIDTH * last_window_size, HEIGHT * last_window_size, 0);

    // Set window icon:
    SDL_Surface* icon = IMG_Load("res/laines_icon.png");
    if (icon) {
        SDL_SetWindowIcon(window, icon);
        SDL_FreeSurface(icon);
    }

    renderer    = SDL_CreateRenderer(window, -1,
                                     SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    gameTexture = SDL_CreateTexture (renderer,
                                     SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
                                     WIDTH, HEIGHT);

    // Apply saved video settings
    if (fullscreen_mode)
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        // Apply stretch if enabled in fullscreen
        if (stretch_aspect)
            SDL_RenderSetLogicalSize(renderer, 0, 0);
    }

    font = TTF_OpenFont("res/font.ttf", FONT_SZ);
    keys = SDL_GetKeyboardState(0);

    // Initial background:
    SDL_Surface* backSurface  = IMG_Load("res/init.png");
    background = SDL_CreateTextureFromSurface(renderer, backSurface);
    SDL_SetTextureColorMod(background, 60, 60, 60);
    SDL_FreeSurface(backSurface);

    // Menus:
    mainMenu = new Menu;
    mainMenu->add(new Entry("Load ROM", []{ menu = fileMenu; }));
    mainMenu->add(new Entry("Reset",    []{ Cartridge::reset(); toggle_pause(); }, []{ return Cartridge::loaded(); }));
    mainMenu->add(new Entry("Save State", []{
        std::string filename = SaveState::get_default_filename();
        if (!filename.empty()) {
            SaveState::save(filename.c_str());
        }
        toggle_pause();
    }, []{ return Cartridge::loaded(); }));
    mainMenu->add(new Entry("Load State", []{
        std::string filename = SaveState::get_default_filename();
        if (!filename.empty()) {
            SaveState::load(filename.c_str());
        }
        toggle_pause();
    }, []{ return Cartridge::loaded(); }));
    mainMenu->add(new Entry("Settings", []{ menu = settingsMenu; }));
    mainMenu->add(new Entry("Exit",     []{ exit(0); }));

    settingsMenu = new Menu;
    settingsMenu->add(new Entry("<",            []{ menu = mainMenu; }));
    settingsMenu->add(new Entry("Video",        []{ menu = videoMenu; }));
    settingsMenu->add(new Entry("Controller 1", []{ menu = useJoystick[0] ? joystickMenu[0] : keyboardMenu[0]; }));
    settingsMenu->add(new Entry("Controller 2", []{ menu = useJoystick[1] ? joystickMenu[1] : keyboardMenu[1]; }));
    settingsMenu->add(new Entry("Save Settings", []{ save_settings(); menu = mainMenu; }));

    videoMenu = new Menu;
    videoMenu->add(new Entry("<",       []{ menu = settingsMenu; }));
    videoMenu->add(new CycleEntry("Size: ", &last_window_size, 1, 6, [](int v){ set_size(v); }, "x", []{ return !fullscreen_mode; }));
    videoMenu->add(new ToggleEntry("Fullscreen: ", &fullscreen_mode, [](bool v){ set_fullscreen(v); }));
    videoMenu->add(new ToggleEntry("Smooth Scaling: ", &scaling_mode, [](bool v){ set_scaling_mode(v); }));
    videoMenu->add(new ToggleEntry("Stretch Aspect: ", &stretch_aspect, [](bool v){ set_aspect_stretch(v); }, []{ return fullscreen_mode; }));

    for (int i = 0; i < 2; i++)
    {
        keyboardMenu[i] = new Menu;
        keyboardMenu[i]->add(new Entry("<", []{ menu = settingsMenu; }));
        if (controller[i] != nullptr)
            keyboardMenu[i]->add(new Entry("Joystick >", [=]{ menu = joystickMenu[i]; useJoystick[i] = true; }));
        keyboardMenu[i]->add(new ControlEntry("Up",     &KEY_UP[i]));
        keyboardMenu[i]->add(new ControlEntry("Down",   &KEY_DOWN[i]));
        keyboardMenu[i]->add(new ControlEntry("Left",   &KEY_LEFT[i]));
        keyboardMenu[i]->add(new ControlEntry("Right",  &KEY_RIGHT[i]));
        keyboardMenu[i]->add(new ControlEntry("A",      &KEY_A[i]));
        keyboardMenu[i]->add(new ControlEntry("B",      &KEY_B[i]));
        keyboardMenu[i]->add(new ControlEntry("Start",  &KEY_START[i]));
        keyboardMenu[i]->add(new ControlEntry("Select", &KEY_SELECT[i]));

        if (controller[i] != nullptr)
        {
            joystickMenu[i] = new Menu;
            joystickMenu[i]->add(new Entry("<", []{ menu = settingsMenu; }));
            joystickMenu[i]->add(new Entry("< Keyboard", [=]{ menu = keyboardMenu[i]; useJoystick[i] = false; }));
            joystickMenu[i]->add(new ControlEntry("Up",     &BTN_UP[i]));
            joystickMenu[i]->add(new ControlEntry("Down",   &BTN_DOWN[i]));
            joystickMenu[i]->add(new ControlEntry("Left",   &BTN_LEFT[i]));
            joystickMenu[i]->add(new ControlEntry("Right",  &BTN_RIGHT[i]));
            joystickMenu[i]->add(new ControlEntry("A",      &BTN_A[i]));
            joystickMenu[i]->add(new ControlEntry("B",      &BTN_B[i]));
            joystickMenu[i]->add(new ControlEntry("Start",  &BTN_START[i]));
            joystickMenu[i]->add(new ControlEntry("Select", &BTN_SELECT[i]));
        }
    }

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
        dest.x = WIDTH/2 - dest.w/2;
    else if (x == TEXT_RIGHT)
        dest.x = WIDTH - dest.w - 10;
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
    const int DEAD_ZONE = 8000;

    u8 j = 0;
    
    // Get normal controller input first
    if (useJoystick[n] && controller[n])
    {
        j |= (SDL_GameControllerGetButton(controller[n], (SDL_GameControllerButton)BTN_A[n]))      << 0;  // A.
        j |= (SDL_GameControllerGetButton(controller[n], (SDL_GameControllerButton)BTN_B[n]))      << 1;  // B.
        j |= (SDL_GameControllerGetButton(controller[n], (SDL_GameControllerButton)BTN_SELECT[n])) << 2;  // Select.
        j |= (SDL_GameControllerGetButton(controller[n], (SDL_GameControllerButton)BTN_START[n]))  << 3;  // Start.

        j |= (SDL_GameControllerGetButton(controller[n], (SDL_GameControllerButton)BTN_UP[n]))     << 4;  // Up.
        j |= (SDL_GameControllerGetAxis(controller[n], SDL_CONTROLLER_AXIS_LEFTY) < -DEAD_ZONE)    << 4;
        j |= (SDL_GameControllerGetButton(controller[n], (SDL_GameControllerButton)BTN_DOWN[n]))   << 5;  // Down.
        j |= (SDL_GameControllerGetAxis(controller[n], SDL_CONTROLLER_AXIS_LEFTY) >  DEAD_ZONE)    << 5;
        j |= (SDL_GameControllerGetButton(controller[n], (SDL_GameControllerButton)BTN_LEFT[n]))   << 6;  // Left.
        j |= (SDL_GameControllerGetAxis(controller[n], SDL_CONTROLLER_AXIS_LEFTX) < -DEAD_ZONE)    << 6;
        j |= (SDL_GameControllerGetButton(controller[n], (SDL_GameControllerButton)BTN_RIGHT[n]))  << 7;  // Right.
        j |= (SDL_GameControllerGetAxis(controller[n], SDL_CONTROLLER_AXIS_LEFTX) >  DEAD_ZONE)    << 7;
    }
    else
    {
        j |= (keys[KEY_A[n]])      << 0;
        j |= (keys[KEY_B[n]])      << 1;
        j |= (keys[KEY_SELECT[n]]) << 2;
        j |= (keys[KEY_START[n]])  << 3;
        j |= (keys[KEY_UP[n]])     << 4;
        j |= (keys[KEY_DOWN[n]])   << 5;
        j |= (keys[KEY_LEFT[n]])   << 6;
        j |= (keys[KEY_RIGHT[n]])  << 7;
    }
    
    return j;
}

/* Send the rendered frame to the GUI */
void new_frame(u32* pixels)
{
    SDL_UpdateTexture(gameTexture, NULL, pixels, WIDTH * sizeof(u32));
}

void new_samples(const blip_sample_t* samples, size_t count)
{
    // Skip audio during fast forward to prevent buffer blocking
    if (!fast_forward) {
        soundQueue->write(samples, count);
    }
}

bool is_fast_forward()
{
    return fast_forward;
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
    if (pause) {
        // If stretch aspect is enabled, temporarily restore logical sizing
        // so the menu scales properly
        if (fullscreen_mode && stretch_aspect) {
            SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);
        }

        menu->render();

        // Restore stretch mode if it was enabled
        if (fullscreen_mode && stretch_aspect) {
            SDL_RenderSetLogicalSize(renderer, 0, 0);
        }
    }

    // Draw fast forward indicator:
    if (fast_forward && !pause) {
        SDL_Texture* ff_indicator = gen_text(">>", { 255, 255, 0 });
        render_texture(ff_indicator, TEXT_RIGHT, 0);
        SDL_DestroyTexture(ff_indicator);
    }

    SDL_RenderPresent(renderer);
}

/* Play/stop the game */
void toggle_pause()
{
    pause = not pause;
    menu  = mainMenu;

    // Disable fast forward when pausing
    if (pause)
        fast_forward = false;

    if (pause)
        SDL_SetTextureColorMod(gameTexture,  60,  60,  60);
    else
        SDL_SetTextureColorMod(gameTexture, 255, 255, 255);
}

/* Set pause state directly */
void set_paused(bool paused)
{
    pause = paused;
    menu  = paused ? mainMenu : nullptr;

    // Disable fast forward when pausing
    if (pause)
        fast_forward = false;

    if (pause)
        SDL_SetTextureColorMod(gameTexture,  60,  60,  60);
    else
        SDL_SetTextureColorMod(gameTexture, 255, 255, 255);
}

/* Prompt for a key, return the scancode */
SDL_Scancode query_key()
{
    SDL_Texture* prompt = gen_text("Press a key...", { 255, 255, 255 });
    render_texture(prompt, TEXT_CENTER, HEIGHT - FONT_SZ*4);
    SDL_RenderPresent(renderer);

    SDL_Event e;
    while (true)
    {
        SDL_PollEvent(&e);
        if (e.type == SDL_KEYDOWN)
            return e.key.keysym.scancode;
    }
}

int query_button()
{
    SDL_Texture* prompt = gen_text("Press a button...", { 255, 255, 255 });
    render_texture(prompt, TEXT_CENTER, HEIGHT - FONT_SZ*4);
    SDL_RenderPresent(renderer);

    SDL_Event e;
    while (true)
    {
        SDL_PollEvent(&e);
        if (e.type == SDL_CONTROLLERBUTTONDOWN)
            return e.cbutton.button;
    }
}

/* Run the emulator */
void run()
{
    SDL_Event e;

    // Framerate control:
    u32 frameStart, frameTime;
    const int FPS   = 60;
    const int DELAY = 1000.0f / FPS;

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

        // Fast forward: hold Tab to enable
        fast_forward = keys[SDL_SCANCODE_TAB] and Cartridge::loaded() and not pause;

        if (not pause) {
            // Run multiple frames when fast forwarding
            int frames_to_run = fast_forward ? FAST_FORWARD_MULTIPLIER : 1;
            for (int i = 0; i < frames_to_run; i++) {
                CPU::run_frame();
            }
        }

        // Always render to maintain smooth 60 FPS visuals
        render();

        // Wait to maintain framerate (skip when fast forwarding):
        if (!fast_forward) {
            frameTime = SDL_GetTicks() - frameStart;
            if (frameTime < DELAY)
                SDL_Delay((int)(DELAY - frameTime));
        }
    }
}


}
