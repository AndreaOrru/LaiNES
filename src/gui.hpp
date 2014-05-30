#pragma once
#include <SDL2/SDL.h>
#include <string>
#include "common.hpp"

namespace GUI {


const unsigned fontSz = 15;

void init();
void toggle_pause();
void run();

SDL_Texture* gen_text(std::string text, SDL_Color color);
void render_texture(SDL_Texture* texture, int x, int y);

u8 get_joypad_state(int n);
void new_frame(u32* pixels);


}
