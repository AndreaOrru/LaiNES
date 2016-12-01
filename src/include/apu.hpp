#pragma once
#include "common.hpp"

namespace APU {


template <bool write> u8 access(int elapsed, u16 addr, u8 v = 0);
void run_frame(int elapsed);
void reset();
void init();


}
