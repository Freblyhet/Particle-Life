#pragma once

struct Particle {
    float x, y;
    float vx, vy;
    int type;
    
    Particle() : x(0.0f), y(0.0f), vx(0.0f), vy(0.0f), type(0) {}
    Particle(float x, float y, float vx, float vy, int type) 
        : x(x), y(y), vx(vx), vy(vy), type(type) {}
};
