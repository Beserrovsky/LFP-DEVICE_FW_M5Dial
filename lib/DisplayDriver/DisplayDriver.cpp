#include "DisplayDriver.h"

DisplayDriver g_displayDriver;

DisplayDriver::DisplayDriver()
    : mirrorEnabled(false), width(240), height(240) {
}

DisplayDriver::~DisplayDriver() {
}

bool DisplayDriver::init() {
    return true;
}

void DisplayDriver::update() {
}

void DisplayDriver::flush() {
}

void DisplayDriver::setMirrorEnabled(bool enabled) {
    mirrorEnabled = enabled;
}

bool DisplayDriver::getMirrorEnabled() const {
    return mirrorEnabled;
}

uint16_t DisplayDriver::getWidth() const {
    return width;
}

uint16_t DisplayDriver::getHeight() const {
    return height;
}
