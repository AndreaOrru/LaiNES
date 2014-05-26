#pragma once
#include "common.hpp"

namespace GUI {


void init();
u8 get_keys_state(int n);
void draw_pixel(unsigned x, unsigned y, u32 rgb);
void flush_screen();


}
