#include "gui.hpp"
#include "cpu.hpp"
#include "apu.hpp"

namespace APU {


Nes_Apu apu;
Blip_Buffer buf;

const int OUT_SIZE = 4096;
blip_sample_t outBuf[OUT_SIZE];

void init()
{
    buf.sample_rate(96000);
    buf.clock_rate(1789773);

    apu.output(&buf);
    apu.dmc_reader(CPU::dmc_read);
}

void reset()
{
    apu.reset();
    buf.clear();
}

template <bool write> u8 access(int elapsed, u16 addr, u8 v)
{
    if (write)
        apu.write_register(elapsed, addr, v);
    else if (addr == apu.status_addr)
        v = apu.read_status(elapsed);

    return v;
}
template u8 access<0>(int, u16, u8); template u8 access<1>(int, u16, u8);

void run_frame(int elapsed)
{
    apu.end_frame(elapsed);
    buf.end_frame(elapsed);

    if (buf.samples_avail() >= OUT_SIZE)
        GUI::new_samples(outBuf, buf.read_samples(outBuf, OUT_SIZE));
}


}
