float camX = 0;
float camY = -150;
float camZ = 400;

float rotY = 0;
float rotX = 0;

float yVelocity = 0;

bool onGround = true;

float gravity = 0.8f;
float jumpForce = -15;

int gridSize = 40;
int blockSize = 50;

float sensitivity = 0.002f;

bool cursorCaptured = false;
bool firstMouse = true;

float prevMouseX;
float prevMouseY;

void setup() {

    size(1280, 720, P3D);
}

void draw() {

    background(135, 206, 235);

    updateMouseLook();
    handleMovement();
    updatePhysics();

    lights();

    float lookX = camX + cos(rotX) * sin(rotY);
    float lookY = camY + sin(rotX);
    float lookZ = camZ - cos(rotX) * cos(rotY);

    camera(
        camX, camY, camZ,
        lookX, lookY, lookZ,
        0, 1, 0
    );

    drawGround();
}

void capture() {

    captureMouse();
    noCursor();

    cursorCaptured = true;
    firstMouse = true;
}

void release() {

    releaseMouse();
    cursor();

    cursorCaptured = false;
    firstMouse = true;
}

bool keyDown(char k) {

    return _keyPressed &&
           (key == k || key == k + 32 || key == k - 32);
}

bool specialDown(int k) {

    return _keyPressed && keyCode == k;
}

void updateMouseLook() {

    if (!cursorCaptured) {
        return;
    }

    if (firstMouse) {

        prevMouseX = mouseX;
        prevMouseY = mouseY;

        firstMouse = false;
    }

    float dx = mouseX - prevMouseX;
    float dy = mouseY - prevMouseY;

    prevMouseX = mouseX;
    prevMouseY = mouseY;

    rotY += dx * sensitivity;
    rotX += dy * sensitivity;

    rotX = constrain(rotX, -PI / 2.2f, PI / 2.2f);
}

void handleMovement() {

    float speed = 8;

    if (specialDown(LEFT)) {
        rotY -= 0.04f;
    }

    if (specialDown(RIGHT)) {
        rotY += 0.04f;
    }

    if (specialDown(UP)) {
        rotX -= 0.04f;
    }

    if (specialDown(DOWN)) {
        rotX += 0.04f;
    }

    rotX = constrain(rotX, -PI / 2.2f, PI / 2.2f);

    if (keyDown(' ')) {

        if (onGround) {

            yVelocity = jumpForce;
            onGround = false;
        }
    }

    float forwardX = sin(rotY);
    float forwardZ = -cos(rotY);

    float rightX = cos(rotY);
    float rightZ = sin(rotY);

    if (keyDown('W')) {

        camX += forwardX * speed;
        camZ += forwardZ * speed;
    }

    if (keyDown('S')) {

        camX -= forwardX * speed;
        camZ -= forwardZ * speed;
    }

    if (keyDown('A')) {

        camX -= rightX * speed;
        camZ -= rightZ * speed;
    }

    if (keyDown('D')) {

        camX += rightX * speed;
        camZ += rightZ * speed;
    }
}

void updatePhysics() {

    camY += yVelocity;

    yVelocity += gravity;

    if (camY > -150) {

        camY = -150;

        yVelocity = 0;

        onGround = true;
    }
}

void drawGround() {

    for (int x = -gridSize; x < gridSize; x++) {

        for (int z = -gridSize; z < gridSize; z++) {

            pushMatrix();

            translate(
                x * blockSize,
                0,
                z * blockSize
            );

            fill(40, 180, 40);
            stroke(20, 120, 20);

            box(blockSize, 20, blockSize);

            popMatrix();
        }
    }
}

void keyPressed() {

    if (key == ESC) {

        release();

        key = 0;
    }

    if (key == 'e' || key == 'E') {

        capture();
    }
}

void mousePressed() {

    if (!cursorCaptured) {

        capture();
    }
}
