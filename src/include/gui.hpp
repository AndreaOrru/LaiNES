#pragma once
#include <SDL2/SDL.h>
#include <string>
#include <Nes_Apu.h>
#include "common.hpp"

namespace GUI {


const int TEXT_CENTER  = -1;
const int TEXT_RIGHT   = -2;
const unsigned FONT_SZ = 15;

void init();
void toggle_pause();
SDL_Scancode query_key();
int query_button();
void run();

SDL_Texture* gen_text(std::string text, SDL_Color color);
void render_texture(SDL_Texture* texture, int x, int y);

u8 get_joypad_state(int n);
void new_frame(u32* pixels);
void new_samples(const blip_sample_t* samples, size_t count);
void set_size(int mul);

}
