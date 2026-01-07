#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <unordered_map>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

// Window
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 900;

// Particle
struct Particle {
    float x, y;
    float vx, vy;
    int type;
};

// Spatial hash
class SpatialHash {
private:
    float cellSize;
    std::unordered_map<int, std::vector<int>> grid;
    
    int hash(int x, int y) const {
        return x * 73856093 ^ y * 19349663;
    }
    
public:
    SpatialHash(float size) : cellSize(size) {}
    
    void clear() { grid.clear(); }
    
    void insert(int idx, float x, float y) {
        int cx = static_cast<int>(std::floor(x / cellSize));
        int cy = static_cast<int>(std::floor(y / cellSize));
        grid[hash(cx, cy)].push_back(idx);
    }
    
    std::vector<int> query(float x, float y, float radius) const {
        std::vector<int> result;
        int minX = static_cast<int>(std::floor((x - radius) / cellSize));
        int maxX = static_cast<int>(std::floor((x + radius) / cellSize));
        int minY = static_cast<int>(std::floor((y - radius) / cellSize));
        int maxY = static_cast<int>(std::floor((y + radius) / cellSize));
        
        for (int cy = minY; cy <= maxY; ++cy) {
            for (int cx = minX; cx <= maxX; ++cx) {
                auto it = grid.find(hash(cx, cy));
                if (it != grid.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }
        return result;
    }
};

// Simulation state
struct Simulation {
    std::vector<Particle> particles;
    std::vector<std::vector<float>> forces;
    SpatialHash spatialHash{0.3f};
    std::mt19937 rng{std::random_device{}()};
    
    // Core parameters - these define the simulation structure
    int numTypes = 4;
    int particlesPerType = 200;
    
    // Physics parameters - BALANCED for interesting behavior
    float interactionRadius = 0.25f;
    float forceFactor = 0.3f;        // Reduced from 1.0
    float friction = 0.98f;
    float maxSpeed = 0.01f;          // Reduced from 0.05
    bool useSpatialHash = true;
    
    // Boundary modes
    enum BoundaryMode { BOUNCE, WRAP, KILL } boundaryMode = BOUNCE;
    
    // Visual effects
    bool enableTrails = false;
    float trailIntensity = 0.95f;
    bool enableGlow = true;
    float particleSize = 8.0f;
    
    // Advanced features
    bool enableGravity = false;
    float gravityStrength = 0.0f;
    glm::vec2 gravityCenter{0.0f, 0.0f};
    
    bool enableVortex = false;
    float vortexStrength = 0.0f;
    glm::vec2 vortexCenter{0.0f, 0.0f};
    
    bool colorBySpeed = false;
    bool showVelocityVectors = false;
    
    // State
    bool paused = false;
    float timeScale = 1.0f;
    
    // Mouse
    float mouseX = -10.0f;
    float mouseY = -10.0f;
    bool mousePressed = false;
    float mouseRadius = 0.3f;
    float mouseForce = 0.005f;       // Reduced from 0.02
    
    // Screenshot notification
    bool showScreenshotNotification = false;
    float screenshotNotificationTime = 0.0f;
    std::string screenshotMessage = "";
    
    // Startup timer
    float startupTimer = 0.0f;
    bool hasShownWelcome = false;
} g_sim;

// Colors
const std::vector<glm::vec3> g_colors = {
    {1.0f, 0.2f, 0.2f},  // Red
    {0.2f, 1.0f, 0.3f},  // Green
    {0.3f, 0.5f, 1.0f},  // Blue
    {1.0f, 0.9f, 0.2f},  // Yellow
    {1.0f, 0.3f, 0.8f},  // Pink
    {0.3f, 1.0f, 1.0f},  // Cyan
    {1.0f, 0.6f, 0.2f},  // Orange
    {0.7f, 0.3f, 1.0f},  // Purple
};

// OpenGL
GLuint g_shaderProgram = 0;
GLuint g_VAO = 0;
GLuint g_VBO = 0;

// Wrap coordinate for torus topology
float wrapCoord(float x) {
    const float boundary = 1.0f;
    if (x < -boundary) return x + 2.0f * boundary;
    if (x > boundary) return x - 2.0f * boundary;
    return x;
}

// Calculate wrapped distance for torus
glm::vec2 getWrappedDelta(const glm::vec2& from, const glm::vec2& to) {
    glm::vec2 delta = to - from;
    
    // Wrap x
    if (delta.x > 1.0f) delta.x -= 2.0f;
    else if (delta.x < -1.0f) delta.x += 2.0f;
    
    // Wrap y
    if (delta.y > 1.0f) delta.y -= 2.0f;
    else if (delta.y < -1.0f) delta.y += 2.0f;
    
    return delta;
}

// Force function
float calculateForce(float dist, float attraction) {
    const float beta = 0.3f;
    if (dist < beta) {
        return dist / beta - 1.0f;
    } else if (dist < 1.0f) {
        return attraction * (1.0f - std::abs(2.0f * dist - 1.0f - beta) / (1.0f - beta));
    }
    return 0.0f;
}

// Initialize force matrix with BALANCED random values
void randomizeForces() {
    std::uniform_real_distribution<float> dist(-0.5f, 0.5f);  // Reduced from -1.0 to 1.0
    g_sim.forces.clear();
    g_sim.forces.resize(g_sim.numTypes, std::vector<float>(g_sim.numTypes, 0.0f));
    
    for (int i = 0; i < g_sim.numTypes; ++i) {
        for (int j = 0; j < g_sim.numTypes; ++j) {
            g_sim.forces[i][j] = dist(g_sim.rng);
        }
    }
}

// Create particles with BALANCED initial conditions
void createParticles() {
    g_sim.particles.clear();
    
    std::cout << "🔧 Creating particles: " << g_sim.particlesPerType << " per type, " << g_sim.numTypes << " types" << std::endl;
    
    // Tighter clustering for more interaction
    std::uniform_real_distribution<float> posDist(-0.5f, 0.5f);  // Reduced from -0.8 to 0.8
    std::uniform_real_distribution<float> velDist(-0.0005f, 0.0005f);  // Reduced by 10x
    
    for (int type = 0; type < g_sim.numTypes; ++type) {
        for (int i = 0; i < g_sim.particlesPerType; ++i) {
            Particle p;
            p.x = posDist(g_sim.rng);
            p.y = posDist(g_sim.rng);
            p.vx = velDist(g_sim.rng);
            p.vy = velDist(g_sim.rng);
            p.type = type;
            g_sim.particles.push_back(p);
        }
    }
    
    std::cout << "Created " << g_sim.particles.size() << " particles with " 
              << g_sim.numTypes << " types" << std::endl;
}

// Full reset - call when structure changes
void resetSimulation(bool randomForces = false) {
    if (randomForces) {
        randomizeForces();
    } else {
        // Resize force matrix to match new type count
        g_sim.forces.resize(g_sim.numTypes);
        for (auto& row : g_sim.forces) {
            row.resize(g_sim.numTypes, 0.0f);
        }
    }
    createParticles();
}

// REBALANCED Presets with gentler forces
void loadPreset(const std::string& name) {
    if (name == "Orbits") {
        g_sim.numTypes = 4;
        g_sim.forces = {
            { 0.0f, -0.3f,  0.4f, -0.2f},
            { 0.5f,  0.0f, -0.2f,  0.3f},
            {-0.1f,  0.4f,  0.0f,  0.2f},
            { 0.3f, -0.2f,  0.5f,  0.0f}
        };
    } else if (name == "Chaos") {
        g_sim.numTypes = 5;
        g_sim.forces = {
            { 0.0f,  0.4f, -0.5f,  0.2f, -0.3f},
            {-0.4f,  0.0f,  0.3f, -0.4f,  0.2f},
            { 0.5f, -0.3f,  0.0f,  0.4f, -0.3f},
            {-0.2f,  0.5f, -0.3f,  0.0f,  0.3f},
            { 0.3f, -0.2f,  0.4f, -0.4f,  0.0f}
        };
    } else if (name == "Balance") {
        g_sim.numTypes = 3;
        g_sim.forces = {
            { 0.0f, -0.3f,  0.3f},
            { 0.3f,  0.0f, -0.3f},
            {-0.3f,  0.3f,  0.0f}
        };
    } else if (name == "Swirls") {
        g_sim.numTypes = 4;
        g_sim.forces = {
            { 0.0f,  0.5f, -0.4f,  0.2f},
            {-0.5f,  0.0f,  0.4f, -0.3f},
            { 0.4f, -0.4f,  0.0f,  0.3f},
            {-0.2f,  0.3f, -0.3f,  0.0f}
        };
    } else if (name == "Snakes") {
        g_sim.numTypes = 6;
        g_sim.forces.clear();
        g_sim.forces.resize(6, std::vector<float>(6, 0.0f));
        for (int i = 0; i < 6; ++i) {
            g_sim.forces[i][(i + 1) % 6] = 0.5f;
            g_sim.forces[i][(i + 2) % 6] = -0.3f;
            g_sim.forces[i][(i + 5) % 6] = -0.2f;
        }
    }
    createParticles();
    std::cout << "Loaded preset: " << name << std::endl;
}

// Update
void updateSimulation() {
    if (g_sim.paused) return;
    
    const float dt = 0.016f * g_sim.timeScale;
    
    // Build spatial hash
    if (g_sim.useSpatialHash) {
        g_sim.spatialHash.clear();
        for (size_t i = 0; i < g_sim.particles.size(); ++i) {
            g_sim.spatialHash.insert(i, g_sim.particles[i].x, g_sim.particles[i].y);
        }
    }
    
    // Calculate forces
    std::vector<float> fx(g_sim.particles.size(), 0.0f);
    std::vector<float> fy(g_sim.particles.size(), 0.0f);
    
    for (size_t i = 0; i < g_sim.particles.size(); ++i) {
        std::vector<int> neighbors;
        
        if (g_sim.useSpatialHash) {
            neighbors = g_sim.spatialHash.query(g_sim.particles[i].x, g_sim.particles[i].y, 
                                                g_sim.interactionRadius);
        } else {
            neighbors.resize(g_sim.particles.size());
            for (size_t j = 0; j < g_sim.particles.size(); ++j) {
                neighbors[j] = j;
            }
        }
        
        // Particle interactions
        for (int j : neighbors) {
            if (i == static_cast<size_t>(j)) continue;
            
            glm::vec2 delta;
            if (g_sim.boundaryMode == Simulation::WRAP) {
                delta = getWrappedDelta(
                    glm::vec2(g_sim.particles[i].x, g_sim.particles[i].y),
                    glm::vec2(g_sim.particles[j].x, g_sim.particles[j].y)
                );
            } else {
                delta = glm::vec2(
                    g_sim.particles[j].x - g_sim.particles[i].x,
                    g_sim.particles[j].y - g_sim.particles[i].y
                );
            }
            
            float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
            
            if (dist > 0.001f && dist < g_sim.interactionRadius) {
                float normDist = dist / g_sim.interactionRadius;
                float attraction = g_sim.forces[g_sim.particles[i].type][g_sim.particles[j].type];
                float force = calculateForce(normDist, attraction) * g_sim.forceFactor;
                
                fx[i] += (delta.x / dist) * force;
                fy[i] += (delta.y / dist) * force;
            }
        }
        
        // Gravity effect
        if (g_sim.enableGravity) {
            glm::vec2 toCenter(g_sim.gravityCenter.x - g_sim.particles[i].x,
                              g_sim.gravityCenter.y - g_sim.particles[i].y);
            float dist = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
            if (dist > 0.001f) {
                float gravForce = g_sim.gravityStrength / (dist * dist + 0.1f);
                fx[i] += (toCenter.x / dist) * gravForce;
                fy[i] += (toCenter.y / dist) * gravForce;
            }
        }
        
        // Vortex effect
        if (g_sim.enableVortex) {
            glm::vec2 toCenter(g_sim.vortexCenter.x - g_sim.particles[i].x,
                              g_sim.vortexCenter.y - g_sim.particles[i].y);
            float dist = std::sqrt(toCenter.x * toCenter.x + toCenter.y * toCenter.y);
            if (dist > 0.001f) {
                // Perpendicular force for rotation
                float vortexForce = g_sim.vortexStrength / (dist + 0.1f);
                fx[i] += (-toCenter.y / dist) * vortexForce;
                fy[i] += (toCenter.x / dist) * vortexForce;
            }
        }
        
        // Mouse interaction
        if (g_sim.mousePressed) {
            float dx = g_sim.mouseX - g_sim.particles[i].x;
            float dy = g_sim.mouseY - g_sim.particles[i].y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist < g_sim.mouseRadius && dist > 0.001f) {
                float strength = (1.0f - dist / g_sim.mouseRadius);
                fx[i] -= (dx / dist) * g_sim.mouseForce * strength;
                fy[i] -= (dy / dist) * g_sim.mouseForce * strength;
            }
        }
    }
    
    // Update particles
    std::vector<size_t> toRemove;
    
    for (size_t i = 0; i < g_sim.particles.size(); ++i) {
        g_sim.particles[i].vx += fx[i] * dt;
        g_sim.particles[i].vy += fy[i] * dt;
        
        g_sim.particles[i].vx *= g_sim.friction;
        g_sim.particles[i].vy *= g_sim.friction;
        
        float speed = std::sqrt(g_sim.particles[i].vx * g_sim.particles[i].vx + 
                               g_sim.particles[i].vy * g_sim.particles[i].vy);
        if (speed > g_sim.maxSpeed) {
            g_sim.particles[i].vx = (g_sim.particles[i].vx / speed) * g_sim.maxSpeed;
            g_sim.particles[i].vy = (g_sim.particles[i].vy / speed) * g_sim.maxSpeed;
        }
        
        g_sim.particles[i].x += g_sim.particles[i].vx;
        g_sim.particles[i].y += g_sim.particles[i].vy;
        
        // Boundary handling
        const float boundary = 0.98f;
        const float damping = 0.8f;
        
        if (g_sim.boundaryMode == Simulation::WRAP) {
            // Torus topology
            g_sim.particles[i].x = wrapCoord(g_sim.particles[i].x);
            g_sim.particles[i].y = wrapCoord(g_sim.particles[i].y);
        } else if (g_sim.boundaryMode == Simulation::BOUNCE) {
            // Bounce off walls
            if (g_sim.particles[i].x < -boundary) {
                g_sim.particles[i].x = -boundary;
                g_sim.particles[i].vx *= -damping;
            } else if (g_sim.particles[i].x > boundary) {
                g_sim.particles[i].x = boundary;
                g_sim.particles[i].vx *= -damping;
            }
            
            if (g_sim.particles[i].y < -boundary) {
                g_sim.particles[i].y = -boundary;
                g_sim.particles[i].vy *= -damping;
            } else if (g_sim.particles[i].y > boundary) {
                g_sim.particles[i].y = boundary;
                g_sim.particles[i].vy *= -damping;
            }
        } else if (g_sim.boundaryMode == Simulation::KILL) {
            // Remove particles that go out of bounds
            if (g_sim.particles[i].x < -boundary || g_sim.particles[i].x > boundary ||
                g_sim.particles[i].y < -boundary || g_sim.particles[i].y > boundary) {
                toRemove.push_back(i);
            }
        }
    }
    
    // Remove out-of-bounds particles (in reverse order)
    for (auto it = toRemove.rbegin(); it != toRemove.rend(); ++it) {
        g_sim.particles.erase(g_sim.particles.begin() + *it);
    }
    
    // Update startup timer and show welcome message
    g_sim.startupTimer += 0.016f;
    if (!g_sim.hasShownWelcome && g_sim.startupTimer > 2.0f) {
        g_sim.hasShownWelcome = true;
        g_sim.showScreenshotNotification = true;
        g_sim.screenshotNotificationTime = 4.0f;
        g_sim.screenshotMessage = "🚀 Particle Life Ready! Press P for screenshot";
        std::cout << "📢 Welcome notification activated!" << std::endl;
    }
    
    // Update screenshot notification timer
    if (g_sim.showScreenshotNotification) {
        g_sim.screenshotNotificationTime -= 0.016f; // Decrease by frame time
        if (g_sim.screenshotNotificationTime <= 0.0f) {
            g_sim.showScreenshotNotification = false;
            std::cout << "📢 Notification hidden" << std::endl;
        }
    }
}

// OpenGL initialization
bool initOpenGL() {
    const char* vertexShader = R"(
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
    
    const char* fragmentShader = R"(
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
    
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, nullptr);
    glCompileShader(vs);
    
    GLint success;
    glGetShaderiv(vs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vs, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed:\n" << infoLog << std::endl;
        return false;
    }
    
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, nullptr);
    glCompileShader(fs);
    
    glGetShaderiv(fs, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fs, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed:\n" << infoLog << std::endl;
        return false;
    }
    
    g_shaderProgram = glCreateProgram();
    glAttachShader(g_shaderProgram, vs);
    glAttachShader(g_shaderProgram, fs);
    glLinkProgram(g_shaderProgram);
    
    glGetProgramiv(g_shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(g_shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        return false;
    }
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    glGenVertexArrays(1, &g_VAO);
    glGenBuffers(1, &g_VBO);
    
    glBindVertexArray(g_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return true;
}

// Render
void renderParticles() {
    std::vector<float> vertexData;
    vertexData.reserve(g_sim.particles.size() * 5);
    
    for (const auto& p : g_sim.particles) {
        vertexData.push_back(p.x);
        vertexData.push_back(p.y);
        
        glm::vec3 color;
        
        if (g_sim.colorBySpeed) {
            // Color by velocity magnitude
            float speed = std::sqrt(p.vx * p.vx + p.vy * p.vy);
            float t = glm::clamp(speed / g_sim.maxSpeed, 0.0f, 1.0f);
            color = glm::mix(glm::vec3(0.2f, 0.2f, 0.8f), glm::vec3(1.0f, 0.2f, 0.2f), t);
        } else {
            color = g_colors[p.type % g_colors.size()];
        }
        
        vertexData.push_back(color.r);
        vertexData.push_back(color.g);
        vertexData.push_back(color.b);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), 
                 vertexData.data(), GL_DYNAMIC_DRAW);
    
    glUseProgram(g_shaderProgram);
    glUniform1f(glGetUniformLocation(g_shaderProgram, "uPointSize"), g_sim.particleSize);
    glUniform1i(glGetUniformLocation(g_shaderProgram, "uEnableGlow"), g_sim.enableGlow ? 1 : 0);
    glBindVertexArray(g_VAO);
    glDrawArrays(GL_POINTS, 0, g_sim.particles.size());
}

// GUI with improved layout and tooltips
void renderGUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(450, 820), ImGuiCond_FirstUseEver);
    
    ImGui::Begin("Particle Life");
    
    // ===== STATUS =====
    ImGui::Text("Particles: %zu", g_sim.particles.size());
    ImGui::Text("Types: %d", g_sim.numTypes);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    
    // ===== KEYBOARD SHORTCUTS =====
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.9f, 1.0f, 1.0f)); // Light blue
    ImGui::Text("⌨️ Shortcuts:");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Space=Pause | R=Reset | P=Screenshot");
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            "Keyboard Shortcuts:\n"
            "• SPACE - Pause/Resume simulation\n"
            "• R - Reset and randomize forces\n"
            "• P - Take screenshot (saves to ParticleLifeScreenshots folder)"
        );
    }
    ImGui::Separator();
    
    // ===== MAIN CONTROLS =====
    if (ImGui::Button(g_sim.paused ? "▶ Resume" : "⏸ Pause", ImVec2(100, 0))) {
        g_sim.paused = !g_sim.paused;
    }
    ImGui::SameLine();
    if (ImGui::Button("🔄 Reset Particles", ImVec2(140, 0))) {
        createParticles();
    }
    ImGui::SameLine();
    if (ImGui::Button("🎲 Randomize", ImVec2(100, 0))) {
        resetSimulation(true);
    }
    
    ImGui::Spacing();
    ImGui::SliderFloat("Time Scale", &g_sim.timeScale, 0.1f, 3.0f, "%.1fx");
    ImGui::Separator();
    
    // ===== PRESETS =====
    if (ImGui::CollapsingHeader("Presets", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("Orbits", ImVec2(85, 0))) loadPreset("Orbits");
        ImGui::SameLine();
        if (ImGui::Button("Chaos", ImVec2(85, 0))) loadPreset("Chaos");
        ImGui::SameLine();
        if (ImGui::Button("Balance", ImVec2(85, 0))) loadPreset("Balance");
        
        if (ImGui::Button("Swirls", ImVec2(85, 0))) loadPreset("Swirls");
        ImGui::SameLine();
        if (ImGui::Button("Snakes", ImVec2(85, 0))) loadPreset("Snakes");
    }
    
    // ===== STRUCTURE (requires reset) =====
    if (ImGui::CollapsingHeader("Structure (Needs Reset)", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.3f, 1.0f));
        ImGui::TextWrapped("⚠ Changes here require clicking 'Apply & Reset'");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        static int newNumTypes = g_sim.numTypes;
        static int newParticlesPerType = g_sim.particlesPerType;
        
        ImGui::SliderInt("Particle Types", &newNumTypes, 2, 8);
        ImGui::SliderInt("Particles Per Type", &newParticlesPerType, 0, 500);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Set to 0 to create an empty simulation");
        }
        
        bool changed = (newNumTypes != g_sim.numTypes || newParticlesPerType != g_sim.particlesPerType);
        
        if (changed) {
            ImGui::Spacing();
            if (ImGui::Button("✓ Apply & Reset", ImVec2(-1, 30))) {
                std::cout << "🔄 Applying new settings: " << newNumTypes << " types, " << newParticlesPerType << " particles per type" << std::endl;
                g_sim.numTypes = newNumTypes;
                g_sim.particlesPerType = newParticlesPerType;
                resetSimulation(false);
            }
        } else {
            // Keep in sync when preset is loaded
            newNumTypes = g_sim.numTypes;
            newParticlesPerType = g_sim.particlesPerType;
        }
    }
    
    // ===== PHYSICS (live adjustable) =====
    if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::SliderFloat("Interaction Radius", &g_sim.interactionRadius, 0.1f, 0.5f, "%.2f");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("How far particles can interact");
        }
        
        ImGui::SliderFloat("Force Factor", &g_sim.forceFactor, 0.05f, 1.0f, "%.2f");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Overall strength multiplier - try 0.1-0.3 for subtle effects");
        }
        
        ImGui::SliderFloat("Friction", &g_sim.friction, 0.9f, 0.999f, "%.3f");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Higher = less damping, smoother movement");
        }
        
        ImGui::SliderFloat("Max Speed", &g_sim.maxSpeed, 0.001f, 0.05f, "%.3f");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Speed limit - lower values create calmer behavior");
        }
        
        ImGui::Checkbox("Use Spatial Hash", &g_sim.useSpatialHash);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Performance optimization for large particle counts");
        }
        
        ImGui::Spacing();
        ImGui::Text("Boundary Mode:");
        ImGui::RadioButton("Bounce", (int*)&g_sim.boundaryMode, Simulation::BOUNCE);
        ImGui::SameLine();
        ImGui::RadioButton("Wrap (Torus)", (int*)&g_sim.boundaryMode, Simulation::WRAP);
        ImGui::SameLine();
        ImGui::RadioButton("Kill", (int*)&g_sim.boundaryMode, Simulation::KILL);
    }
    
    // ===== SPECIAL EFFECTS =====
    if (ImGui::CollapsingHeader("Special Effects")) {
        ImGui::Checkbox("Enable Gravity", &g_sim.enableGravity);
        if (g_sim.enableGravity) {
            ImGui::SliderFloat("Gravity Strength", &g_sim.gravityStrength, 0.0f, 0.1f, "%.4f");
            ImGui::SliderFloat2("Gravity Center", &g_sim.gravityCenter.x, -1.0f, 1.0f, "%.2f");
        }
        
        ImGui::Spacing();
        ImGui::Checkbox("Enable Vortex", &g_sim.enableVortex);
        if (g_sim.enableVortex) {
            ImGui::SliderFloat("Vortex Strength", &g_sim.vortexStrength, -0.1f, 0.1f, "%.4f");
            ImGui::SliderFloat2("Vortex Center", &g_sim.vortexCenter.x, -1.0f, 1.0f, "%.2f");
        }
    }
    
    // ===== VISUAL =====
    if (ImGui::CollapsingHeader("Visual")) {
        ImGui::SliderFloat("Particle Size", &g_sim.particleSize, 2.0f, 20.0f, "%.1f");
        ImGui::Checkbox("Enable Glow", &g_sim.enableGlow);
        ImGui::Checkbox("Color by Speed", &g_sim.colorBySpeed);
        
        ImGui::Spacing();
        ImGui::Checkbox("Enable Trails", &g_sim.enableTrails);
        if (g_sim.enableTrails) {
            ImGui::SliderFloat("Trail Fade", &g_sim.trailIntensity, 0.8f, 0.99f, "%.3f");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Higher = longer lasting trails");
            }
        }
    }
    
    // ===== MOUSE =====
    if (ImGui::CollapsingHeader("Mouse Interaction")) {
        ImGui::SliderFloat("Radius", &g_sim.mouseRadius, 0.1f, 0.6f, "%.2f");
        ImGui::SliderFloat("Force", &g_sim.mouseForce, 0.0f, 0.03f, "%.4f");
        ImGui::TextWrapped("Hold left mouse button to repel particles");
    }
    
    // ===== FORCE MATRIX =====
    if (ImGui::CollapsingHeader("Force Matrix", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "■ Green = Attract");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "■ Red = Repel");
        ImGui::Spacing();
        ImGui::TextWrapped("💡 Tip: Start with small values (±0.1 to ±0.3)");
        ImGui::Spacing();
        
        for (int i = 0; i < g_sim.numTypes; ++i) {
            ImGui::PushID(i);
            ImGui::Text("T%d", i);
            
            for (int j = 0; j < g_sim.numTypes; ++j) {
                ImGui::SameLine();
                ImGui::PushID(j);
                ImGui::PushItemWidth(60);
                
                float val = g_sim.forces[i][j];
                if (val > 0.0f) {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0f, 0.3f, 0.0f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
                } else if (val < 0.0f) {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.3f, 0.0f, 0.0f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                } else {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                }
                
                ImGui::VSliderFloat("##", ImVec2(55, 100), &g_sim.forces[i][j], -1.0f, 1.0f, "%.2f");
                
                ImGui::PopStyleColor(2);
                ImGui::PopItemWidth();
                ImGui::PopID();
            }
            ImGui::PopID();
        }
    }
    
    // ===== COLORS =====
    if (ImGui::CollapsingHeader("Color Legend")) {
        if (g_sim.colorBySpeed) {
            ImGui::TextWrapped("Colors represent speed (blue=slow, red=fast)");
        } else {
            for (int i = 0; i < g_sim.numTypes; ++i) {
                const auto& c = g_colors[i % g_colors.size()];
                ImGui::ColorButton("##", ImVec4(c.r, c.g, c.b, 1.0f), 0, ImVec2(30, 30));
                ImGui::SameLine();
                ImGui::Text("Type %d (%d particles)", i, g_sim.particlesPerType);
            }
        }
    }
    
    ImGui::End();
    
    // Screenshot notification overlay
    if (g_sim.showScreenshotNotification && !g_sim.screenshotMessage.empty()) {
        std::cout << "🎨 Rendering notification: " << g_sim.screenshotMessage << " (time: " << g_sim.screenshotNotificationTime << ")" << std::endl;
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;
        if (displaySize.x > 0 && displaySize.y > 0) {
            ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f - 150, 50), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Always);
        
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | 
                                ImGuiWindowFlags_NoResize | 
                                ImGuiWindowFlags_NoMove | 
                                ImGuiWindowFlags_NoCollapse |
                                ImGuiWindowFlags_NoScrollbar |
                                ImGuiWindowFlags_AlwaysAutoResize;
        
        // Semi-transparent background
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.3f, 0.8f, 1.0f, 1.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 15));
        
        if (ImGui::Begin("Screenshot Notification", nullptr, flags)) {
            // Animated colors based on time remaining
            float alpha = ImGui::GetStyle().Alpha;
            float timeRatio = g_sim.screenshotNotificationTime / 3.0f;
            
            // Fade out effect
            if (timeRatio < 0.3f) {
                alpha *= (timeRatio / 0.3f);
            }
            
            ImVec4 textColor;
            if (g_sim.screenshotMessage.find("failed") != std::string::npos || 
                g_sim.screenshotMessage.find("❌") != std::string::npos) {
                textColor = ImVec4(1.0f, 0.4f, 0.4f, alpha); // Red for errors
            } else {
                textColor = ImVec4(0.4f, 1.0f, 0.4f, alpha); // Green for success
            }
            
            ImGui::PushStyleColor(ImGuiCol_Text, textColor);
            
            // Center the text
            ImVec2 text_size = ImGui::CalcTextSize(g_sim.screenshotMessage.c_str());
            ImVec2 window_size = ImGui::GetWindowSize();
            ImGui::SetCursorPosX((window_size.x - text_size.x) * 0.5f);
            
            ImGui::Text("%s", g_sim.screenshotMessage.c_str());
            
            ImGui::PopStyleColor(1); // Text color
        }
        ImGui::End();
        
        ImGui::PopStyleVar(3); // Window padding, rounding, border
        ImGui::PopStyleColor(2); // Window bg, border
        }  // Close displaySize check
    }  // Close notification check
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// Callbacks
void mouseCallback(GLFWwindow* window, double x, double y) {
    g_sim.mouseX = (2.0f * x) / SCREEN_WIDTH - 1.0f;
    g_sim.mouseY = 1.0f - (2.0f * y) / SCREEN_HEIGHT;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_sim.mousePressed = (action == GLFW_PRESS);
    }
}

// Screenshot functionality
void takeScreenshot() {
    // Create screenshots directory if it doesn't exist
    std::filesystem::create_directories("../ParticleLifeScreenshots");
    
    // Generate timestamp for filename
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S_");
    ss << std::setfill('0') << std::setw(3) << ms.count();
    
    std::string filename = "../ParticleLifeScreenshots/particle_life_" + ss.str() + ".png";
    
    std::cout << "📸 Taking screenshot..." << std::endl;
    
    // Try multiple approaches for window capture
    // First: try to capture the focused window
    std::string command1 = "screencapture -w \"" + filename + "\" 2>/dev/null";
    int result1 = std::system(command1.c_str());
    
    if (result1 == 0) {
        std::cout << "📸 Screenshot saved: " << filename << std::endl;
        g_sim.showScreenshotNotification = true;
        g_sim.screenshotNotificationTime = 3.0f; // Show for 3 seconds
        g_sim.screenshotMessage = "📸 Screenshot saved!";
        return;
    }
    
    // Second: try to find ParticleLife process
    std::string command2 = "screencapture -l$(osascript -e 'tell app \"System Events\" to id of first process whose name contains \"ParticleLife\"') \"" + filename + "\" 2>/dev/null";
    int result2 = std::system(command2.c_str());
    
    if (result2 == 0) {
        std::cout << "📸 Screenshot saved (process capture): " << filename << std::endl;
        g_sim.showScreenshotNotification = true;
        g_sim.screenshotNotificationTime = 3.0f;
        g_sim.screenshotMessage = "📸 Screenshot captured!";
        return;
    }
    
    // Third: fallback to interactive selection
    std::cout << "⚠️  Auto-capture failed, opening selection mode..." << std::endl;
    std::string fallback_command = "screencapture -i \"" + filename + "\" 2>/dev/null";
    int result3 = std::system(fallback_command.c_str());
    
    if (result3 == 0) {
        std::cout << "📸 Screenshot saved (interactive): " << filename << std::endl;
        g_sim.showScreenshotNotification = true;
        g_sim.screenshotNotificationTime = 3.0f;
        g_sim.screenshotMessage = "📸 Interactive screenshot saved!";
    } else {
        std::cout << "❌ Screenshot failed - please use external screenshot tool" << std::endl;
        g_sim.showScreenshotNotification = true;
        g_sim.screenshotNotificationTime = 3.0f;
        g_sim.screenshotMessage = "❌ Screenshot failed - try external tool";
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_SPACE) {
            g_sim.paused = !g_sim.paused;
            std::cout << "⏸️ Pause toggled: " << (g_sim.paused ? "PAUSED" : "RESUMED") << std::endl;
        } else if (key == GLFW_KEY_R) {
            resetSimulation(true);
            std::cout << "🔄 Simulation reset" << std::endl;
        } else if (key == GLFW_KEY_P) {
            std::cout << "📸 P key pressed - taking screenshot..." << std::endl;
            takeScreenshot();
        }
    }
}

// Main
int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, 
                                          "Particle Life Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;
    
    if (!initOpenGL()) {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        return -1;
    }
    
    // ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    // Initialize
    resetSimulation(true);
    
    // Don't show startup notification immediately - wait for first frame
    // g_sim.showScreenshotNotification = true;
    // g_sim.screenshotNotificationTime = 5.0f;
    // g_sim.screenshotMessage = "🚀 Particle Life Ready! Press P for screenshot";
    
    std::cout << "\n=== Particle Life Simulation (BALANCED) ===" << std::endl;
    std::cout << "Started with " << g_sim.particles.size() << " particles" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  SPACE - Pause/Resume" << std::endl;
    std::cout << "  R - Randomize forces" << std::endl;
    std::cout << "  Left Click + Drag - Repel particles" << std::endl;
    std::cout << "\nOptimizations:" << std::endl;
    std::cout << "  ✓ Balanced force values (±0.5 max)" << std::endl;
    std::cout << "  ✓ Reduced initial velocities (10x)" << std::endl;
    std::cout << "  ✓ Lower default force factor (0.3)" << std::endl;
    std::cout << "  ✓ Reduced max speed (0.01)" << std::endl;
    std::cout << "  ✓ Tighter particle clustering" << std::endl;
    std::cout << "  ✓ Shader error checking" << std::endl;
    std::cout << "\n💡 Tip: Most interesting behavior happens with:" << std::endl;
    std::cout << "   - Force Factor: 0.1 - 0.3" << std::endl;
    std::cout << "   - Force Matrix values: ±0.1 to ±0.4" << std::endl;
    std::cout << "   - Max Speed: 0.005 - 0.01" << std::endl;
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Handle trail effect
        if (g_sim.enableTrails) {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f - g_sim.trailIntensity);
            glClear(GL_COLOR_BUFFER_BIT);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        } else {
            glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        
        updateSimulation();
        renderParticles();
        renderGUI();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glDeleteVertexArrays(1, &g_VAO);
    glDeleteBuffers(1, &g_VBO);
    glDeleteProgram(g_shaderProgram);
    
    glfwTerminate();
    
    std::cout << "\nSimulation ended" << std::endl;
    return 0;
}