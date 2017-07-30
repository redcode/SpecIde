#pragma once

/** Z80CpPtrIx.h
 *
 * Instruction: CP (IX+d)
 *
 */

bool z80CpPtrIx()
{
    switch (executionStep)
    {
        case 0:
            memRdCycles = 1;
            memAddrMode = 0x00000061;
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
            tmp.w += ix.w;
            return false;

        case 5:
            return false;

        case 6:
            memRdCycles = 1;
            return true;

        case 7:
            tmp.l = iReg.h;
            acc.w = af.h - tmp.l;

            // Flags 5 & 3 are copied from the operand.
            af.l = tmp.l & (FLAG_5 | FLAG_3);
            af.l |= acc.l & FLAG_S;
            af.l |= acc.h & FLAG_C;
            af.l |= FLAG_N;
            af.l |= (acc.l ^ af.h ^ tmp.l) & FLAG_H;
            af.l |= (((acc.l ^ tmp.l ^ af.h) >> 5) 
                    ^ (acc.h << 2)) & FLAG_PV;
            af.l |= acc.l ? 0x00 : FLAG_Z;            // SZ5H3VNC
            prefix = PREFIX_NO;
            return true;

        default:    // Should not happen
            assert(false);
            return true;
    }
}

// vim: et:sw=4:ts=4
