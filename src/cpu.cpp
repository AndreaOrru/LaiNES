#include <cstdio>
#include <cstdlib>
#include "cpu.hpp"

/* CPU state */
u8 mem[0x10000];
u8 A, X, Y, S;
u16 PC;
Flags P;

/* Cycle counting */
u64 cycles;
#define T   tick()
inline void tick() { cycles++; }

/* Flags updating */
inline void upd_cv(u8 x, u8 y, s16 r) { P.c = (r>0xFF); P.v = ~(x^y) & (x^r) & 0x80; }
inline void upd_nz(u8 x)              { P.n = x & 0x80; P.z = (x == 0);              }
// Does adding I to A cross a page?
inline bool cross(u16 a, u8 i) { return ((a+i) & 0xFF00) != ((a & 0xFF00)); }

/* Memory access */
inline u8  wr(u16 a, u8 v)      { T; return mem[a] = v;        }
inline u8  rd(u16 a)            { T; return mem[a];            }
inline u16 rd16_d(u16 a, u16 b) { return rd(a) | (rd(b) << 8); }  // Read from A and B and merge.
inline u16 rd16(u16 a)          { return rd16_d(a, a+1);       }
inline u8  push(u8 v)           { return wr(0x100 | (S--), v); }
inline u8  pop()                { return rd(0x100 | (++S));    }

/* Addressing modes */
inline u16 imm()   { return PC++;                                          }
inline u16 imm16() { PC += 2; return PC - 2;                               }
inline u16 abs()   { return rd16(imm16());                                 }
inline u16 _abx()  { T; return abs() + X;                                  }  // Exception.
inline u16 abx()   { u16 a = abs(); if (cross(a, X)) { T; }; return a + X; }
inline u16 aby()   { u16 a = abs(); if (cross(a, Y)) { T; }; return a + Y; }
inline u16 zp()    { return rd(imm());                                     }
inline u16 zpx()   { T; return (zp() + X) % 0x100;                         }
inline u16 zpy()   { T; return (zp() + Y) % 0x100;                         }
inline u16 izx()   { u8 i = zpx(); return rd16_d(i, (i+1) % 0x100);        }
inline u16 _izy()  { u8 i = zp();  return rd16_d(i, (i+1) % 0x100) + Y;    }  // Exception.
inline u16 izy()   { u16 a = _izy(); if (cross(a-Y, Y)) { T; }; return a;  }

/* STx */
template<u8& r, Mode m> void st()        { wr(   m()    , r);    }
template<>              void st<A,izy>() { wr(_izy()    , A); T; }  // Exceptions.
template<>              void st<A,abx>() { wr( abs() + X, A); T; }  // ...
template<>              void st<A,aby>() { wr( abs() + Y, A); T; }  // ...

#define G  u16 a = m(); u8 p = rd(a)  /* Fetch parameter */
template<u8& r, Mode m> void ld()  { G; upd_nz(r = p);                 }  // LDx
template<u8& r, Mode m> void cmp() { G; upd_nz(r - p); P.c = (r >= p); }  // CMP, CPx
/* Arithmetic and bitwise */
template<Mode m> void ADC() { G       ; s16 r = A + p + P.c; upd_cv(A, p, r); upd_nz(A = r); }
template<Mode m> void SBC() { G ^ 0xFF; s16 r = A + p + P.c; upd_cv(A, p, r); upd_nz(A = r); }
template<Mode m> void BIT() { G; P.z = !(A & p); P.n = p & 0x80; P.v = p & 0x40; }
template<Mode m> void AND() { G; upd_nz(A &= p); }
template<Mode m> void EOR() { G; upd_nz(A ^= p); }
template<Mode m> void ORA() { G; upd_nz(A |= p); }
/* Read-Modify-Write */
template<Mode m> void ASL() { G; P.c = p & 0x80; upd_nz(wr(a, p << 1)); T; }
template<Mode m> void LSR() { G; P.c = p & 0x01; upd_nz(wr(a, p >> 1)); T; }
template<Mode m> void ROL() { G; u8 c = P.c     ; P.c = p & 0x80; upd_nz(wr(a, (p << 1) | c) ); T; }
template<Mode m> void ROR() { G; u8 c = P.c << 7; P.c = p & 0x01; upd_nz(wr(a, c | (p >> 1)) ); T; }
template<Mode m> void DEC() { G; upd_nz(wr(a, --p)); T; }
template<Mode m> void INC() { G; upd_nz(wr(a, ++p)); T; }
#undef G

/* DEx, INx */
template<u8& r> void dec() { upd_nz(--r); T; }
template<u8& r> void inc() { upd_nz(++r); T; }
/* Bit shifting on the accumulator */
void ASL_A() { P.c = A & 0x80; upd_nz(A <<= 1); T; }
void LSR_A() { P.c = A & 0x01; upd_nz(A >>= 1); T; }
void ROL_A() { u8 c = P.c     ; P.c = A & 0x80; upd_nz(A = ((A << 1) | c) ); T; }
void ROR_A() { u8 c = P.c << 7; P.c = A & 0x01; upd_nz(A = (c | (A >> 1)) ); T; }

/* Txx (move values between registers) */
template<u8& s, u8& d> void tr()      { upd_nz(d = s); T; }
template<>             void tr<X,S>() { S = X;         T; }  // TSX, exception.

/* Stack operations */
void PLP() { P.reg = (pop() & 0b11001111) | (P.reg & 0b00110000); T; T; }
void PHP() { push(P.reg | (1 << 4)); T; }
void PLA() { A = pop(); upd_nz(A); T; T; }
void PHA() { push(A); T; }

/* Flow control (branches, jumps) */
template<u8 f, bool v> void br() { s8 j = rd(imm()); if (P.get(f) == v) { PC += j; T; } }
void JMP_IND() { u16 i = rd16(imm16()); PC = rd16_d(i, (i&0xFF00) | ((i+1) % 0x100)); }
void JMP()     { PC = rd16(imm16()); }
void JSR()     { u16 t = PC+1; T; push(t >> 8); push(t); PC = rd16(imm16()); }

/* Return instructions */
void RTS() {        PC = (pop() | (pop() << 8)) + 1; T; T; T; }
void RTI() { PLP(); PC =  pop() | (pop() << 8);               }

template<u8 f, bool v> void flag() { P.set(f, v); T; }  // Clear and set flags.
void NOP() { T;       }
void BRK() { exit(0); }


/* Execute a CPU instruction */
void step() 
{
    switch (rd(PC++))  // Fetch the opcode.
    {
        // Select the right function to emulate the instruction:
        case 0x00: return BRK()       ;  case 0x01: return ORA<izx>()  ;
        case 0x05: return ORA<zp>()   ;  case 0x06: return ASL<zp>()   ;
        case 0x08: return PHP()       ;  case 0x09: return ORA<imm>()  ;
        case 0x0A: return ASL_A()     ;  case 0x0D: return ORA<abs>()  ;
        case 0x0E: return ASL<abs>()  ;  case 0x10: return br<N,0>()   ;
        case 0x11: return ORA<izy>()  ;  case 0x15: return ORA<zpx>()  ;
        case 0x16: return ASL<zpx>()  ;  case 0x18: return flag<C,0>() ;
        case 0x19: return ORA<aby>()  ;  case 0x1D: return ORA<abx>()  ;
        case 0x1E: return ASL<_abx>() ;  case 0x20: return JSR()       ;
        case 0x21: return AND<izx>()  ;  case 0x24: return BIT<zp>()   ;
        case 0x25: return AND<zp>()   ;  case 0x26: return ROL<zp>()   ;
        case 0x28: return PLP()       ;  case 0x29: return AND<imm>()  ;
        case 0x2A: return ROL_A()     ;  case 0x2C: return BIT<abs>()  ;
        case 0x2D: return AND<abs>()  ;  case 0x2E: return ROL<abs>()  ;
        case 0x30: return br<N,1>()   ;  case 0x31: return AND<izy>()  ;
        case 0x35: return AND<zpx>()  ;  case 0x36: return ROL<zpx>()  ;
        case 0x38: return flag<C,1>() ;  case 0x39: return AND<aby>()  ;
        case 0x3D: return AND<abx>()  ;  case 0x3E: return ROL<_abx>() ;
        case 0x40: return RTI()       ;  case 0x41: return EOR<izx>()  ;
        case 0x45: return EOR<zp>()   ;  case 0x46: return LSR<zp>()   ;
        case 0x48: return PHA()       ;  case 0x49: return EOR<imm>()  ;
        case 0x4A: return LSR_A()     ;  case 0x4C: return JMP()       ;
        case 0x4D: return EOR<abs>()  ;  case 0x4E: return LSR<abs>()  ;
        case 0x50: return br<V,0>()   ;  case 0x51: return EOR<izy>()  ;
        case 0x55: return EOR<zpx>()  ;  case 0x56: return LSR<zpx>()  ;
        case 0x58: return flag<I,0>() ;  case 0x59: return EOR<aby>()  ;
        case 0x5D: return EOR<abx>()  ;  case 0x5E: return LSR<_abx>() ;
        case 0x60: return RTS()       ;  case 0x61: return ADC<izx>()  ;
        case 0x65: return ADC<zp>()   ;  case 0x66: return ROR<zp>()   ;
        case 0x68: return PLA()       ;  case 0x69: return ADC<imm>()  ;
        case 0x6A: return ROR_A()     ;  case 0x6C: return JMP_IND()   ;
        case 0x6D: return ADC<abs>()  ;  case 0x6E: return ROR<abs>()  ;
        case 0x70: return br<V,1>()   ;  case 0x71: return ADC<izy>()  ;
        case 0x75: return ADC<zpx>()  ;  case 0x76: return ROR<zpx>()  ;
        case 0x78: return flag<I,1>() ;  case 0x79: return ADC<aby>()  ;
        case 0x7D: return ADC<abx>()  ;  case 0x7E: return ROR<_abx>() ;
        case 0x81: return st<A,izx>() ;  case 0x84: return st<Y,zp>()  ;
        case 0x85: return st<A,zp>()  ;  case 0x86: return st<X,zp>()  ;
        case 0x88: return dec<Y>()    ;  case 0x8A: return tr<X,A>()   ;
        case 0x8C: return st<Y,abs>() ;  case 0x8D: return st<A,abs>() ;
        case 0x8E: return st<X,abs>() ;  case 0x90: return br<C,0>()   ;
        case 0x91: return st<A,izy>() ;  case 0x94: return st<Y,zpx>() ;
        case 0x95: return st<A,zpx>() ;  case 0x96: return st<X,zpy>() ;
        case 0x98: return tr<Y,A>()   ;  case 0x99: return st<A,aby>() ;
        case 0x9A: return tr<X,S>()   ;  case 0x9D: return st<A,abx>() ;
        case 0xA0: return ld<Y,imm>() ;  case 0xA1: return ld<A,izx>() ; 
        case 0xA2: return ld<X,imm>() ;  case 0xA4: return ld<Y,zp>()  ;
        case 0xA5: return ld<A,zp>()  ;  case 0xA6: return ld<X,zp>()  ;
        case 0xA8: return tr<A,Y>()   ;  case 0xA9: return ld<A,imm>() ;
        case 0xAA: return tr<A,X>()   ;  case 0xAC: return ld<Y,abs>() ;
        case 0xAD: return ld<A,abs>() ;  case 0xAE: return ld<X,abs>() ;
        case 0xB0: return br<C,1>()   ;  case 0xB1: return ld<A,izy>() ;
        case 0xB4: return ld<Y,zpx>() ;  case 0xB5: return ld<A,zpx>() ;
        case 0xB6: return ld<X,zpy>() ;  case 0xB8: return flag<V,0>() ;
        case 0xB9: return ld<A,aby>() ;  case 0xBA: return tr<S,X>()   ;
        case 0xBC: return ld<Y,abx>() ;  case 0xBD: return ld<A,abx>() ;
        case 0xBE: return ld<X,aby>() ;  case 0xC0: return cmp<Y,imm>();
        case 0xC1: return cmp<A,izx>();  case 0xC4: return cmp<Y,zp>() ;
        case 0xC5: return cmp<A,zp>() ;  case 0xC6: return DEC<zp>()   ;
        case 0xC8: return inc<Y>()    ;  case 0xC9: return cmp<A,imm>();
        case 0xCA: return dec<X>()    ;  case 0xCC: return cmp<Y,abs>();
        case 0xCD: return cmp<A,abs>();  case 0xCE: return DEC<abs>()  ;
        case 0xD0: return br<Z,0>()   ;  case 0xD1: return cmp<A,izy>();
        case 0xD5: return cmp<A,zpx>();  case 0xD6: return DEC<zpx>()  ;
        case 0xD8: return flag<D,0>() ;  case 0xD9: return cmp<A,aby>();
        case 0xDD: return cmp<A,abx>();  case 0xDE: return DEC<_abx>() ;
        case 0xE0: return cmp<X,imm>();  case 0xE1: return SBC<izx>()  ;
        case 0xE4: return cmp<X,zp>() ;  case 0xE5: return SBC<zp>()   ;
        case 0xE6: return INC<zp>()   ;  case 0xE8: return inc<X>()    ;
        case 0xE9: return SBC<imm>()  ;  case 0xEA: return NOP()       ;
        case 0xEC: return cmp<X,abs>();  case 0xED: return SBC<abs>()  ;
        case 0xEE: return INC<abs>()  ;  case 0xF0: return br<Z,1>()   ;
        case 0xF1: return SBC<izy>()  ;  case 0xF5: return SBC<zpx>()  ;
        case 0xF6: return INC<zpx>()  ;  case 0xF8: return flag<D,1>() ;
        case 0xF9: return SBC<aby>()  ;  case 0xFD: return SBC<abx>()  ;
        case 0xFE: return INC<_abx>() ;  default:   return exit(1)     ;
    }
}

/* Reset the CPU */
void reset()
{
    A = X = Y = 0x00;
    P.reg = 0b00110100;
    PC = 0xC000;
    S = 0xFD;
}

/* Entry point */
int main(int argc, char const *argv[])
{
    reset();  // Set the initial state.

    /* Read the ROM */
    FILE* f = fopen(argv[1], "rb");
    fseek(f, 0, SEEK_END);
    int s = ftell(f);
    fseek(f, 16, SEEK_SET);
    fread(mem + 0xC000, 0x4000, 1, f);

    /* Emulate forever */
    u64 old_clock = 0;
    while(1)
    {
        printf("[PC: $%.4x] - | A: $%.2x | X: $%.2x | Y: $%.2x | P: $%.2x | S: $%.2x | CYC: %lu\n", PC, A, X, Y, P.reg, S, (clock - old_clock) * 3);
        fflush(stdout);
        
        old_clock = clock;
        step();
    }


    return 0;
}
