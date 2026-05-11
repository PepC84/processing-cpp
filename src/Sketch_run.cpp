#include "Processing.h"
namespace Processing {
/**
 * Processing++ Logo
 * White background, gold plus with shadow and glow particles.
 */

void setup() {
    size(1200, 700);
    smooth();
    noLoop();
}

void draw() {
    background(255);
    translate(width/2, height/2);

    // Soft shadow plus
    noStroke();
    fill(120, 90, 40, 90);
    rect(-25 + 35, -120 + 35, 50, 240, 16);
    rect(-120 + 35, -25 + 35, 240, 50, 16);

    // Main plus
    fill(210, 170, 70);
    rect(-25, -120, 50, 240, 16);
    rect(-120, -25, 240, 50, 16);

    // Small glow rings
    noFill();
    strokeWeight(2);
    for (int i = 0; i < 20; i++) {
        stroke(230, 190, 90, 8);
        rect(-120 - i*2, -120 - i*2, 240 + i*4, 240 + i*4, 20);
    }

    // Floating particles
    noStroke();
    fill(220, 180, 80, 100);
    ellipse(-170, -100, 10, 10);
    ellipse( 140, -130, 14, 14);
    ellipse( 180,   90,  8,  8);
    ellipse(-150,  140, 12, 12);
}

static void _sketchWire() {
}
static int _autoWire = []{ _wireCallbacksFn = _sketchWire; return 0; }();
} // namespace Processing
