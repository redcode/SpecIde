#pragma once

/** Z80OrRegX.h
 *
 * Instruction: OR rx
 *
 */

bool z80OrRegX()
{
    switch (executionStep)
    {
        case 0:
            memRdCycles = 0;
            memWrCycles = 0;
            memAddrMode = 0x00000000;

            // Calculate the result.
            acc.l = acc.h = af.h | *regx8[z];
            acc.h ^= acc.h >> 1;
            acc.h ^= acc.h >> 2;
            acc.h ^= acc.h >> 4;
            af.l = (acc.h & 0x01) ? 0x00 : FLAG_PV;   // ...0.P00
            af.l |=
                acc.l & (FLAG_S | FLAG_5 | FLAG_3);      // S.503P00
            af.l |= (acc.l) ? 0x00 : FLAG_Z;          // SZ503P00
            af.h = acc.l;
            prefix = PREFIX_NO;
            return true;

        default:    // Should not happen
            assert(false);
            return true;
    }
}

// vim: et:sw=4:ts=4
