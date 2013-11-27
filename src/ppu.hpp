#ifndef PPU_HPP
#define PPU_HPP
namespace PPU {


union Status
{
    struct
    {
        unsigned bus    : 5;
        unsigned sprOvf : 1;
        unsigned sprHit : 1;
        unsigned vBlank : 1;
    };
    u8 r;
};

union Ctrl
{
    struct
    {
        unsigned nt     : 2;
        unsigned incr   : 1;
        unsigned sprTbl : 1;
        unsigned bgTbl  : 1;
        unsigned sprSz  : 1;
        unsigned slave  : 1;
        unsigned nmi    : 1;
    };
    u8 r;
};

union Mask
{
    struct
    {
        unsigned gray    : 1;
        unsigned bgLeft  : 1;
        unsigned sprLeft : 1;
        unsigned bg      : 1;
        unsigned spr     : 1;
        unsigned red     : 1;
        unsigned green   : 1;
        unsigned blue    : 1;
    };
    u8 r;
};

union Addr
{
    struct
    {
        unsigned cX : 5;
        unsigned cY : 5;
        unsigned nt : 2;
        unsigned fY : 3;
    };
    struct
    {
        unsigned l : 8;
        unsigned h : 8;
    };
    unsigned addr : 12;
    unsigned r : 15;
};


}
#endif  // PPU_HPP
