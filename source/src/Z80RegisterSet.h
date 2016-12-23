#pragma once

/** Z80RegisterSet
 *
 * Registers and bits in the Z80
 *
 */

#include <cstdint>

#include "Z80Register.h"

using namespace std;

class Z80RegisterSet
{
    public:
        Z80RegisterSet();

        void selectRegisterSet(size_t set);
        void reset();

        size_t registerSet;

        // Registers
        Z80Register ir;
        Z80Register sp;
        Z80Register ix;
        Z80Register iy;

        Z80Register af_pair[2];
        Z80Register bc_pair[2];
        Z80Register de_pair[2];
        Z80Register hl_pair[2];
        Z80Register *af;
        Z80Register *bc;
        Z80Register *de;
        Z80Register *hl;

        uint8_t* r8[2][8];
        uint16_t* rp[2][4];
        uint16_t* rp2[2][4];

        uint8_t** reg8;
        uint16_t** regp;
        uint16_t** regp2;
};

// vim: et:sw=4:ts=4
