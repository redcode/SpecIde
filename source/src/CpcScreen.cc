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

#include "CpcScreen.h"
#include "config.h"
#include "KeyBinding.h"

#ifdef USE_BOOST_THREADS
#include <boost/chrono/include.hpp>
#include <boost/thread.hpp>
using namespace boost::this_thread;
using namespace boost::chrono;
#else
#include <chrono>
#include <thread>
using namespace std::this_thread;
using namespace std::chrono;
#endif

#include <cfenv>
#include <cmath>

using namespace std;
using namespace sf;

CpcScreen::CpcScreen(map<string, string> o, vector<string> f) :
    Screen(o, f) {}

CpcScreen::~CpcScreen() {}

void CpcScreen::setup() {

    cout << "Initialising common settings..." << endl;
    Screen::setup();

    cout << "Initialising Amstrad CPC..." << endl;
    // Select model and ROMs.
    if (options["model"] == "cpc464") {
        cpc.set464();
    } else if (options["model"] == "cpc664") {
        cpc.set664();
    } else if (options["model"] == "cpc6128") {
        cpc.set6128();
    } // Default model is ZX Spectrum 48K Issue 3...

    uint_fast32_t crtc = 0;
    if (!options["crtc"].empty()) {
        try {
            crtc = stoi(options["crtc"]);
        } catch (invalid_argument &ia) {
            cout << "Invalid CRTC type: '" << options["crtc"] << "' - " << ia.what() << endl;
            options["crtc"] = "0";
        }
    }
    cout << "CRTC type: " << crtc << endl;
    cpc.ga.crtc.type = crtc;

    // Select joystick options.
    pad = (options["pad"] == "yes");
    cout << "Map game pad extra buttons to keys: " << options["pad"] << endl;

    // Sound settings.
    tapeSound = (options["tapesound"] != "no");
    cout << "Play tape sound: " << options["tapesound"] << endl;
    playSound = (options["sound"] != "no");
    cout << "Play sound: " << options["sound"] << endl;

    if (options["stereo"] == "acb") {
        cpc.stereo = StereoMode::STEREO_ACB;
    } else if (options["stereo"] == "abc") {
        cpc.stereo = StereoMode::STEREO_ABC;
    } else if (options["stereo"] == "mono") {
        cpc.stereo = StereoMode::STEREO_MONO;
    } else {
        cpc.stereo = StereoMode::STEREO_MONO;
    }
    cout << "Stereo type: " << options["stereo"] << endl;

    aychip = (options["psgtype"] != "ym");
    cpc.psgChip(aychip);
    cout << "PSG chip: " << options["psgtype"] << endl;

    cpc.z80.zeroByte = options["z80type"] == "cmos" ? 0xFF : 0x00;
    cout << "Z80 type: " << options["z80type"] << endl;

    // Screen settings.
    if (options["scanmode"] == "scanlines") {
        doubleScanMode = true;
        cpc.ga.scanlines = 1;
        cpc.ga.yInc = 2;
    } else if (options["scanmode"] == "average") {
        doubleScanMode = false;
        cpc.ga.scanlines = 2;
        cpc.ga.yInc = 1;
    } else {
        doubleScanMode = false;
        cpc.ga.scanlines = 0;
        cpc.ga.yInc = 1;
    }
    cout << "Scan mode: " << options["scanmode"] << endl;

    xSize = GateArray::X_SIZE;
    ySize = GateArray::Y_SIZE / (doubleScanMode ? 1 : 2);
    texture(xSize, ySize);

    cpc.tape.speed = 1.16;
    loadFiles();

    lBorder = 208;
    rBorder = 40;
    tBorder = 8;
    bBorder = 0;

    wide = true;
    reopenWindow(fullscreen);
    setFullScreen(fullscreen);
    cpc.tapeSound = tapeSound && playSound;
    cpc.psgPlaySound(playSound);
    cpc.setSoundRate(FRAME_TIME_CPC, syncToVideo);
    cpc.skipCycles = cpc.skip;
}

void CpcScreen::loadFiles() {

    for (vector<string>::iterator it = files.begin(); it != files.end(); ++it) {
        switch (guessFileType(*it)) {
            case FileTypes::FILETYPE_CDT:
                cpc.tape.loadCdt(*it);
                break;

            case FileTypes::FILETYPE_CSW:
                cpc.tape.loadCsw(*it);
                break;

            case FileTypes::FILETYPE_DSK:
                {
                    DSKFile dsk;
                    dsk.load(*it);

                    if (dsk.validFile) {
                        cpc.fdc765.drive[0].images.push_back(dsk);
                        cpc.fdc765.drive[0].imageNames.push_back(*it);
                        cpc.fdc765.drive[0].disk = true;
                    }
                }
                break;

            default:
                cout << "Amstrad CPC does not support this file type: " << *it << endl;
                break;
        }
    }
}

void CpcScreen::run() {

    while (!done) {
        high_resolution_clock::time_point start = high_resolution_clock::now();
        high_resolution_clock::time_point frame;
        high_resolution_clock::time_point wakeup;

        while (!done && !menu) {
            start = high_resolution_clock::now();

            // Run until either we get a new frame, or we get 20ms of emulation.
            pollEvents();
            pollCommands();

            cpc.run(!syncToVideo);
            cpc.playSound(true);

            update();

            if (!syncToVideo) {
                uint_fast32_t delay = cpc.cycles / 16;
                uint_fast32_t sleep = delay - (delay % 2000);

                // By not sleeping until the next frame is due, we get some
                // better adjustment
#ifdef USE_BOOST_THREADS
                frame = start + boost::chrono::microseconds(delay);
                wakeup = start + boost::chrono::microseconds(sleep);
#else
                frame = start + std::chrono::microseconds(delay);
                wakeup = start + std::chrono::microseconds(sleep);
#endif
#ifndef DO_NOT_SLEEP
                sleep_until(wakeup);
#endif
                while (high_resolution_clock::now() < frame);
            } else {
                // If we are syncing with the PC's vertical refresh, we need
                // to get at least 20ms of emulation. If this is the case, we
                // update no matter the status of the screen.
                cpc.setSoundRate(FRAME_TIME_CPC, true);
            }
        }

        cpc.playSound(false);

        while (!done && menu) {

            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::KeyPressed) {
                    menu = false;
                }
            }
#ifdef USE_BOOST_THREADS
            sleep_for(boost::chrono::microseconds(20000));
#else
            sleep_for(std::chrono::microseconds(20000));
#endif
        }
    }
}

void CpcScreen::update() {

    scrTexture.update(reinterpret_cast<Uint8*>(doubleScanMode ?
                cpc.ga.pixelsX2 : cpc.ga.pixelsX1));
    window.clear(Color::Black);
    window.draw(scrSprite);
    window.display();

    if (cpc.tape.pulseData.size()) {
        char str[64];
        unsigned int percent = 100 * cpc.tape.pointer / cpc.tape.pulseData.size();
        snprintf(str, 64, "SpecIde [%s(%s)] [%03u%%]",
                SPECIDE_BUILD_DATE, SPECIDE_BUILD_COMMIT, percent);
        window.setTitle(str);
    }
}

void CpcScreen::updateMenu() {
}

void CpcScreen::close() {

    cpc.psgPlaySound(false);
    done = true;
}

void CpcScreen::createEmptyDisk() {

    cpc.fdc765.drive[0].emptyDisk();
}

void CpcScreen::saveDisk() {

    cpc.fdc765.drive[0].saveDisk();
}

void CpcScreen::selectPreviousDisk() {

    cpc.fdc765.drive[0].prevDisk();
}

void CpcScreen::selectNextDisk() {

    cpc.fdc765.drive[0].nextDisk();
}

void CpcScreen::reset() {

    cpc.reset();
}

void CpcScreen::appendLoadTape() {
}

void CpcScreen::clearSaveTape() {
}

void CpcScreen::writeSaveTape() {
}

void CpcScreen::selectSaveTape() {
}

void CpcScreen::resetTapeCounter() {

    cpc.tape.resetCounter();
}

void CpcScreen::startStopTape() {

    cpc.tape.play();
    cpc.tapeSound = tapeSound;
}

void CpcScreen::rewindTape(bool toCounter) {

    cpc.tape.rewind(toCounter ? cpc.tape.counter : 0);
}

void CpcScreen::toggleTapeSound() {

    cpc.tapeSound = tapeSound = !tapeSound;
}

void CpcScreen::toggleSound() {

    cpc.tapeSound = playSound = !playSound;
    cpc.psgPlaySound(playSound);
}

void CpcScreen::togglePsgType() {

    aychip = !aychip;
    cpc.psgChip(aychip);
}

void CpcScreen::joystickHorizontalAxis(uint_fast32_t id, bool l, bool r) {

    if (id < 2) {
        mapKeyJoystickAxis(id, MOVE_L, MOVE_R, l, r);
    }
}

void CpcScreen::joystickVerticalAxis(uint_fast32_t id, bool u, bool d) {

    if (id < 2) {
        mapKeyJoystickAxis(id, MOVE_U, MOVE_D, u, d);
    }
}

void CpcScreen::joystickButtonPress(uint_fast32_t id, uint_fast32_t button) {

    if (id < 2) {
        button += 4;
        if (button < 6) {
            pressKeyJoystickButton(id, button);
        }
    }
}

void CpcScreen::joystickButtonRelease(uint_fast32_t id, uint_fast32_t button) {

    if (id < 2) {
        button += 4;
        if (button < 6) {
            releaseKeyJoystickButton(id, button);
        }
    }
}


void CpcScreen::keyPress(Keyboard::Scancode key) {

    try {
        InputMatrixPosition pos = cpcKeys.at(key);
        cpc.keys[pos.row] &= ~pos.key;
    } catch (out_of_range const& oor) {}
}

void CpcScreen::keyRelease(Keyboard::Scancode key) {

    try {
        InputMatrixPosition pos = cpcKeys.at(key);
        cpc.keys[pos.row] |= pos.key;
    } catch (out_of_range const& oor) {}
}

float CpcScreen::getPixelClock() {

    return static_cast<float>(BASE_CLOCK_CPC) / 1000000.0;
}

void CpcScreen::mapKeyJoystickAxis(uint_fast32_t id,
        uint_fast32_t indexA, uint_fast32_t indexB, bool a, bool b) {
    cpc.keys[cpcJoystick[id][indexA].row] |= cpcJoystick[id][indexA].key;
    cpc.keys[cpcJoystick[id][indexB].row] |= cpcJoystick[id][indexB].key;
    if (a) {
        cpc.keys[cpcJoystick[id][indexA].row] &= ~cpcJoystick[id][indexA].key;
    } else if (b) {
        cpc.keys[cpcJoystick[id][indexB].row] &= ~cpcJoystick[id][indexB].key;
    }
}

void CpcScreen::pressKeyJoystickButton(uint_fast32_t id, uint_fast32_t button) {
    cpc.keys[cpcJoystick[id][button].row] &= ~cpcJoystick[id][button].key;
}

void CpcScreen::releaseKeyJoystickButton(uint_fast32_t id, uint_fast32_t button) {
    cpc.keys[cpcJoystick[id][button].row] |= cpcJoystick[id][button].key;
}
// vim: et:sw=4:ts=4
