#pragma once

/** Z80PrefixFD.h
 *
 * Prefix FD.
 *
 */

#include "Z80Instruction.h"
#include "Z80RegisterSet.h"

class Z80PrefixFD : public Z80Instruction
{
    public:
        Z80PrefixFD() {}

        bool operator()(Z80RegisterSet* r)
        {
            switch (r->executionStep)
            {
                case 0:
                    r->memAddrMode = 0x00000000;
                    r->prefix = PREFIX_FD;
                    return true;

                default:    // Should not happen
                    assert(false);
                    return true;
            }
        }
};

// vim: et:sw=4:ts=4
