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

#include "CRTC.h"

#include <iostream>
using namespace std;

CRTC::CRTC(uint_fast8_t type) :
    type(type),
    index(0),
    regs{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, static_cast<uint_fast8_t>((type == 1) ? 0xFF : 0x00)},
    mask{
        0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x1F, 0x7F, 0x7F,
        0x03, 0x1F, 0x7F, 0x1F, 0x3F, 0xFF, 0x3F, 0xFF,
        0x3F, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, static_cast<uint_fast8_t>((type == 1) ? 0xFF : 0x00)},
    dirs{
        AccessType::CRTC_WO,    // Horizontal Total (WO)
        AccessType::CRTC_WO,    // Horizontal Displayed (WO)
        AccessType::CRTC_WO,    // Horizontal Sync Position (WO)
        AccessType::CRTC_WO,    // Horizontal & Vertical Sync Width (WO)
        AccessType::CRTC_WO,    // Vertical Total (WO)
        AccessType::CRTC_WO,    // Vertical Total Adjust (WO)
        AccessType::CRTC_WO,    // Vertical Displayed (WO)
        AccessType::CRTC_WO,    // Vertical Sync Position (WO)
        AccessType::CRTC_WO,    // Interlace & Skey (WO)
        AccessType::CRTC_WO,    // Maximum Raster Address (WO)
        AccessType::CRTC_RW,    // Cursor Start Raster (RW)
        AccessType::CRTC_RW,    // Cursor End Raster (RW)
        (type == 0 || type == 3 || type == 4)
            ? AccessType::CRTC_RW : AccessType::CRTC_WO,    // Display Start Address (High byte)
        (type == 0 || type == 3 || type == 4)
            ? AccessType::CRTC_RW : AccessType::CRTC_WO,    // Display Start Address (Low byte)
        AccessType::CRTC_RW,    // Cursor Address (High byte)
        AccessType::CRTC_RW,    // Cursor Address (Low byte)
        AccessType::CRTC_RO,    // Light Pen Address (High byte)
        AccessType::CRTC_RO,    // Light Pen Address (Low byte)
        AccessType::CRTC_RO,    // All the remaining addresses are unused.
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO,
        AccessType::CRTC_RO} {}

void CRTC::wrAddress(uint_fast8_t byte) {

    index = byte & 0x1F;
}

void CRTC::wrRegister(uint_fast8_t byte) {

    if (dirs[index] == AccessType::CRTC_WO
            || dirs[index] == AccessType::CRTC_RW) {
        regs[index] = byte & mask[index];

        switch (index) {
            case 0: // Horizontal Total. Actual value = Set value + 1.
                if (type == 0) {
                    if (regs[0] == 0x00) regs[0] = 0x01;
                }
                hTotal = regs[0] + 1;
                break;
            case 1: // Horizontal Displayed.
                hDisplayed = regs[1];
                break;
            case 2: // Horizontal Sync Position.
                if (type == 0) {
                    hsPos = regs[2] + 1;
                } else {
                    hsPos = regs[2];
                }
                break;
            case 3: // Horizontal & Vertical Sync Width.
                vswMax = (regs[3] & 0xF0) >> 4;
                hswMax = (regs[3] & 0x0F);
                switch (type) {
                    case 0: // Type 0 (UM6845):
                        // If VSW = 0, this gives 16 VSYNC lines.
                        // If HSW = 0, this gives no HSYNC.
                        if (vswMax == 0) vswMax = 0x10;
                        break;
                    case 1: // Type 1 (UM6845R):
                        // VSW is ignored. VSYNC is fixed to 16 lines.
                        // If HWS = 0, this gives no HSYNC.
                        vswMax = 0x10;
                        break;
                    case 2: // Type 2 (MC6845):
                        // VSW is ignored. VSYNC is fixed to 16 lines.
                        // If HSW = 0, this gives 16 HSYNC cycles.
                        vswMax = 0x10;
                        if (hswMax == 0) hswMax = 0x10;
                        break;
                    case 3: // fall-through
                    case 4: // Type 3, 4 (Pre-ASIC, ASIC)
                        // If VSW = 0, this gives 16 VSYNC lines.
                        // if HSW = 0, this gives 16 HSYNC cycles.
                        if (vswMax == 0) vswMax = 0x10;
                        if (hswMax == 0) hswMax = 0x10;
                    default:
                        break;
                }
                break;
            case 4: // Vertical total.
                vTotal = regs[4] + 1;
                break;
            case 5: // Vertical adjust.
                vAdjust = regs[5];
                break;
            case 6: // Vertical displayed.
                vDisplayed = regs[6];
                break;
            case 7: // Vertical Sync Position.
                vsPos = regs[7];
                break;
            case 9: // Max Raster Address.
                rMax = regs[9] + 1;
                break;
            default:
                break;
        }

        outOfRange = (vsPos >= vTotal);
    }
}

void CRTC::rdStatus(uint_fast8_t &byte) {

    switch (type) {
        case 0: // fall-through
        case 2:
            byte = 0x00;
            break;
        case 1:
            byte = status;
            break;
        case 3: // fall-through
        case 4:
            rdRegister(byte);
            break;
        default:
            break;
    }
}

void CRTC::rdRegister(uint_fast8_t &byte) {

    if (dirs[index] != AccessType::CRTC_WO) {
        switch (index) {
            case 0x1f:  // fall-through
                if (type != 1) {   // Hi-Z in type 1
                    byte = regs[(type < 3) ? index : ((index & 0x7) | 0x8)];
                }
                break;
            default:
                byte = regs[(type < 3) ? index : ((index & 0x7) | 0x8)];
                break;
        }
    } else {
        switch (type) {
            case 0: // fall-through
            case 1: // fall-through
            case 2:
                byte = 0x00;
                break;
            default:
                break;
        }
    }
}

void CRTC::clock() {

    // Counters:
    // - Character counter (aka Horizontal Counter)
    //      Compared to R0 (Horizontal Total Register)
    //          Set -> Skew control -> DISPTMG
    //          Reset Character counter (MR)
    //          Clock Raster counter
    //          Clock Vertical Sync Width Counter
    //          Affect Linear Address Generator
    //          Affect Interlace Control
    //
    //      Compared to R1 (Horizontal Displayed Register)
    //          Reset -> Skew control -> DISPTMG
    //          Reset -> Linear Address Generator
    //      Compared to R2 (Horizontal Sync Position Register)
    //          Set HSYNC
    //          Enable Horizontal Sync Width Counter
    //      Compared to R0 / 2
    //          Affect Interlace Control

    // Horizontal counter is incremented with each tick
    ++hCounter;

    // This is for interlace control
    hh = (hCounter > (hTotal >> 1));

    // Here increment Raster Counter, Vertical Sync Width Counter
    if (hCounter >= hTotal) {   // Horizontal Total marks the end of a scan
        hCounter = 0;               // Reset Horizontal Counter
        hDisplay = true;            // Drawing screen

        // Increment raster counter and check
        rCounter = (rCounter + 1) & 0x1F;
        if (rCounter >= rMax) { // Maximum Raster Address
            rCounter = 0;           // Reset Raster Counter

            vCounter = (vCounter + 1) & 0x7F;
            // Vertical Total marks the end of a frame, but we also must
            // account for Vertical Total Adjustment
            if (((vCounter == vTotal) && (rCounter >= vAdjust))
                    || (vCounter > vTotal)) {
                vCounter = 0;
                rCounter = 0;
                vDisplay = true;
                status &= 0xDF;
            }

            if (vCounter == vDisplayed) {  // Vertical Displayed
                vDisplay = false;
                status |= 0x20;
            }

            if (vCounter == vsPos) {  // Vertical Sync Position
                vSync = true;
                vswCounter = 0;
            }

            // Base address is updated on VCC=0 (CRTC 1) or VCC=0 and VLC=0 (other)
            if (vCounter == 0 && (type == 1 || rCounter == 0)) {
                lineAddress = (regs[12] & 0x3F) * 0x100 + regs[13];
            }
        }

        // Raster level
        if (vSync) {
            if (vswCounter++ == vswMax) {
                vSync = false;
                vswCounter = 0;
            }
        }
    }

    if (hCounter == hDisplayed) {   // Horizontal Displayed
        hDisplay = false;                   // Drawing border
        if (rCounter == rMax - 1) {
            lineAddress += hCounter;
        }
    }

    if (hCounter == hsPos) {   // Horizontal Sync Position
        hSync = true;                       // HSYNC pulse
        hswCounter = 0;
    }

    if (hSync) {    // Horizontal Sync Width is incremented during HSYNC pulse
        if (hswCounter++ == hswMax) {    // Horizontal Sync Width
            hSync = false;
            hswCounter = 0;
        }
    }

    charAddress = lineAddress + hCounter;
    pageAddress = (charAddress & 0x3000) << 2;
    byteAddress = pageAddress | ((rCounter & 7) << 11) | ((charAddress & 0x3FF) << 1);
    dispEn = hDisplay && vDisplay;
}
// vim: et:sw=4:ts=4
