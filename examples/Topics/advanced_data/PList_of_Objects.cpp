/**
 * ArrayList of objects
 * by Daniel Shiffman.
 * Translated to C++ by Jose Llamas.
 *
 * This example demonstrates how to use a PList to store
 * a variable number of objects. Items can be added and removed
 * from the PList.
 *
 * Click the mouse to add bouncing balls.
 */

int ballWidth = 48;

struct Ball {
    float x, y, speed, gravity, w, life;

    Ball(float tempX, float tempY, float tempW) {
        x = tempX;
        y = tempY;
        w = tempW;
        speed = 0;
        gravity = 0.1f;
        life = 255;
    }

    void move() {
        // Add gravity to speed
        speed += gravity;

        // Add speed to y location
        y += speed;

        // If square reaches the bottom
        // Reverse speed
        if (y > height) {
            // Dampening
            speed *= -0.8f;
            y = height;
        }
    }

    bool finished() {
        // Balls fade out
        life--;
        return life < 0;
    }

    void display() {
        // Display the circle
        fill(0, life);
        ellipse(x, y, w, w);
    }
};

PList<Ball> balls;

void setup() {
    size(640, 360);
    noStroke();

    // Create an empty PList (will store Ball objects)

    // Start by adding one element
    balls.add(Ball(width/2, 0, ballWidth));
}

void draw() {
    background(255);

    // With an array, we say balls.length, with a PList, we say balls.size()
    // The length of a PList is dynamic
    // Notice how we are looping through the PList backwards
    // This is because we are deleting elements from the list

    for (int i = balls.size() - 1; i >= 0; i--) {

        Ball& ball = balls[i];

        ball.move();
        ball.display();

        if (ball.finished()) {
            // Items can be deleted with remove()
            balls.remove(i);
        }
    }
}

void mousePressed() {
    // A new ball object is added to the PList (by default to the end)
    balls.add(Ball(mouseX, mouseY, ballWidth));
}
