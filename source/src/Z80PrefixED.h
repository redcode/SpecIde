#pragma once

/** Z80PrefixED.h
 *
 * Prefix ED.
 *
 */

#include "Z80Instruction.h"
#include "Z80RegisterSet.h"

class Z80PrefixED : public Z80Instruction
{
    public:
        Z80PrefixED() {}

        bool operator()(Z80RegisterSet* r)
        {
            switch (r->executionStep)
            {
                case 0:
                    r->memAddrMode = 0x00000000;
                    r->prefix = PREFIX_ED;
                    return true;

                default:    // Should not happen
                    assert(false);
                    return true;
            }
        }
};

// vim: et:sw=4:ts=4
