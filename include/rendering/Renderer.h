#pragma once

#include "simulation/Particle.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class ShaderManager; // Forward declaration

class Renderer {
public:
    struct Config {
        bool enableTrails = false;
        float trailIntensity = 0.95f;
        bool enableGlow = true;
        float particleSize = 8.0f;
        bool colorBySpeed = false;
        bool showVelocityVectors = false;
        float maxSpeed = 0.01f; // For speed-based coloring
        
        // Enhanced rendering
        bool sizeBySpeed = false;
        float minParticleSize = 4.0f;
        float maxParticleSize = 16.0f;
        bool enablePulsation = false;
        float pulsationSpeed = 2.0f;
        float pulsationAmount = 0.3f;
        
        // Visual helpers
        bool showGrid = false;
        bool showCenter = false;
        bool useTypeColors = true;
        bool useVelocityColors = false;
    };

private:
    GLuint VAO, VBO;
    std::unique_ptr<ShaderManager> ownedShaderManager;
    ShaderManager* shaderManager;
    Config config;
    
    // Predefined color palette
    std::vector<glm::vec3> colors;

public:
    Renderer();
    Renderer(ShaderManager& shaderMgr);
    ~Renderer();
    
    bool initialize();
    void cleanup();
    
    Config& getConfig() { return config; }
    const Config& getConfig() const { return config; }
    
    void setupFrame();
    void renderParticles(const std::vector<Particle>& particles);
    void present();
    
    // Viewport management
    void setViewport(int width, int height);
    
    // Color utilities
    const std::vector<glm::vec3>& getColors() const { return colors; }
};