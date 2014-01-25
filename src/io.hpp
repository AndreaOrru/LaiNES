#pragma once
#include "common.hpp"

namespace IO {


void init();
void draw_pixel(unsigned x, unsigned y, u32 rgb);
void flush_screen();


}
