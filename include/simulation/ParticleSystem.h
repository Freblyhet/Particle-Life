#pragma once

#include "simulation/Particle.h"
#include "simulation/SpatialHash.h"
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <chrono>

class ParticleSystem {
public:
    enum BoundaryMode { BOUNCE, WRAP, KILL };
    
    struct PerformanceMetrics {
        float updateTimeMs = 0.0f;
        float renderTimeMs = 0.0f;
        int forceCalculations = 0;
        int spatialQueries = 0;
        float averageFPS = 0.0f;
        
        void reset() {
            forceCalculations = 0;
            spatialQueries = 0;
        }
    };
    
    struct Config {
        // Core parameters
        int numTypes = 4;
        int particlesPerType = 200;
        
        // Physics parameters
        float interactionRadius = 0.25f;
        float forceFactor = 0.3f;
        float friction = 0.98f;
        float maxSpeed = 0.01f;
        bool useSpatialHash = true;
        
        // Boundary mode
        BoundaryMode boundaryMode = WRAP;
        
        // Advanced features removed for simplicity

        
        // State
        bool paused = false;
        float timeScale = 1.0f;
        
        // Mouse interaction
        float mouseX = -10.0f;
        float mouseY = -10.0f;
        bool mousePressed = false;
        // Simplified mouse parameters
        float mouseRadius = 0.25f;
        float mouseForce = 0.1f;
        
        // Interactive editing
        bool enableParticleSpawning = true;
        int spawnParticleType = 0;
        float spawnRadius = 0.1f;
        int spawnCount = 5;
        int mouseMode = 0; // 0 = spawn, 1 = interact
    };

private:
    std::vector<Particle> particles;
    std::vector<std::vector<float>> forces;
    SpatialHash spatialHash;
    std::mt19937 rng;
    Config config;
    PerformanceMetrics metrics;
    
    // Performance tracking
    std::chrono::high_resolution_clock::time_point lastUpdateTime;
    std::vector<float> fpsHistory;
    const size_t maxFpsHistory = 60;

    // Helper functions
    float wrapCoord(float x) const;
    glm::vec2 getWrappedDelta(const glm::vec2& from, const glm::vec2& to) const;
    float calculateForce(float dist, float attraction) const;
    
public:
    ParticleSystem();
    ~ParticleSystem() = default;
    
    // Configuration
    Config& getConfig() { return config; }
    const Config& getConfig() const { return config; }
    
    // Performance metrics
    const PerformanceMetrics& getMetrics() const { return metrics; }
    
    // Force matrix management
    std::vector<std::vector<float>>& getForces() { return forces; }
    const std::vector<std::vector<float>>& getForces() const { return forces; }
    void randomizeForces();
    void resizeForceMatrix();
    
    // Particle management
    const std::vector<Particle>& getParticles() const { return particles; }
    std::vector<Particle>& getParticles() { return particles; }
    void createParticles();
    void resetSimulation(bool randomForces = false);
    
    // Dynamic particle management
    void addParticles(int count, int type = -1); // -1 = random type
    void removeParticles(int count);
    void setParticleCount(int totalCount);
    void setNumTypes(int numTypes);
    
    // Force matrix utilities
    void setForce(int fromType, int toType, float force);
    float getForce(int fromType, int toType) const;
    
    // Presets
    void loadPreset(const std::string& name);
    
    // Simulation
    void update(float deltaTime);
    
    // Utility methods
    int getParticleCount() const { return particles.size(); }
    
    // Mouse interaction
    void setMousePosition(float x, float y);
    void setMousePressed(bool pressed);
    void spawnParticlesAtMouse(int count, int type);
    void removeParticlesAtMouse(float radius);
};