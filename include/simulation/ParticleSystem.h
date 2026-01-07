#pragma once

#include "simulation/Particle.h"
#include "simulation/SpatialHash.h"
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>

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
        BoundaryMode boundaryMode = BOUNCE;
        
        // Advanced features
        bool enableGravity = false;
        float gravityStrength = 0.0f;
        glm::vec2 gravityCenter{0.0f, 0.0f};
        
        bool enableVortex = false;
        float vortexStrength = 0.0f;
        glm::vec2 vortexCenter{0.0f, 0.0f};
        
        // State
        bool paused = false;
        float timeScale = 1.0f;
        
        // Mouse interaction
        float mouseX = -10.0f;
        float mouseY = -10.0f;
        bool mousePressed = false;
        float mouseRadius = 0.3f;
        float mouseForce = 0.005f;
        
        // Interactive editing
        bool enableParticleSpawning = true;
        int spawnParticleType = 0;
        float spawnRadius = 0.1f;
        int spawnCount = 5;
        float removeRadius = 50.0f;
        int mouseMode = 0; // 0 = spawn, 1 = remove
        
        // Physics scaling
        float forceScale = 1.0f;
        
        // World bounds (for UI display)
        float worldWidth = 800.0f;
        float worldHeight = 600.0f;
        
        // Multi-threading configuration
        bool enableMultiThreading = true;
        int numThreads = 0; // 0 = auto-detect hardware threads
        int minParticlesForThreading = 100; // Don't use threading below this count
        
        // Renamed for better UX
        bool isPaused = false; // Better than 'paused' for checkbox logic
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
    const size_t maxFpsHistory = 60; // Keep 1 second of history at 60 FPS

    // Multi-threading support
    int hardwareThreads;
    std::atomic<int> activeThreads{0};
    
    // Thread synchronization
    std::mutex particleMutex;

    // Helper functions
    float wrapCoord(float x) const;
    glm::vec2 getWrappedDelta(const glm::vec2& from, const glm::vec2& to) const;
    float calculateForce(float dist, float attraction) const;
    
    // Multi-threaded force calculation helpers
    void calculateForcesForRange(size_t start, size_t end);
    void updatePositionsForRange(size_t start, size_t end);

public:
    ParticleSystem();
    ~ParticleSystem() = default;
    
    // Configuration
    Config& getConfig() { return config; }
    const Config& getConfig() const { return config; }
    
    // Performance metrics
    const PerformanceMetrics& getMetrics() const { return metrics; }
    const PerformanceMetrics& getPerformanceMetrics() const { return metrics; } // Alias for UI
    
    // Force matrix management
    std::vector<std::vector<float>>& getForces() { return forces; }
    const std::vector<std::vector<float>>& getForces() const { return forces; }
    std::vector<std::vector<float>>& getForceMatrix() { return forces; } // Alias for UI
    const std::vector<std::vector<float>>& getForceMatrix() const { return forces; } // Alias for UI
    void randomizeForces();
    void generateRandomForces() { randomizeForces(); } // Alias for UI
    void resizeForceMatrix();
    
    // Particle management
    const std::vector<Particle>& getParticles() const { return particles; }
    std::vector<Particle>& getParticles() { return particles; } // Non-const version for UI
    void createParticles();
    void reset() { resetSimulation(true); } // Alias for UI
    void resetSimulation(bool randomForces = false);
    
    // Dynamic particle management
    void addParticles(int count, int type = -1); // -1 = random type
    void removeParticles(int count);
    void setParticleCount(int totalCount);
    void setParticleTypes(int numTypes);
    
    // Force matrix utilities
    void setForce(int fromType, int toType, float force);
    float getForce(int fromType, int toType) const;
    
    // Presets
    void loadPreset(const std::string& name);
    
    // Simulation
    void update();
    
    // Mouse interaction
    void setMousePosition(float x, float y);
    void setMousePressed(bool pressed);
    void spawnParticlesAtMouse(int count, int type);
    void removeParticlesAtMouse(float radius);
};