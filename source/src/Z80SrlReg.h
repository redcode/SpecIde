#pragma once

/** Z80SrlReg.h
 *
 * Instruction: SRL r
 *
 * Encoding: 11 001 011  00 111 rrr
 * M Cycles: 2 (OCF, OCF)
 * T States: 8
 *
 *  r  rrr
 * --- ---
 *  B  000
 *  C  001
 *  D  010
 *  E  011
 *  H  100
 *  L  101
 *  A  111
 *
 * Flags: SZ503P0C
 * - 0 is shifted into MSB.
 *
 */

#include "Z80Instruction.h"
#include "Z80RegisterSet.h"

class Z80SrlReg : public Z80Instruction
{
    public:
        Z80SrlReg() {}

        bool operator()(Z80RegisterSet* r)
        {
            switch (r->executionStep)
            {
                case 0:
                    r->memAddrMode = 0x00000000;

                    r->acc.w = *r->reg8[r->z];
                    r->af.l = r->acc.l & FLAG_C;
                    r->acc.w >>= 1;
                    *r->reg8[r->z] = r->acc.h = r->acc.l;
                    r->acc.h ^= r->acc.h >> 1;
                    r->acc.h ^= r->acc.h >> 2;
                    r->acc.h ^= r->acc.h >> 4;
                    r->af.l |= r->acc.l & (FLAG_S | FLAG_5 | FLAG_3);
                    r->af.l |= (r->acc.l) ? 0x00 : FLAG_Z;
                    r->af.l |= (r->acc.h & 0x01) ? 0x00 : FLAG_PV;
                    r->prefix = PREFIX_NO;
                    return true;

                default:    // Should not happen
                    assert(false);
                    return true;
            }
        }
};

// vim: et:sw=4:ts=4
