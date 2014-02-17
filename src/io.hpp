#pragma once
#include "common.hpp"

namespace IO {


void init();
u8 read_joypad(int n);
void write_joypad_strobe(bool v);
void draw_pixel(unsigned x, unsigned y, u32 rgb);
void flush_screen();


}
