/* This file is part of SpecIde, (c) Marta Sevillano Mancilla, 2016-2021.
 *
 * SpecIde is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * SpecIde is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SpecIde.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "CRTC.h"
#include "Z80Defs.h"

uint_fast32_t constexpr KEEP = 2;
uint_fast32_t constexpr MOVE = 1;
uint_fast32_t constexpr LOAD = 0;

class GateArray {

    public:
        /** CRTC instance. */
        CRTC crtc;

        /** Gate Array data bus. */
        uint_fast8_t d; 
        /** CPU control bus. */
        uint_fast16_t z80_c = 0xFFFF;

        /** Selected pen. */
        uint_fast8_t pen = 0;
        /** Pen colour references. */
        uint_fast8_t pens[16];
        /** Border colour reference. */
        uint_fast8_t border = 1;
        /** New video mode. */
        uint_fast8_t newMode = 1;
        /** Actual video mode. */
        uint_fast8_t actMode = 1;
        /** Lower ROM readable at $0000-$3FFF. */
        bool lowerRom = true;
        /** Upper ROM readable at $C000-$FFFF. */
        bool upperRom = true;

        /** Select ink, as oppossed to border. */
        bool inksel = false;
        /** Display enable signal. */
        bool dispen = false;
        /** Current video data byte. */
        uint_fast8_t colour = 0x00;
        /** Latched video data byte. */
        uint_fast8_t videoByte = 0x00;
        bool blanking = true;

        uint_fast32_t xPos = 0;
        uint_fast32_t yPos = 0;
        uint_fast32_t xInc = 0;
        uint_fast32_t yInc = 0;
        uint_fast32_t yCnt = 0;
         
        bool hSync_d = false;
        bool vSync_d = false;
        bool sync = false;

        uint_fast32_t displayHSync = 0;
        uint_fast32_t displayVSync = 0;

        /** Cycle counter. */
        uint_fast32_t counter = 0;
        /** HSYNC counter for INT generation. */
        uint_fast32_t intCounter = 0;
        /** HSYNC counter for upper border delay. */
        uint_fast32_t hCounter = 0;

        uint_fast32_t scanlines = 0;

        /**
         * Clock the Gate Array.
         */
        void clock();

        void reset();

        /**
         * Paint pixels into bitmap.
         */
        void paint();

        /**
         * Update the position of the electron beam.
         */
        void updateBeam();

        /**
         * Evaluate the status of hSync and change video mode.
         */
        void updateVideoMode();

        /**
         * Generate interrupts.
         */
        void generateInterrupts();

        /**
         * Acknowledge interrupts.
         */
        void intAcknowledge();

        bool psgClock() const { return counter == 0; }
        bool cpuClock() const { return (counter & 1) == 1; }
        bool crtcClock() const { return counter == 0xb; }
        bool cpuReady() const { return readyTable[counter]; }
        uint_fast16_t cClkOffset() const { return cClkBit[counter]; }
        bool muxVideo() const { return muxTable[counter]; }
        bool blockIorq() const { return e244Table[counter]; }

        /**
         * Write a byte to the Gate Array.
         *
         * This function invokes the appropriate action.
         *
         * @param byte The byte from data bus.
         */
        void write(uint_fast8_t byte);

        /**
         * Select a pen on which to operate.
         *
         * @param byte Byte from Z80.
         */
        void selectPen(uint_fast8_t byte);

        /**
         * Select colour for currently selected pen.
         *
         * @param byte Byte from Z80.
         */
        void selectColour(uint_fast8_t byte);

        /**
         * Select screen mode, ROM visibility, and delay interrupts.
         *
         * @param byte Byte from Z80.
         */
        void selectScreenAndRom(uint_fast8_t byte);

        static uint_fast32_t constexpr X_SIZE = 1024;
        static uint_fast32_t constexpr Y_SIZE = 625;

        static uint32_t pixelsX1[X_SIZE * Y_SIZE / 2];
        static uint32_t pixelsX2[X_SIZE * Y_SIZE];

        /** Averaged colours between two scans. */
        uint32_t averagedColours[1024];
        /** Colour definitions. */
#if SPECIDE_BYTE_ORDER == 1
        static uint32_t constexpr colours[32] = {
            0x7F7F7FFF, 0x7F7F7FFF, 0x00FF7FFF, 0xFFFF7FFF,
            0x00007FFF, 0xFF007FFF, 0x007F7FFF, 0xFF7F7FFF,
            0xFF007FFF, 0xFFFF7FFF, 0xFFFF00FF, 0xFFFFFFFF,
            0xFF0000FF, 0xFF00FFFF, 0xFF7F00FF, 0xFF7FFFFF,
            0x00007FFF, 0x00FF7FFF, 0x00FF00FF, 0x00FFFFFF,
            0x000000FF, 0x0000FFFF, 0x007F00FF, 0x007FFFFF,
            0x7F007FFF, 0x7FFF7FFF, 0x7FFF00FF, 0x7FFFFFFF,
            0x7F0000FF, 0x7F00FFFF, 0x7F7F00FF, 0x7F7FFFFF};
#else
        static uint32_t constexpr colours[32] = {
            0xFF7F7F7F, 0xFF7F7F7F, 0xFF7FFF00, 0xFF7FFFFF,
            0xFF7F0000, 0xFF7F00FF, 0xFF7F7F00, 0xFF7F7FFF,
            0xFF7F00FF, 0xFF7FFFFF, 0xFF00FFFF, 0xFFFFFFFF,
            0xFF0000FF, 0xFFFF00FF, 0xFF007FFF, 0xFFFF7FFF,
            0xFF7F0000, 0xFF7FFF00, 0xFF00FF00, 0xFFFFFF00,
            0xFF000000, 0xFFFF0000, 0xFF007F00, 0xFFFF7F00,
            0xFF7F007F, 0xFF7FFF7F, 0xFF00FF7F, 0xFFFFFF7F,
            0xFF00007F, 0xFFFF007F, 0xFF007F7F, 0xFFFF7F7F};
#endif

        /**
         * Gate array sequence.
         *
         * With the information available, looks like this sequence is never reset.
         * In order to be reset, RST needs to be hign, and M1, IORQ and RD need to be
         * low. This last combination never happens in a Z80 CPU.
         */
        static uint_fast8_t constexpr sequence[16] = {
            0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80,
            0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F
        };

        /**
         * 4MHz CPU clock.
         *
         * The equation is (S1 ^ S3) | (S5 ^ S7), so it's true for any value of the
         * sequencer with digits 3, 7, 8 or C.
         * The result is latched, so this table is shifted right one position.
         */
        static bool constexpr phiTable[16] = {
            true, false, false, true, true, false, false, true,
            true, false, false, true, true, false, false, true
        };

        /**
         * 4MHz CPU clock edge.
         *
         * Here the CPU clock changes phase. This is useful for clocking Z80::clock()
         */
        static bool constexpr cpuTable[16] = {
            false, true, false, true, false, true, false, true,
            false, true, false, true, false, true, false, true
        };

        /**
         * 1MHz clock.
         *
         * The equation is !(S2 | S5), so it's true from 0xC0 to 0x03.
         */
        static bool constexpr cClkTable[16] = {
            false, false, false, false, false, false, true, true,
            true, true, true, false, false, false, false, false
        };

        static uint_fast16_t constexpr cClkBit[16] = {
            0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };

        /**
         * Gate Array READY signal.
         *
         * This is the Z80 #WAIT signal.
         */
        static bool constexpr readyTable[16] = {
            true, false, false, false, false, false, false, false,
            false, false, false, false, true, true, true, true
        };

        /**
         * I/O latch signal.
         */
        static bool constexpr e244Table[16] = {
            false, false, false, true, true, true, true, true,
            true, true, true, true, false, false, false, false
        };

        /**
         * Address MUX setting table.
         *
         * Select Video address (true) or CPU address (false) to read data.
         */
        static bool constexpr muxTable[16] = {
            true, true, true, true, true, true, true, true,
            true, true, false, false, false, false, false, false
        };

        static uint_fast32_t constexpr modeTable[4][8] = {
            { KEEP, KEEP, KEEP, MOVE, KEEP, KEEP, KEEP, LOAD },
            { KEEP, MOVE, KEEP, MOVE, KEEP, MOVE, KEEP, LOAD },
            { MOVE, MOVE, MOVE, MOVE, MOVE, MOVE, MOVE, LOAD },
            { KEEP, KEEP, KEEP, MOVE, KEEP, KEEP, KEEP, LOAD }
        };

        static uint_fast8_t constexpr pixelTable[4][256] = {
            { 
                0x0, 0x0, 0x8, 0x8, 0x0, 0x0, 0x8, 0x8, 0x2, 0x2, 0xa, 0xa, 0x2, 0x2, 0xa, 0xa,
                0x0, 0x0, 0x8, 0x8, 0x0, 0x0, 0x8, 0x8, 0x2, 0x2, 0xa, 0xa, 0x2, 0x2, 0xa, 0xa,
                0x4, 0x4, 0xc, 0xc, 0x4, 0x4, 0xc, 0xc, 0x6, 0x6, 0xe, 0xe, 0x6, 0x6, 0xe, 0xe,
                0x4, 0x4, 0xc, 0xc, 0x4, 0x4, 0xc, 0xc, 0x6, 0x6, 0xe, 0xe, 0x6, 0x6, 0xe, 0xe,
                0x0, 0x0, 0x8, 0x8, 0x0, 0x0, 0x8, 0x8, 0x2, 0x2, 0xa, 0xa, 0x2, 0x2, 0xa, 0xa,
                0x0, 0x0, 0x8, 0x8, 0x0, 0x0, 0x8, 0x8, 0x2, 0x2, 0xa, 0xa, 0x2, 0x2, 0xa, 0xa,
                0x4, 0x4, 0xc, 0xc, 0x4, 0x4, 0xc, 0xc, 0x6, 0x6, 0xe, 0xe, 0x6, 0x6, 0xe, 0xe,
                0x4, 0x4, 0xc, 0xc, 0x4, 0x4, 0xc, 0xc, 0x6, 0x6, 0xe, 0xe, 0x6, 0x6, 0xe, 0xe,
                0x1, 0x1, 0x9, 0x9, 0x1, 0x1, 0x9, 0x9, 0x3, 0x3, 0xb, 0xb, 0x3, 0x3, 0xb, 0xb,
                0x1, 0x1, 0x9, 0x9, 0x1, 0x1, 0x9, 0x9, 0x3, 0x3, 0xb, 0xb, 0x3, 0x3, 0xb, 0xb,
                0x5, 0x5, 0xd, 0xd, 0x5, 0x5, 0xd, 0xd, 0x7, 0x7, 0xf, 0xf, 0x7, 0x7, 0xf, 0xf,
                0x5, 0x5, 0xd, 0xd, 0x5, 0x5, 0xd, 0xd, 0x7, 0x7, 0xf, 0xf, 0x7, 0x7, 0xf, 0xf,
                0x1, 0x1, 0x9, 0x9, 0x1, 0x1, 0x9, 0x9, 0x3, 0x3, 0xb, 0xb, 0x3, 0x3, 0xb, 0xb,
                0x1, 0x1, 0x9, 0x9, 0x1, 0x1, 0x9, 0x9, 0x3, 0x3, 0xb, 0xb, 0x3, 0x3, 0xb, 0xb,
                0x5, 0x5, 0xd, 0xd, 0x5, 0x5, 0xd, 0xd, 0x7, 0x7, 0xf, 0xf, 0x7, 0x7, 0xf, 0xf,
                0x5, 0x5, 0xd, 0xd, 0x5, 0x5, 0xd, 0xd, 0x7, 0x7, 0xf, 0xf, 0x7, 0x7, 0xf, 0xf },
            {
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3 },
            {
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1 },
            {
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3,
                0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3
            }
        };
};
// vim: et:sw=4:ts=4
