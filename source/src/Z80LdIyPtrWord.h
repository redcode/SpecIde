#pragma once

/** Z80LdIyPtrWord.h
 *
 * Instruction: LD IY, (nn)
 *
 * Encoding: 11 111 101  00 101 010
 * M Cycles: 5 (FD, OCF, MRL, MRH, MRL, MRH)
 * T States: 20
 *
 */

#include "Z80Instruction.h"
#include "Z80RegisterSet.h"

class Z80LdIyPtrWord : public Z80Instruction
{
    public:
        Z80LdIyPtrWord() {}

        bool operator()(Z80RegisterSet* r)
        {
            switch (r->executionStep)
            {
                case 0:
                    r->memRdCycles = 4;
                    r->memAddrMode = 0x00009811;
                    return true;

                case 1:
                    return true;

                case 2:
                    return true;

                case 3:
                    return true;

                case 4:
                    r->iy.w = r->iReg.w;
                    r->prefix = PREFIX_NO;
                    return true;

                default:    // Should not happen
                    assert(false);
                    return true;
            }
        }
};

// vim: et:sw=4:ts=4
