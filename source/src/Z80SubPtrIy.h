#pragma once

/** Z80SubPtrIy.h
 *
 * Instruction: SUB (IY+d)
 *
 */

bool z80SubPtrIy()
{
    switch (executionStep)
    {
        case 0:
            memRdCycles = 1;
            memAddrMode = 0x00000071;
            return true;

        case 1:
            cpuProcCycles = 1;
            return true;

        case 2:
            tmp.l = iReg.h;
            return false;

        case 3:
            tmp.h = ((tmp.l & 0x80) == 0x80) ? 0xFF : 0x00;
            return false;

        case 4:
            tmp.w += iy.w;
            return false;

        case 5:
            return false;

        case 6:
            memRdCycles = 1;
            return true;

        case 7:
            tmp.l = iReg.h;
            acc.w = af.h - tmp.l;

            af.l = acc.l & (FLAG_S | FLAG_5 | FLAG_3);
            af.l |= FLAG_N;
            af.l |= (acc.l ^ tmp.l ^ af.h) & FLAG_H;
            af.l |= (((acc.l ^ tmp.l ^ af.h) >> 5) 
                    ^ (acc.h << 2)) & FLAG_PV;
            af.l |= acc.h & FLAG_C;                   // S.5H3V0C
            af.l |= (acc.l) ? 0x00 : FLAG_Z;          // SZ5H3V0C
            af.h = acc.l;
            prefix = PREFIX_NO;
            return true;

        default:    // Should not happen
            assert(false);
            return true;
    }
}

// vim: et:sw=4:ts=4
