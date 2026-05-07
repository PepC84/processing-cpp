/**
 * Minecraft.cpp -- ProcessingGL
 *
 * First-person voxel world.
 * Controls:
 *   WASD        -- move
 *   Space       -- jump
 *   Mouse       -- look around (click window to capture)
 *   Left click  -- break block
 *   Right click -- place block (currently selected type)
 *   1-5         -- select block type
 *   E           -- toggle cursor capture
 *   ESC         -- release cursor
 */

#include <vector>
#include <cmath>
#include <algorithm>

// ── World constants ───────────────────────────────────────────────────────────
static const int WORLD_W  = 32;
static const int WORLD_H  = 32;
static const int WORLD_D  = 32;
static const int SEA_LEVEL = 12;
static const float BLOCK   = 1.0f;
static const float GRAVITY = -20.0f;
static const float JUMP_V  =  8.0f;
static const float SPEED   =  5.0f;
static const float REACH   =  5.0f;

// ── Block types ───────────────────────────────────────────────────────────────
enum BlockType : uint8_t {
    AIR=0, GRASS, DIRT, STONE, WOOD, LEAVES, SAND, WATER, BEDROCK
};

// ── World storage ─────────────────────────────────────────────────────────────
static uint8_t world[WORLD_W][WORLD_H][WORLD_D];

// ── Player state ──────────────────────────────────────────────────────────────
static float px, py, pz;       // position (feet)
static float yaw   = 0;        // horizontal angle (radians)
static float pitch = 0;        // vertical angle
static float velY  = 0;        // vertical velocity
static bool  onGround = false;
static bool  cursorCaptured = false;
static int   selectedBlock = GRASS;
static float prevMouseX, prevMouseY;
static bool  firstMouse = true;

// ── Key state ─────────────────────────────────────────────────────────────────
static bool kW=false, kS=false, kA=false, kD=false, kSpace=false;

// ── Noise helper ─────────────────────────────────────────────────────────────
static float smoothNoise(int x, int z) {
    // simple hash-based height
    int h = (x*374761393 + z*668265263);
    h = (h ^ (h >> 13)) * 1274126177;
    h = h ^ (h >> 16);
    return (float)(h & 0xFF) / 255.0f;
}
static float interpNoise(float x, float z) {
    int xi=(int)floor(x), zi=(int)floor(z);
    float xf=x-xi, zf=z-zi;
    // cosine interpolation
    auto cerp=[](float a,float b,float t){
        float f=(1-cos(t*PI))*0.5f; return a*(1-f)+b*f;
    };
    float v00=smoothNoise(xi,zi),   v10=smoothNoise(xi+1,zi);
    float v01=smoothNoise(xi,zi+1), v11=smoothNoise(xi+1,zi+1);
    return cerp(cerp(v00,v10,xf), cerp(v01,v11,xf), zf);
}
static int terrainHeight(int x, int z) {
    float h = interpNoise(x*0.15f, z*0.15f) * 0.6f
            + interpNoise(x*0.05f, z*0.05f) * 0.4f;
    return SEA_LEVEL + (int)(h * 8) - 2;
}

// ── World init ────────────────────────────────────────────────────────────────
static void generateWorld() {
    memset(world, AIR, sizeof(world));
    for (int x=0; x<WORLD_W; x++) {
        for (int z=0; z<WORLD_D; z++) {
            int top = terrainHeight(x, z);
            top = constrain(top, 1, WORLD_H-2);
            for (int y=0; y<=top; y++) {
                if (y == 0)        world[x][y][z] = BEDROCK;
                else if (y < top-3) world[x][y][z] = STONE;
                else if (y < top)   world[x][y][z] = DIRT;
                else {
                    // surface
                    if (top <= SEA_LEVEL+1) world[x][y][z] = SAND;
                    else                    world[x][y][z] = GRASS;
                }
            }
            // water in low areas
            for (int y=top+1; y<=SEA_LEVEL; y++)
                world[x][y][z] = WATER;
            // occasional trees on grass
            if (top > SEA_LEVEL+1 && world[x][top][z]==GRASS) {
                int h2 = (int)(smoothNoise(x+100,z+100)*4);
                if (h2 == 0 && x>1 && x<WORLD_W-2 && z>1 && z<WORLD_D-2) {
                    int trunk = 3 + (int)(smoothNoise(x,z+50)*2);
                    for (int t=1; t<=trunk && top+t<WORLD_H-1; t++)
                        world[x][top+t][z] = WOOD;
                    int lh = top + trunk;
                    for (int lx=-2;lx<=2;lx++) for (int lz=-2;lz<=2;lz++)
                        for (int ly=0;ly<=1;ly++) {
                            int bx=x+lx, by=lh+ly, bz=z+lz;
                            if (bx>=0&&bx<WORLD_W&&by>=0&&by<WORLD_H&&bz>=0&&bz<WORLD_D)
                                if (world[bx][by][bz]==AIR)
                                    world[bx][by][bz]=LEAVES;
                        }
                }
            }
        }
    }
}

// ── Block colours ─────────────────────────────────────────────────────────────
// top, bottom, side colors as packed RGB
struct BlockColors { int top,bot,side; };
static BlockColors blockColor(uint8_t t) {
    switch(t) {
        case GRASS:  return {color(94,139,57),  color(134,96,67),  color(134,96,67)};
        case DIRT:   return {color(134,96,67),  color(134,96,67),  color(134,96,67)};
        case STONE:  return {color(128,128,128),color(128,128,128),color(128,128,128)};
        case WOOD:   return {color(107,83,49),  color(107,83,49),  color(107,83,49)};
        case LEAVES: return {color(58,107,36),  color(58,107,36),  color(58,107,36)};
        case SAND:   return {color(210,196,140),color(210,196,140),color(210,196,140)};
        case WATER:  return {color(64,100,200), color(64,100,200), color(64,100,200)};
        case BEDROCK:return {color(40,40,40),   color(40,40,40),   color(40,40,40)};
        default:     return {color(200,200,200),color(200,200,200),color(200,200,200)};
    }
}

// ── Draw a single voxel face (6 faces, only exposed ones) ────────────────────
static bool inBounds(int x,int y,int z){
    return x>=0&&x<WORLD_W&&y>=0&&y<WORLD_H&&z>=0&&z<WORLD_D;
}
static bool isSolid(int x,int y,int z){
    if(!inBounds(x,y,z)) return false;
    return world[x][y][z]!=AIR && world[x][y][z]!=WATER;
}
static bool isOpaque(int x,int y,int z){
    if(!inBounds(x,y,z)) return false;
    return world[x][y][z]!=AIR;
}

static void drawFace(float x,float y,float z,
                     float nx,float ny,float nz,
                     int col, float bright) {
    // Apply simple directional brightness
    int r = (int)(red(col)   * bright);
    int g = (int)(green(col) * bright);
    int b = (int)(blue(col)  * bright);
    fill(r, g, b);
    noStroke();

    float x0=x, x1=x+BLOCK, y0=y, y1=y+BLOCK, z0=z, z1=z+BLOCK;

    beginShape(QUADS);
    if (ny > 0.5f) { // +Y top
        vertex(x0,y1,z0); vertex(x1,y1,z0);
        vertex(x1,y1,z1); vertex(x0,y1,z1);
    } else if (ny < -0.5f) { // -Y bottom
        vertex(x0,y0,z1); vertex(x1,y0,z1);
        vertex(x1,y0,z0); vertex(x0,y0,z0);
    } else if (nz > 0.5f) { // +Z front
        vertex(x0,y0,z1); vertex(x1,y0,z1);
        vertex(x1,y1,z1); vertex(x0,y1,z1);
    } else if (nz < -0.5f) { // -Z back
        vertex(x1,y0,z0); vertex(x0,y0,z0);
        vertex(x0,y1,z0); vertex(x1,y1,z0);
    } else if (nx > 0.5f) { // +X right
        vertex(x1,y0,z1); vertex(x1,y0,z0);
        vertex(x1,y1,z0); vertex(x1,y1,z1);
    } else { // -X left
        vertex(x0,y0,z0); vertex(x0,y0,z1);
        vertex(x0,y1,z1); vertex(x0,y1,z0);
    }
    endShape();
}

static void drawBlock(int bx,int by,int bz) {
    uint8_t t = world[bx][by][bz];
    if (t==AIR) return;
    BlockColors bc = blockColor(t);
    float x=bx,y=by,z=bz;
    bool water = (t==WATER);

    // Only draw faces exposed to air (or water surface)
    struct Face { int dx,dy,dz; float nx,ny,nz; int col; float bright; };
    Face faces[6] = {
        { 0, 1, 0,  0, 1, 0, bc.top,  1.00f},
        { 0,-1, 0,  0,-1, 0, bc.bot,  0.50f},
        { 0, 0, 1,  0, 0, 1, bc.side, 0.80f},
        { 0, 0,-1,  0, 0,-1, bc.side, 0.80f},
        { 1, 0, 0,  1, 0, 0, bc.side, 0.65f},
        {-1, 0, 0, -1, 0, 0, bc.side, 0.65f},
    };
    for (auto& f : faces) {
        int nx=bx+f.dx, ny=by+f.dy, nz=bz+f.dz;
        bool neighbor = isOpaque(nx,ny,nz);
        if (!neighbor) {
            if (water) {
                // water: semi-transparent, only draw top if exposed to air
                if (f.ny > 0.5f) {
                    fill(64,100,200,160);
                    beginShape(QUADS);
                    vertex(x,y+BLOCK,z);       vertex(x+BLOCK,y+BLOCK,z);
                    vertex(x+BLOCK,y+BLOCK,z+BLOCK); vertex(x,y+BLOCK,z+BLOCK);
                    endShape();
                }
            } else {
                drawFace(x,y,z, f.nx,f.ny,f.nz, f.col, f.bright);
            }
        }
    }
}

// ── Raycasting for block selection ───────────────────────────────────────────
static bool raycastBlock(int &hx,int &hy,int &hz,
                          int &px2,int &py2,int &pz2) {
    // DDA raycast from player eye
    float ex=px, ey=py+1.6f, ez=pz;
    float dx= cos(pitch)*sin(yaw);
    float dy= sin(pitch);
    float dz= cos(pitch)*cos(yaw);

    int sx=(dx>0)?1:-1, sy=(dy>0)?1:-1, sz=(dz>0)?1:-1;
    int ix=(int)floor(ex), iy=(int)floor(ey), iz=(int)floor(ez);
    float txd=abs(1.0f/dx), tyd=abs(1.0f/dy), tzd=abs(1.0f/dz);
    float tx=((dx>0)?(ix+1-ex):(ex-ix))*txd;
    float ty=((dy>0)?(iy+1-ey):(ey-iy))*tyd;
    float tz=((dz>0)?(iz+1-ez):(ez-iz))*tzd;
    px2=ix; py2=iy; pz2=iz;

    for (int step=0; step<(int)(REACH/0.2f); step++) {
        if (inBounds(ix,iy,iz) && world[ix][iy][iz]!=AIR && world[ix][iy][iz]!=WATER) {
            hx=ix; hy=iy; hz=iz;
            return true;
        }
        px2=ix; py2=iy; pz2=iz;
        if (tx<ty && tx<tz) { ix+=sx; tx+=txd; }
        else if (ty<tz)      { iy+=sy; ty+=tyd; }
        else                 { iz+=sz; tz+=tzd; }
    }
    return false;
}

// ── Collision ─────────────────────────────────────────────────────────────────
static bool collidesAt(float x,float y,float z) {
    float hw=0.3f, hh=1.8f;
    for (int bx=(int)floor(x-hw); bx<=(int)floor(x+hw); bx++)
    for (int by=(int)floor(y);    by<=(int)floor(y+hh);  by++)
    for (int bz=(int)floor(z-hw); bz<=(int)floor(z+hw);  bz++)
        if (inBounds(bx,by,bz) && isSolid(bx,by,bz)) return true;
    return false;
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
    size(854, 480, P3D);
    generateWorld();

    // Spawn player above center of world
    px = WORLD_W/2.0f;
    pz = WORLD_D/2.0f;
    py = terrainHeight((int)px,(int)pz)+1;
    yaw = PI;

    frameRate(60);
}

// ── Draw ─────────────────────────────────────────────────────────────────────
void draw() {
    float dt = 1.0f/60.0f;

    // ── Mouse look ──────────────────────────────────────────────────────────
    if (cursorCaptured) {
        if (firstMouse) {
            prevMouseX = mouseX;
            prevMouseY = mouseY;
            firstMouse = false;
        }
        float dx = mouseX - prevMouseX;
        float dy = mouseY - prevMouseY;
        prevMouseX = mouseX;
        prevMouseY = mouseY;

        float sens = 0.002f;
        yaw   -= dx * sens;
        pitch += dy * sens;
        pitch = constrain(pitch, -PI/2+0.01f, PI/2-0.01f);
    }

    // ── Movement ────────────────────────────────────────────────────────────
    float fx = cos(pitch)*sin(yaw);
    float fz = cos(pitch)*cos(yaw);
    float rx = cos(yaw);
    float rz = -sin(yaw);

    float mvx=0, mvz=0;
    if (kW) { mvx+=fx; mvz+=fz; }
    if (kS) { mvx-=fx; mvz-=fz; }
    if (kA) { mvx-=rx; mvz-=rz; }
    if (kD) { mvx+=rx; mvz+=rz; }
    float ml=sqrt(mvx*mvx+mvz*mvz);
    if (ml>0.001f) { mvx=mvx/ml*SPEED*dt; mvz=mvz/ml*SPEED*dt; }

    // X movement
    if (!collidesAt(px+mvx, py, pz)) px+=mvx;
    // Z movement
    if (!collidesAt(px, py, pz+mvz)) pz+=mvz;

    // Gravity / jump
    velY += GRAVITY*dt;
    float dy2 = velY*dt;
    if (!collidesAt(px, py+dy2, pz)) { py+=dy2; onGround=false; }
    else {
        if (dy2<0) onGround=true;
        velY=0;
    }
    if (kSpace && onGround) { velY=JUMP_V; onGround=false; }

    // Clamp
    px=constrain(px,0.5f,WORLD_W-0.5f);
    pz=constrain(pz,0.5f,WORLD_D-0.5f);
    py=constrain(py,0,WORLD_H-2);

    // ── Camera setup ────────────────────────────────────────────────────────
    float eyeX=px, eyeY=py+1.6f, eyeZ2=pz;
    float lx=eyeX+cos(pitch)*sin(yaw);
    float ly=eyeY+sin(pitch);
    float lz=eyeZ2+cos(pitch)*cos(yaw);

    // Sky color (fog)
    int skyR=135,skyG=206,skyB=235;
    background(skyR,skyG,skyB);

    // Set camera manually
    camera(eyeX,eyeY,eyeZ2, lx,ly,lz, 0,1,0);
    perspective(PI/3.0f, (float)width/height, 0.05f, 80.0f);

    lights();

    // ── Draw blocks in view ─────────────────────────────────────────────────
    // Simple fog: skip blocks > fogDist away
    float fogDist = 18.0f;
    int px_i=(int)px, py_i=(int)py, pz_i=(int)pz;
    int rad = (int)fogDist+1;

    for (int bx=max(0,px_i-rad); bx<min(WORLD_W,px_i+rad); bx++)
    for (int bz=max(0,pz_i-rad); bz<min(WORLD_D,pz_i+rad); bz++) {
        float dx3=bx-px, dz3=bz-pz;
        if (dx3*dx3+dz3*dz3 > fogDist*fogDist) continue;
        for (int by=0; by<WORLD_H; by++) {
            if (world[bx][by][bz]==AIR) continue;
            drawBlock(bx,by,bz);
        }
    }

    // ── Highlight targeted block ────────────────────────────────────────────
    int hx,hy,hz,ppx,ppy,ppz;
    if (raycastBlock(hx,hy,hz,ppx,ppy,ppz)) {
        // draw wireframe outline
        stroke(0);
        strokeWeight(2);
        noFill();
        float m=0.002f;
        // draw box outline manually
        pushMatrix();
        translate(hx+0.5f,hy+0.5f,hz+0.5f);
        box(BLOCK+m*2);
        popMatrix();
        noStroke();
    }

    // ── HUD (2D overlay) ────────────────────────────────────────────────────
    // Switch to 2D
    camera();
    ortho(0,width,0,height,-1,1);
    noLights();

    // Crosshair
    stroke(255);
    strokeWeight(2);
    int cx2=width/2, cy2=height/2;
    line(cx2-10,cy2,cx2+10,cy2);
    line(cx2,cy2-10,cx2,cy2+10);

    // Block selector bar
    int barW=50, barH=50, gap=5;
    int numTypes=5;
    int barTotal=(barW+gap)*numTypes-gap;
    int barX=(width-barTotal)/2;
    int barY=height-barH-10;

    uint8_t types[5]={GRASS,DIRT,STONE,WOOD,LEAVES};
    for (int i=0;i<numTypes;i++) {
        int bx3=barX+i*(barW+gap);
        BlockColors bc=blockColor(types[i]);
        // slot bg
        fill(0,0,0,120);
        noStroke();
        rect(bx3,barY,barW,barH);
        // block color swatch
        fill(bc.side);
        rect(bx3+5,barY+5,barW-10,barH-10);
        // highlight selected
        if (types[i]==selectedBlock) {
            noFill();
            stroke(255,255,255);
            strokeWeight(3);
            rect(bx3,barY,barW,barH);
            noStroke();
        }
    }

    // Capture hint
    if (!cursorCaptured) {
        fill(255,255,255,200);
        textSize(18);
        textAlign(CENTER);
        text("Click to capture mouse | E to toggle | ESC to release",
             width/2, 30);
    }

    // Coordinates
    fill(255);
    textSize(12);
    textAlign(LEFT);
    text("X:"+nf(px,1,1)+" Y:"+nf(py,1,1)+" Z:"+nf(pz,1,1), 8, 8);
    text("FPS:" + std::to_string(frameCount), 8, 24);
}

// ── Input ─────────────────────────────────────────────────────────────────────
void keyPressed() {
    if (key=='w'||key=='W') kW=true;
    if (key=='s'||key=='S') kS=true;
    if (key=='a'||key=='A') kA=true;
    if (key=='d'||key=='D') kD=true;
    if (key==' ')            kSpace=true;
    if (key=='e'||key=='E') {
        cursorCaptured=!cursorCaptured;
        firstMouse=true;
        if (cursorCaptured) noCursor();
        else cursor();
    }
    if (key=='1') selectedBlock=GRASS;
    if (key=='2') selectedBlock=DIRT;
    if (key=='3') selectedBlock=STONE;
    if (key=='4') selectedBlock=WOOD;
    if (key=='5') selectedBlock=LEAVES;
    if (key==ESC) {
        cursorCaptured=false;
        firstMouse=true;
        cursor();
        key=0; // prevent ESC from closing sketch
    }
}
void keyReleased() {
    if (key=='w'||key=='W') kW=false;
    if (key=='s'||key=='S') kS=false;
    if (key=='a'||key=='A') kA=false;
    if (key=='d'||key=='D') kD=false;
    if (key==' ')            kSpace=false;
}
void mousePressed() {
    if (!cursorCaptured) {
        // First click just captures cursor
        cursorCaptured=true;
        firstMouse=true;
        noCursor();
        return;
    }
    int hx,hy,hz,ppx,ppy,ppz;
    bool hit=raycastBlock(hx,hy,hz,ppx,ppy,ppz);
    if (!hit) return;

    if (mouseButton==LEFT) {
        // Break block
        world[hx][hy][hz]=AIR;
    } else if (mouseButton==RIGHT) {
        // Place block at the face we hit (previous position in ray)
        if (inBounds(ppx,ppy,ppz) && world[ppx][ppy][ppz]==AIR) {
            // Don't place inside player
            float dpx=ppx+0.5f-px, dpy=ppy-py, dpz=ppz+0.5f-pz;
            if (abs(dpx)>0.4f || dpy>1.9f || dpy<-0.1f || abs(dpz)>0.4f)
                world[ppx][ppy][ppz]=selectedBlock;
        }
    }
}
void mouseMoved() {
    // handled in draw()
}
void mouseDragged() {
    // handled in draw()
}
