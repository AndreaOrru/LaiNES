#pragma once
#include "common.hpp"

namespace GUI {


void init();
u8 get_joypad_state(int n);
void draw_pixel(unsigned x, unsigned y, u32 rgb);
void new_frame(u32* pixels);


}
