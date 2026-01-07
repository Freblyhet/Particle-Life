#pragma once

struct Particle {
    float x, y;
    float vx, vy;
    float fx, fy; // Force accumulation for multi-threading
    int type;
    
    Particle() : x(0.0f), y(0.0f), vx(0.0f), vy(0.0f), fx(0.0f), fy(0.0f), type(0) {}
    Particle(float x, float y, float vx, float vy, int type) 
        : x(x), y(y), vx(vx), vy(vy), fx(0.0f), fy(0.0f), type(type) {}
};