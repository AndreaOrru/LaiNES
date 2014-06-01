#pragma once
#include "common.hpp"

namespace PPU {


enum Scanline  { VISIBLE, POST, NMI, PRE };
enum Mirroring { VERTICAL, HORIZONTAL };

/* Sprite buffer */
struct Sprite
{
    u8 id;     // Index in OAM.
    u8 x;      // X position.
    u8 y;      // Y position.
    u8 tile;   // Tile index.
    u8 attr;   // Attributes.
    u8 dataL;  // Tile data (low).
    u8 dataH;  // Tile data (high).
};

/* PPUCTRL ($2000) register */
union Ctrl
{
    struct
    {
        unsigned nt     : 2;  // Nametable ($2000 / $2400 / $2800 / $2C00).
        unsigned incr   : 1;  // Address increment (1 / 32).
        unsigned sprTbl : 1;  // Sprite pattern table ($0000 / $1000).
        unsigned bgTbl  : 1;  // BG pattern table ($0000 / $1000).
        unsigned sprSz  : 1;  // Sprite size (8x8 / 8x16).
        unsigned slave  : 1;  // PPU master/slave.
        unsigned nmi    : 1;  // Enable NMI.
    };
    u8 r;
};

/* PPUMASK ($2001) register */
union Mask
{
    struct
    {
        unsigned gray    : 1;  // Grayscale.
        unsigned bgLeft  : 1;  // Show background in leftmost 8 pixels.
        unsigned sprLeft : 1;  // Show sprite in leftmost 8 pixels.
        unsigned bg      : 1;  // Show background.
        unsigned spr     : 1;  // Show sprites.
        unsigned red     : 1;  // Intensify reds.
        unsigned green   : 1;  // Intensify greens.
        unsigned blue    : 1;  // Intensify blues.
    };
    u8 r;
};

/* PPUSTATUS ($2002) register */
union Status
{
    struct
    {
        unsigned bus    : 5;  // Not significant.
        unsigned sprOvf : 1;  // Sprite overflow.
        unsigned sprHit : 1;  // Sprite 0 Hit.
        unsigned vBlank : 1;  // In VBlank?
    };
    u8 r;
};

/* Loopy's VRAM address */
union Addr
{
    struct
    {
        unsigned cX : 5;  // Coarse X.
        unsigned cY : 5;  // Coarse Y.
        unsigned nt : 2;  // Nametable.
        unsigned fY : 3;  // Fine Y.
    };
    struct
    {
        unsigned l : 8;
        unsigned h : 7;
    };
    unsigned addr : 14;
    unsigned r : 15;
};

template <bool write> u8 access(u16 index, u8 v = 0);
void set_mirroring(Mirroring mode);
void step();
void reset();


}
