#pragma once

/** Z80AndByte.h
 *
 * Instruction: AND n
 *
 */

bool z80AndByte()
{
    switch (executionStep)
    {
        case 0:
            memRdCycles = 1;
            memWrCycles = 0;
            memAddrMode = 0x00000001;
            return true;

        case 1:
            // Calculate the result.
            acc.l = acc.h = af.h & iReg.h;
            acc.h ^= acc.h >> 1;
            acc.h ^= acc.h >> 2;
            acc.h ^= acc.h >> 4;
            af.l = (acc.h & 0x01) 
                ? FLAG_H : FLAG_H | FLAG_PV;                // ...H.P00
            af.l |=
                acc.l & (FLAG_S | FLAG_5 | FLAG_3);      // S.5H3P00
            af.l |= (acc.l) ? 0x00 : FLAG_Z;          // SZ5H3P00
            af.h = acc.l;
            prefix = PREFIX_NO;
            return true;

        default:    // Should not happen
            assert(false);
            return true;
    }
}

// vim: et:sw=4:ts=4
