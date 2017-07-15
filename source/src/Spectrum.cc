#include "Spectrum.h"

Spectrum::Spectrum() :
    ram{Memory(14), Memory(14), Memory(14), Memory(14),     // 64K
        Memory(14), Memory(14), Memory(14), Memory(14),     // 128K
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),     // 256K
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),     // 512K
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14),
        Memory(14), Memory(14), Memory(14), Memory(14)},    // 1024K
    rom{Memory(14, true), Memory(14, true), Memory(14, true), Memory(14, true),
        Memory(14, true), Memory(14, true), Memory(14, true), Memory(14, true),
        Memory(14, true), Memory(14, true), Memory(14, true), Memory(14, true),
        Memory(14, true), Memory(14, true), Memory(14, true), Memory(14, true)},
    map{&rom[0], &ram[0], &ram[1], &ram[2]}
{
    size_t pos = 0;
    char c;
    ifstream ifs("48.rom", ifstream::binary);
    while (ifs.get(c))
        rom[0].memory[pos++] = c;
}

void Spectrum::clock()
{
    bool as_ = ((z80.c & SIGNAL_MREQ_) == SIGNAL_MREQ_);
    bool io_ = ((z80.c & SIGNAL_IORQ_) == SIGNAL_IORQ_);
    bool rd_ = ((z80.c & SIGNAL_RD_) == SIGNAL_RD_);
    bool wr_ = ((z80.c & SIGNAL_WR_) == SIGNAL_WR_);

    // First we clock the ULA. This generates video and contention signals.
    // We need to provide the ULA with the Z80 address and control buses.
    ula.z80_a = z80.a;
    ula.z80_c = z80.c;
    
    // ULA gets the data from memory or Z80, or outputs data to Z80.
    if (ula.hiz == false)           // Is ULA mastering the bus?
        ula.d = map[1]->d;
    else if ((io_ == false) && (wr_ == false)) // Is Z80 mastering and writing?
        ula.d = z80.d;

    ula.clock();
    z80.c = ula.c;

    if (ula.cpuClock)
    {
        // Bank 0: 0000h - ROM
        map[0]->a = z80.a;
        map[0]->d = z80.d;
        map[0]->as_ = as_ || ((z80.a & 0xC000) != 0x0000);
        map[0]->rd_ = rd_;
        map[0]->wr_ = wr_;
        map[0]->clock();

        // Bank 2: 8000h - Extended memory
        map[2]->a = z80.a;
        map[2]->d = z80.d;
        map[2]->as_ = as_ || ((z80.a & 0xC000) != 0x8000);
        map[2]->rd_ = rd_;
        map[2]->wr_ = wr_;
        map[2]->clock();

        // Bank 3: C000h - Extended memory
        map[3]->a = z80.a;
        map[3]->d = z80.d;
        map[3]->as_ = as_ || ((z80.a & 0xC000) != 0xC000);
        map[3]->rd_ = rd_;
        map[3]->wr_ = wr_;
        map[3]->clock();
    }

    // Bank 1: 4000h - Contended memory
    if (ula.hiz == false) // Is ULA mastering the bus?
    {
        map[1]->a = ula.a;
        map[1]->as_ = false;
        map[1]->rd_ = false;
        map[1]->wr_ = true;
    }
    else
    {
        map[1]->a = z80.a;
        map[1]->d = z80.d;  // Only Z80 writes here.
        map[1]->as_ = as_ || ((z80.a & 0xC000) != 0x4000);
        map[1]->rd_ = rd_;
        map[1]->wr_ = wr_;
    }
    map[1]->clock();

    if (ula.cpuClock == true)
    {
        // Z80 gets data from the ULA or memory, only when reading.
        if (rd_ == false)
        {
            if (io_ == false)
                if ((z80.a & 0x0001) == 0x0000) // Reading from ULA port
                    z80.d = ula.d;
                else if (ula.hiz == false)      // Floating bus
                    z80.d = map[1]->d;
                else                            // High impedance
                    z80.d = 0xFF;
            else if (as_ == false)
                z80.d = map[(z80.a & 0xC000) >> 14]->d;
        }

        z80.clock();
    }
}

void Spectrum::reset()
{
    z80.reset();
}

// vim: et:sw=4:ts=4
