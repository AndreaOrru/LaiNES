#pragma once
#include "common.hpp"

namespace GUI {


void init();
u8 get_joypad_state(int n);
void new_frame(u32* pixels);
void run();


}
