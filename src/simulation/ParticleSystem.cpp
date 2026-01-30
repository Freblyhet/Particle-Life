#include "simulation/ParticleSystem.h"
#include <iostream>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ParticleSystem::ParticleSystem() : spatialHash(0.3f) {
    rng.seed(std::random_device{}());
    
    resizeForceMatrix();
    createParticles();
    lastUpdateTime = std::chrono::high_resolution_clock::now();
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
        // Only apply repulsion if there's actual attraction
        // This prevents clustering when forces are zero
        return attraction * (dist / beta - 1.0f);
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

void ParticleSystem::update(float deltaTime) {
    if (config.paused) return;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    metrics.reset();
    
    // Convert real time to normalized simulation time
    // The simulation was designed with dt=1.0 representing one frame at 60fps
    const float targetFrameTime = 1.0f / 60.0f;  // 0.01667 seconds
    const float dt = (deltaTime / targetFrameTime) * config.timeScale;
    
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
    
    const size_t n = particles.size();
    const float radiusSq = config.interactionRadius * config.interactionRadius;
    const float invRadius = 1.0f / config.interactionRadius;
    const float forceFactor = config.forceFactor;
    
    // Only use parallel processing for larger particle counts
    const bool useParallel = (n > 200);
    
    // Process particles - only parallelize if beneficial
    if (useParallel) {
        #pragma omp parallel for schedule(dynamic, 64)
        for (size_t i = 0; i < n; ++i) {
            // Thread-local neighbor buffer (each thread gets its own)
            std::vector<int> neighbors;
            neighbors.reserve(100);
            
            const float px = particles[i].x;
            const float py = particles[i].y;
            const int ptype = particles[i].type;
            
            if (config.useSpatialHash) {
                spatialHash.queryInto(px, py, config.interactionRadius, neighbors);
                #pragma omp atomic
                metrics.spatialQueries++;
            } else {
                // Brute force: check all particles
                neighbors.reserve(n);
                for (size_t j = 0; j < n; ++j) {
                    neighbors.push_back(j);
                }
            }
            
            float force_x = 0.0f;
            float force_y = 0.0f;
            
            // Vectorized inner loop - compiler can auto-vectorize this
            for (size_t idx = 0; idx < neighbors.size(); ++idx) {
                const int j = neighbors[idx];
                if (i == static_cast<size_t>(j)) continue;
                
                // Calculate delta (vectorizable)
                float dx, dy;
                if (config.boundaryMode == WRAP) {
                    dx = particles[j].x - px;
                    dy = particles[j].y - py;
                    // Wrap handling
                    if (dx > 0.5f) dx -= 1.0f;
                    else if (dx < -0.5f) dx += 1.0f;
                    if (dy > 0.5f) dy -= 1.0f;
                    else if (dy < -0.5f) dy += 1.0f;
                } else {
                    dx = particles[j].x - px;
                    dy = particles[j].y - py;
                }
                
                const float distSq = dx * dx + dy * dy;
                
                // Branchless distance check using masking
                const bool inRange = (distSq > 0.00001f) && (distSq < radiusSq);
                if (inRange) {
                    const float invDist = 1.0f / std::sqrt(distSq);
                    const float dist = distSq * invDist;
                    const float normDist = dist * invRadius;
                    
                    const float attraction = forces[ptype][particles[j].type];
                    const float force = calculateForce(normDist, attraction) * forceFactor;
                    
                    force_x += dx * invDist * force;
                    force_y += dy * invDist * force;
                    
                    #pragma omp atomic
                    metrics.forceCalculations++;
                }
            }
            
            // Mouse interaction (done serially, not in parallel section)
            if (config.mousePressed) {
                const float dx = config.mouseX - px;
                const float dy = config.mouseY - py;
                const float distSq = dx * dx + dy * dy;
                const float mouseRadiusSq = config.mouseRadius * config.mouseRadius;
                
                if (distSq < mouseRadiusSq && distSq > 0.00001f) {
                    const float invDist = 1.0f / std::sqrt(distSq);
                    const float dist = distSq * invDist;
                    const float strength = (1.0f - dist / config.mouseRadius);
                    const float forceMagnitude = config.mouseForce * strength * invDist;
                    
                    force_x += dx * forceMagnitude;
                    force_y += dy * forceMagnitude;
                }
            }
            
            fx[i] = force_x;
            fy[i] = force_y;
        }
    } else {
        // Sequential processing for small particle counts (more stable)
        std::vector<int> neighbors;
        neighbors.reserve(100);
        
        for (size_t i = 0; i < n; ++i) {
            neighbors.clear();
            
            const float px = particles[i].x;
            const float py = particles[i].y;
            const int ptype = particles[i].type;
        
            if (config.useSpatialHash) {
                spatialHash.queryInto(px, py, config.interactionRadius, neighbors);
                metrics.spatialQueries++;
            } else {
                // Brute force: check all particles
                neighbors.reserve(n);
                for (size_t j = 0; j < n; ++j) {
                    neighbors.push_back(j);
                }
            }
            
            float force_x = 0.0f;
            float force_y = 0.0f;
            
            // Vectorized inner loop
            for (size_t idx = 0; idx < neighbors.size(); ++idx) {
                const int j = neighbors[idx];
                if (i == static_cast<size_t>(j)) continue;
                
                float dx, dy;
                if (config.boundaryMode == WRAP) {
                    dx = particles[j].x - px;
                    dy = particles[j].y - py;
                    if (dx > 0.5f) dx -= 1.0f;
                    else if (dx < -0.5f) dx += 1.0f;
                    if (dy > 0.5f) dy -= 1.0f;
                    else if (dy < -0.5f) dy += 1.0f;
                } else {
                    dx = particles[j].x - px;
                    dy = particles[j].y - py;
                }
                
                const float distSq = dx * dx + dy * dy;
                
                const bool inRange = (distSq > 0.00001f) && (distSq < radiusSq);
                if (inRange) {
                    const float invDist = 1.0f / std::sqrt(distSq);
                    const float dist = distSq * invDist;
                    const float normDist = dist * invRadius;
                    
                    const float attraction = forces[ptype][particles[j].type];
                    const float force = calculateForce(normDist, attraction) * forceFactor;
                    
                    force_x += dx * invDist * force;
                    force_y += dy * invDist * force;
                    metrics.forceCalculations++;
                }
            }
            
            // Mouse interaction
            if (config.mousePressed) {
                const float dx = config.mouseX - px;
                const float dy = config.mouseY - py;
                const float distSq = dx * dx + dy * dy;
                const float mouseRadiusSq = config.mouseRadius * config.mouseRadius;
                
                if (distSq < mouseRadiusSq && distSq > 0.00001f) {
                    const float invDist = 1.0f / std::sqrt(distSq);
                    const float dist = distSq * invDist;
                    const float strength = (1.0f - dist / config.mouseRadius);
                    const float forceMagnitude = config.mouseForce * strength * invDist;
                    
                    force_x += dx * forceMagnitude;
                    force_y += dy * forceMagnitude;
                }
            }
            
            fx[i] = force_x;
            fy[i] = force_y;
        }
    }
    
    // Update particles - vectorized velocity integration
    std::vector<size_t> toRemove;
    
    const float frictionFactor = config.friction;
    const float maxSpeedSq = config.maxSpeed * config.maxSpeed;
    
    // This loop can be auto-vectorized by the compiler with -O3 -march=native
    #pragma omp simd
    for (size_t i = 0; i < n; ++i) {
        // Velocity update with force application
        particles[i].vx += fx[i] * dt;
        particles[i].vy += fy[i] * dt;
        
        // Apply friction
        particles[i].vx *= frictionFactor;
        particles[i].vy *= frictionFactor;
        
        // Speed limiting (branchless where possible)
        const float speedSq = particles[i].vx * particles[i].vx + particles[i].vy * particles[i].vy;
        if (speedSq > maxSpeedSq) {
            const float invSpeed = 1.0f / std::sqrt(speedSq);
            const float speedLimit = config.maxSpeed * invSpeed;
            particles[i].vx *= speedLimit;
            particles[i].vy *= speedLimit;
        }
        
    // Advance one step.
    particles[i].x += particles[i].vx * dt;
    particles[i].y += particles[i].vy * dt;
        
        // Boundary handling - use fixed ±0.99 to keep particles cleanly inside viewport
        const float boundary = 0.99f;  // Slightly inset for clean edges
        const float damping = 0.8f;
        
        if (config.boundaryMode == WRAP) {
            // Instant wrapping
            if (particles[i].x < -boundary) particles[i].x = boundary - 0.001f;
            else if (particles[i].x > boundary) particles[i].x = -boundary + 0.001f;
            if (particles[i].y < -boundary) particles[i].y = boundary - 0.001f;
            else if (particles[i].y > boundary) particles[i].y = -boundary + 0.001f;
        } else if (config.boundaryMode == BOUNCE) {
            // Hard bounce at boundary
            if (particles[i].x < -boundary) {
                particles[i].x = -boundary;
                particles[i].vx = std::abs(particles[i].vx) * damping;
            } else if (particles[i].x > boundary) {
                particles[i].x = boundary;
                particles[i].vx = -std::abs(particles[i].vx) * damping;
            }
            
            if (particles[i].y < -boundary) {
                particles[i].y = -boundary;
                particles[i].vy = std::abs(particles[i].vy) * damping;
            } else if (particles[i].y > boundary) {
                particles[i].y = boundary;
                particles[i].vy = -std::abs(particles[i].vy) * damping;
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

void ParticleSystem::setParticleCount(int totalCount) {
    config.particlesPerType = totalCount / config.numTypes;
    createParticles();
}

void ParticleSystem::setNumTypes(int numTypes) {
    if (numTypes < 1) return;
    config.numTypes = numTypes;
    resizeForceMatrix();
    createParticles();
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

void ParticleSystem::addParticles(int count, int type) {
    std::uniform_real_distribution<float> posDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> velDist(-0.001f, 0.001f);
    
    for(int i=0; i<count; ++i) {
        Particle p;
        p.x = posDist(rng);
        p.y = posDist(rng);
        p.vx = velDist(rng);
        p.vy = velDist(rng);
        if (type < 0) p.type = std::uniform_int_distribution<int>(0, config.numTypes-1)(rng);
        else p.type = type % config.numTypes;
        particles.push_back(p);
    }
}

void ParticleSystem::removeParticles(int count) {
    if (particles.empty()) return;
    if (count >= particles.size()) {
        particles.clear();
        return;
    }
    particles.resize(particles.size() - count);
}


