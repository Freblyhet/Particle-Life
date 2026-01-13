#include "rendering/Renderer.h"
#include "rendering/ShaderManager.h"
#include <iostream>
#include <memory>
#include <cmath>

Renderer::Renderer() : VAO(0), VBO(0), shaderManager(nullptr) {
    // Initialize color palette
    colors = {
        {1.0f, 0.2f, 0.2f},  // Red
        {0.2f, 1.0f, 0.3f},  // Green
        {0.3f, 0.5f, 1.0f},  // Blue
        {1.0f, 0.9f, 0.2f},  // Yellow
        {1.0f, 0.3f, 0.8f},  // Pink
        {0.3f, 1.0f, 1.0f},  // Cyan
        {1.0f, 0.6f, 0.2f},  // Orange
        {0.7f, 0.3f, 1.0f},  // Purple
    };
}

Renderer::Renderer(ShaderManager& shaderMgr) : VAO(0), VBO(0), shaderManager(&shaderMgr) {
    // Initialize color palette
    colors = {
        {1.0f, 0.2f, 0.2f},  // Red
        {0.2f, 1.0f, 0.3f},  // Green
        {0.3f, 0.5f, 1.0f},  // Blue
        {1.0f, 0.9f, 0.2f},  // Yellow
        {1.0f, 0.3f, 0.8f},  // Pink
        {0.3f, 1.0f, 1.0f},  // Cyan
        {1.0f, 0.6f, 0.2f},  // Orange
        {0.7f, 0.3f, 1.0f},  // Purple
    };
}

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::initialize() {
    // Create shader manager if we don't have one
    if (!shaderManager) {
        ownedShaderManager = std::make_unique<ShaderManager>();
        shaderManager = ownedShaderManager.get();
    }
    
    // Embedded shaders for particle rendering
    const std::string vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec3 aColor;
        out vec3 vColor;
        uniform float uPointSize;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            gl_PointSize = uPointSize;
            vColor = aColor;
        }
    )";
    
    const std::string fragmentShader = R"(
        #version 330 core
        in vec3 vColor;
        out vec4 FragColor;
        uniform bool uEnableGlow;
        void main() {
            vec2 coord = gl_PointCoord - vec2(0.5);
            float dist = length(coord);
            if (dist > 0.5) discard;
            float alpha = smoothstep(0.5, 0.35, dist);
            
            vec3 color = vColor;
            if (uEnableGlow) {
                float glow = exp(-dist * 3.0);
                color = mix(vColor, vec3(1.0), glow * 0.4);
            }
            
            FragColor = vec4(color, alpha);
        }
    )";
    
    if (!shaderManager->loadShadersFromSource(vertexShader, fragmentShader)) {
        std::cerr << "Failed to load particle shaders" << std::endl;
        return false;
    }
    
    // Create VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Position attribute (location 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Enable point size and blending
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return true;
}

void Renderer::cleanup() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (shaderManager && ownedShaderManager) {
        shaderManager->cleanup();
        ownedShaderManager.reset();
        shaderManager = nullptr;
    }
}

void Renderer::setupFrame() {
    if (config.enableTrails) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f - config.trailIntensity);
        glClear(GL_COLOR_BUFFER_BIT);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    } else {
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void Renderer::renderParticles(const std::vector<Particle>& particles) {
    if (!shaderManager || particles.empty()) return;
    
    // Prepare vertex data - use static to avoid reallocations
    static std::vector<float> vertexData;
    vertexData.clear();
    vertexData.reserve(particles.size() * 5);
    
    static float time = 0.0f;
    time += 0.016f; // Approximate frame time for animation
    
    if (config.colorBySpeed) {
        // Color by velocity - with speed calculations
        const float invMaxSpeed = 1.0f / config.maxSpeed;
        for (const auto& p : particles) {
            vertexData.push_back(p.x);
            vertexData.push_back(p.y);
            
            // Color by velocity magnitude
            float speed = std::sqrt(p.vx * p.vx + p.vy * p.vy);
            float t = std::min(speed * invMaxSpeed, 1.0f);
            
            glm::vec3 color;
            // More vibrant color transition: blue -> cyan -> green -> yellow -> red
            if (t < 0.25f) {
                float localT = t * 4.0f; // Precalculate division
                color = glm::mix(glm::vec3(0.2f, 0.2f, 1.0f), glm::vec3(0.2f, 1.0f, 1.0f), localT);
            } else if (t < 0.5f) {
                float localT = (t - 0.25f) * 4.0f;
                color = glm::mix(glm::vec3(0.2f, 1.0f, 1.0f), glm::vec3(0.2f, 1.0f, 0.2f), localT);
            } else if (t < 0.75f) {
                float localT = (t - 0.5f) * 4.0f;
                color = glm::mix(glm::vec3(0.2f, 1.0f, 0.2f), glm::vec3(1.0f, 1.0f, 0.2f), localT);
            } else {
                float localT = (t - 0.75f) * 4.0f;
                color = glm::mix(glm::vec3(1.0f, 1.0f, 0.2f), glm::vec3(1.0f, 0.2f, 0.2f), localT);
            }
            
            vertexData.push_back(color.r);
            vertexData.push_back(color.g);
            vertexData.push_back(color.b);
        }
    } else {
        // Simple type-based coloring (faster)
        const size_t colorCount = colors.size();
        for (const auto& p : particles) {
            vertexData.push_back(p.x);
            vertexData.push_back(p.y);
            
            const glm::vec3& color = colors[p.type % colorCount];
            vertexData.push_back(color.r);
            vertexData.push_back(color.g);
            vertexData.push_back(color.b);
        }
    }
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), 
                 vertexData.data(), GL_DYNAMIC_DRAW);
    
    // Render particles with dynamic sizing
    shaderManager->use();
    
    float particleSize = config.particleSize;
    
    // Apply size variation based on speed or pulsation if enabled
    if (config.sizeBySpeed || config.enablePulsation) {
        // Apply global pulsation
        if (config.enablePulsation) {
            float pulsate = 1.0f + config.pulsationAmount * std::sin(time * config.pulsationSpeed);
            particleSize *= pulsate;
        }
    }
    
    shaderManager->setFloat("uPointSize", particleSize);
    shaderManager->setBool("uEnableGlow", config.enableGlow);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, particles.size());
}

void Renderer::setViewport(int width, int height) {
    glViewport(0, 0, width, height);
}