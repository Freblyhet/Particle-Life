#include "simulation/ParticleSystem.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ParticleSystem::ParticleSystem() : spatialHash(0.3f) {
    rng.seed(std::random_device{}());
    
    // Initialize multi-threading
    hardwareThreads = std::thread::hardware_concurrency();
    if (hardwareThreads == 0) hardwareThreads = 4; // fallback
    
    resizeForceMatrix();
    createParticles();
    lastUpdateTime = std::chrono::high_resolution_clock::now();
    
    std::cout << "ParticleSystem initialized with " << hardwareThreads << " hardware threads detected" << std::endl;
}

float ParticleSystem::wrapCoord(float x) const {
    const float boundary = 1.0f;
    if (x < -boundary) return x + 2.0f * boundary;
    if (x > boundary) return x - 2.0f * boundary;
    return x;
}

glm::vec2 ParticleSystem::getWrappedDelta(const glm::vec2& from, const glm::vec2& to) const {
    glm::vec2 delta = to - from;
    
    // Wrap x
    if (delta.x > 1.0f) delta.x -= 2.0f;
    else if (delta.x < -1.0f) delta.x += 2.0f;
    
    // Wrap y
    if (delta.y > 1.0f) delta.y -= 2.0f;
    else if (delta.y < -1.0f) delta.y += 2.0f;
    
    return delta;
}

float ParticleSystem::calculateForce(float dist, float attraction) const {
    const float beta = 0.3f;
    if (dist < beta) {
        return dist / beta - 1.0f;
    } else if (dist < 1.0f) {
        return attraction * (1.0f - std::abs(2.0f * dist - 1.0f - beta) / (1.0f - beta));
    }
    return 0.0f;
}

void ParticleSystem::randomizeForces() {
    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
    forces.clear();
    forces.resize(config.numTypes, std::vector<float>(config.numTypes, 0.0f));
    
    for (int i = 0; i < config.numTypes; ++i) {
        for (int j = 0; j < config.numTypes; ++j) {
            forces[i][j] = dist(rng);
        }
    }
}

void ParticleSystem::resizeForceMatrix() {
    forces.resize(config.numTypes);
    for (auto& row : forces) {
        row.resize(config.numTypes, 0.0f);
    }
}

void ParticleSystem::createParticles() {
    particles.clear();
    
    std::uniform_real_distribution<float> posDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> velDist(-0.0005f, 0.0005f);
    
    for (int type = 0; type < config.numTypes; ++type) {
        for (int i = 0; i < config.particlesPerType; ++i) {
            Particle p;
            p.x = posDist(rng);
            p.y = posDist(rng);
            p.vx = velDist(rng);
            p.vy = velDist(rng);
            p.type = type;
            particles.push_back(p);
        }
    }
    
    std::cout << "Created " << particles.size() << " particles with " 
              << config.numTypes << " types" << std::endl;
}

void ParticleSystem::resetSimulation(bool randomForces) {
    if (randomForces) {
        randomizeForces();
    } else {
        resizeForceMatrix();
    }
    createParticles();
}

void ParticleSystem::loadPreset(const std::string& name) {
    if (name == "Orbits") {
        config.numTypes = 4;
        forces = {
            { 0.0f, -0.3f,  0.4f, -0.2f},
            { 0.5f,  0.0f, -0.2f,  0.3f},
            {-0.1f,  0.4f,  0.0f,  0.2f},
            { 0.3f, -0.2f,  0.5f,  0.0f}
        };
    } else if (name == "Chaos") {
        config.numTypes = 5;
        forces = {
            { 0.0f,  0.4f, -0.5f,  0.2f, -0.3f},
            {-0.4f,  0.0f,  0.3f, -0.4f,  0.2f},
            { 0.5f, -0.3f,  0.0f,  0.4f, -0.3f},
            {-0.2f,  0.5f, -0.3f,  0.0f,  0.3f},
            { 0.3f, -0.2f,  0.4f, -0.4f,  0.0f}
        };
    } else if (name == "Balance") {
        config.numTypes = 3;
        forces = {
            { 0.0f, -0.3f,  0.3f},
            { 0.3f,  0.0f, -0.3f},
            {-0.3f,  0.3f,  0.0f}
        };
    } else if (name == "Swirls") {
        config.numTypes = 4;
        forces = {
            { 0.0f,  0.5f, -0.4f,  0.2f},
            {-0.5f,  0.0f,  0.4f, -0.3f},
            { 0.4f, -0.4f,  0.0f,  0.3f},
            {-0.2f,  0.3f, -0.3f,  0.0f}
        };
    } else if (name == "Snakes") {
        config.numTypes = 6;
        forces.clear();
        forces.resize(6, std::vector<float>(6, 0.0f));
        for (int i = 0; i < 6; ++i) {
            forces[i][(i + 1) % 6] = 0.5f;
            forces[i][(i + 2) % 6] = -0.3f;
            forces[i][(i + 5) % 6] = -0.2f;
        }
    }
    createParticles();
    std::cout << "Loaded preset: " << name << std::endl;
}

void ParticleSystem::update() {
    if (config.paused) return;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    metrics.reset();
    
    const float dt = 0.016f * config.timeScale;
    
    // Build spatial hash
    if (config.useSpatialHash) {
        spatialHash.clear();
        for (size_t i = 0; i < particles.size(); ++i) {
            spatialHash.insert(i, particles[i].x, particles[i].y);
        }
    }
    
    // Calculate forces
    std::vector<float> fx(particles.size(), 0.0f);
    std::vector<float> fy(particles.size(), 0.0f);
    
    for (size_t i = 0; i < particles.size(); ++i) {
        std::vector<int> neighbors;
        
        if (config.useSpatialHash) {
            neighbors = spatialHash.query(particles[i].x, particles[i].y, config.interactionRadius);
            metrics.spatialQueries++;
        } else {
            neighbors.resize(particles.size());
            for (size_t j = 0; j < particles.size(); ++j) {
                neighbors[j] = j;
            }
        }
        
        // Particle interactions
        for (int j : neighbors) {
            if (i == static_cast<size_t>(j)) continue;
            
            glm::vec2 delta;
            if (config.boundaryMode == WRAP) {
                delta = getWrappedDelta(
                    glm::vec2(particles[i].x, particles[i].y),
                    glm::vec2(particles[j].x, particles[j].y)
                );
            } else {
                delta = glm::vec2(
                    particles[j].x - particles[i].x,
                    particles[j].y - particles[i].y
                );
            }
            
            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            
            if (dist > 0.001f && dist < config.interactionRadius) {
                float normDist = dist / config.interactionRadius;
                float attraction = forces[particles[i].type][particles[j].type];
                float force = calculateForce(normDist, attraction) * config.forceFactor;
                
                fx[i] += (delta.x / dist) * force;
                fy[i] += (delta.y / dist) * force;
                metrics.forceCalculations++;
            }
        }
        
        // Gravity effect
        if (config.enableGravity) {
            glm::vec2 toCenter(config.gravityCenter.x - particles[i].x,
                              config.gravityCenter.y - particles[i].y);
            float dist = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
            if (dist > 0.001f) {
                float gravForce = config.gravityStrength / (dist * dist + 0.1f);
                fx[i] += (toCenter.x / dist) * gravForce;
                fy[i] += (toCenter.y / dist) * gravForce;
            }
        }
        
        // Vortex effect
        if (config.enableVortex) {
            glm::vec2 toCenter(config.vortexCenter.x - particles[i].x,
                              config.vortexCenter.y - particles[i].y);
            float dist = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
            if (dist > 0.001f) {
                float vortexForce = config.vortexStrength / (dist + 0.1f);
                fx[i] += (-toCenter.y / dist) * vortexForce;
                fy[i] += (toCenter.x / dist) * vortexForce;
            }
        }
        
        // Mouse interaction
        if (config.mousePressed) {
            float dx = config.mouseX - particles[i].x;
            float dy = config.mouseY - particles[i].y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist < config.mouseRadius && dist > 0.001f) {
                float strength = (1.0f - dist / config.mouseRadius);
                fx[i] -= (dx / dist) * config.mouseForce * strength;
                fy[i] -= (dy / dist) * config.mouseForce * strength;
            }
        }
    }
    
    // Update particles
    std::vector<size_t> toRemove;
    
    for (size_t i = 0; i < particles.size(); ++i) {
        particles[i].vx += fx[i] * dt;
        particles[i].vy += fy[i] * dt;
        
        particles[i].vx *= config.friction;
        particles[i].vy *= config.friction;
        
        float speed = std::sqrt(particles[i].vx * particles[i].vx + particles[i].vy * particles[i].vy);
        if (speed > config.maxSpeed) {
            particles[i].vx = (particles[i].vx / speed) * config.maxSpeed;
            particles[i].vy = (particles[i].vy / speed) * config.maxSpeed;
        }
        
        particles[i].x += particles[i].vx;
        particles[i].y += particles[i].vy;
        
        // Boundary handling
        const float boundary = 0.98f;
        const float damping = 0.8f;
        
        if (config.boundaryMode == WRAP) {
            particles[i].x = wrapCoord(particles[i].x);
            particles[i].y = wrapCoord(particles[i].y);
        } else if (config.boundaryMode == BOUNCE) {
            if (particles[i].x < -boundary) {
                particles[i].x = -boundary;
                particles[i].vx *= -damping;
            } else if (particles[i].x > boundary) {
                particles[i].x = boundary;
                particles[i].vx *= -damping;
            }
            
            if (particles[i].y < -boundary) {
                particles[i].y = -boundary;
                particles[i].vy *= -damping;
            } else if (particles[i].y > boundary) {
                particles[i].y = boundary;
                particles[i].vy *= -damping;
            }
        } else if (config.boundaryMode == KILL) {
            if (particles[i].x < -boundary || particles[i].x > boundary ||
                particles[i].y < -boundary || particles[i].y > boundary) {
                toRemove.push_back(i);
            }
        }
    }
    
    // Remove out-of-bounds particles (in reverse order)
    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
        particles.erase(particles.begin() + *it);
    }
    
    // Update performance metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    metrics.updateTimeMs = duration.count() / 1000.0f;
    
    // Update FPS calculation
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastUpdateTime);
    lastUpdateTime = currentTime;
    
    if (frameDuration.count() > 0) {
        float currentFPS = 1000000.0f / frameDuration.count();
        fpsHistory.push_back(currentFPS);
        
        if (fpsHistory.size() > maxFpsHistory) {
            fpsHistory.erase(fpsHistory.begin());
        }
        
        // Calculate average FPS
        float sum = 0.0f;
        for (float fps : fpsHistory) {
            sum += fps;
        }
        metrics.averageFPS = sum / fpsHistory.size();
    }
}

void ParticleSystem::setMousePosition(float x, float y) {
    config.mouseX = x;
    config.mouseY = y;
}

void ParticleSystem::setMousePressed(bool pressed) {
    config.mousePressed = pressed;
}

void ParticleSystem::spawnParticlesAtMouse(int count, int type) {
    if (config.mouseX < -1.0f || config.mouseX > 1.0f || 
        config.mouseY < -1.0f || config.mouseY > 1.0f) return;
    
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * M_PI);
    std::uniform_real_distribution<float> radiusDist(0.0f, config.spawnRadius);
    std::uniform_real_distribution<float> velDist(-0.001f, 0.001f);
    
    for (int i = 0; i < count; ++i) {
        Particle p;
        
        // Random position around mouse
        float angle = angleDist(rng);
        float radius = radiusDist(rng);
        p.x = config.mouseX + radius * std::cos(angle);
        p.y = config.mouseY + radius * std::sin(angle);
        
        // Small random velocity
        p.vx = velDist(rng);
        p.vy = velDist(rng);
        p.type = type % config.numTypes;
        
        particles.push_back(p);
    }
}

void ParticleSystem::removeParticlesAtMouse(float radius) {
    if (config.mouseX < -1.0f || config.mouseX > 1.0f || 
        config.mouseY < -1.0f || config.mouseY > 1.0f) return;
    
    std::vector<size_t> toRemove;
    
    for (size_t i = 0; i < particles.size(); ++i) {
        float dx = particles[i].x - config.mouseX;
        float dy = particles[i].y - config.mouseY;
        float dist = std::sqrt(dx * dx + dy * dy);
        
        if (dist <= radius) {
            toRemove.push_back(i);
        }
    }
    
    // Remove in reverse order to maintain indices
    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
        particles.erase(particles.begin() + *it);
    }
}

// ========================================
// MULTI-THREADING IMPLEMENTATIONS
// ========================================

void ParticleSystem::calculateForcesForRange(size_t start, size_t end) {
    // Thread-local force accumulation
    std::vector<float> fx_local(particles.size(), 0.0f);
    std::vector<float> fy_local(particles.size(), 0.0f);
    int forceCalcs = 0;
    int spatialQueries = 0;
    
    for (size_t i = start; i < end && i < particles.size(); ++i) {
        std::vector<int> neighbors;
        
        if (config.useSpatialHash) {
            neighbors = spatialHash.query(particles[i].x, particles[i].y, config.interactionRadius);
            spatialQueries++;
        } else {
            neighbors.resize(particles.size());
            for (size_t j = 0; j < particles.size(); ++j) {
                neighbors[j] = j;
            }
        }
        
        // Particle interactions
        for (int j : neighbors) {
            if (i == static_cast<size_t>(j)) continue;
            
            glm::vec2 delta;
            if (config.boundaryMode == WRAP) {
                delta = getWrappedDelta(
                    glm::vec2(particles[i].x, particles[i].y),
                    glm::vec2(particles[j].x, particles[j].y)
                );
            } else {
                delta = glm::vec2(
                    particles[j].x - particles[i].x,
                    particles[j].y - particles[i].y
                );
            }
            
            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            
            if (dist > 0.001f && dist < config.interactionRadius) {
                float normDist = dist / config.interactionRadius;
                float attraction = forces[particles[i].type][particles[j].type];
                float force = calculateForce(normDist, attraction) * config.forceFactor;
                
                fx_local[i] += (delta.x / dist) * force;
                fy_local[i] += (delta.y / dist) * force;
                forceCalcs++;
            }
        }
        
        // Gravity effect
        if (config.enableGravity) {
            glm::vec2 toCenter(config.gravityCenter.x - particles[i].x,
                              config.gravityCenter.y - particles[i].y);
            float dist = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
            if (dist > 0.001f) {
                float gravForce = config.gravityStrength / (dist * dist + 0.1f);
                fx_local[i] += (toCenter.x / dist) * gravForce;
                fy_local[i] += (toCenter.y / dist) * gravForce;
            }
        }
        
        // Vortex effect
        if (config.enableVortex) {
            glm::vec2 toCenter(config.vortexCenter.x - particles[i].x,
                              config.vortexCenter.y - particles[i].y);
            float dist = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
            if (dist > 0.001f) {
                float vortexForce = config.vortexStrength / (dist + 0.1f);
                fx_local[i] += (-toCenter.y / dist) * vortexForce;
                fy_local[i] += (toCenter.x / dist) * vortexForce;
            }
        }
        
        // Mouse interaction
        if (config.mousePressed) {
            float dx = config.mouseX - particles[i].x;
            float dy = config.mouseY - particles[i].y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist < config.mouseRadius && dist > 0.001f) {
                float strength = (1.0f - dist / config.mouseRadius);
                fx_local[i] -= (dx / dist) * config.mouseForce * strength;
                fy_local[i] -= (dy / dist) * config.mouseForce * strength;
            }
        }
    }
    
    // Thread-safely accumulate results
    {
        std::lock_guard<std::mutex> lock(particleMutex);
        for (size_t i = start; i < end && i < particles.size(); ++i) {
            particles[i].fx += fx_local[i];
            particles[i].fy += fy_local[i];
        }
        metrics.forceCalculations += forceCalcs;
        metrics.spatialQueries += spatialQueries;
    }
}

void ParticleSystem::updatePositionsForRange(size_t start, size_t end) {
    const float dt = 0.016f * config.timeScale;
    
    for (size_t i = start; i < end && i < particles.size(); ++i) {
        // Update velocity
        particles[i].vx = (particles[i].vx + particles[i].fx * dt) * config.friction;
        particles[i].vy = (particles[i].vy + particles[i].fy * dt) * config.friction;
        
        // Limit speed
        float speed = std::sqrt(particles[i].vx * particles[i].vx + particles[i].vy * particles[i].vy);
        if (speed > config.maxSpeed) {
            particles[i].vx = (particles[i].vx / speed) * config.maxSpeed;
            particles[i].vy = (particles[i].vy / speed) * config.maxSpeed;
        }
        
        // Update position
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        
        // Handle boundaries
        if (config.boundaryMode == BOUNCE) {
            if (particles[i].x < -1.0f || particles[i].x > 1.0f) {
                particles[i].vx *= -0.8f;
                particles[i].x = std::max(-1.0f, std::min(1.0f, particles[i].x));
            }
            if (particles[i].y < -1.0f || particles[i].y > 1.0f) {
                particles[i].vy *= -0.8f;
                particles[i].y = std::max(-1.0f, std::min(1.0f, particles[i].y));
            }
        } else if (config.boundaryMode == WRAP) {
            particles[i].x = wrapCoord(particles[i].x);
            particles[i].y = wrapCoord(particles[i].y);
        }
    }
}

// ========================================
// DYNAMIC PARTICLE MANAGEMENT
// ========================================

void ParticleSystem::addParticles(int count, int type) {
    std::uniform_real_distribution<float> posDist(-0.8f, 0.8f);
    std::uniform_real_distribution<float> velDist(-0.001f, 0.001f);
    std::uniform_int_distribution<int> typeDist(0, config.numTypes - 1);
    
    for (int i = 0; i < count; ++i) {
        int particleType = (type == -1) ? typeDist(rng) : type % config.numTypes;
        particles.emplace_back(
            posDist(rng), posDist(rng),    // position
            velDist(rng), velDist(rng),    // velocity
            particleType                   // type
        );
    }
}

void ParticleSystem::removeParticles(int count) {
    if (count >= static_cast<int>(particles.size())) {
        particles.clear();
        return;
    }
    
    // Remove from the end for efficiency
    particles.resize(particles.size() - count);
}

void ParticleSystem::setParticleCount(int totalCount) {
    if (totalCount < 0) totalCount = 0;
    
    int currentCount = static_cast<int>(particles.size());
    if (totalCount > currentCount) {
        addParticles(totalCount - currentCount);
    } else if (totalCount < currentCount) {
        removeParticles(currentCount - totalCount);
    }
}

void ParticleSystem::setParticleTypes(int numTypes) {
    if (numTypes < 1) numTypes = 1;
    if (numTypes > 8) numTypes = 8; // Reasonable limit
    
    config.numTypes = numTypes;
    resizeForceMatrix();
    
    // Update existing particles to valid types
    for (auto& particle : particles) {
        if (particle.type >= numTypes) {
            particle.type = particle.type % numTypes;
        }
    }
}

void ParticleSystem::setForce(int fromType, int toType, float force) {
    if (fromType >= 0 && fromType < config.numTypes && 
        toType >= 0 && toType < config.numTypes) {
        forces[fromType][toType] = force;
    }
}

float ParticleSystem::getForce(int fromType, int toType) const {
    if (fromType >= 0 && fromType < config.numTypes && 
        toType >= 0 && toType < config.numTypes) {
        return forces[fromType][toType];
    }
    return 0.0f;
}