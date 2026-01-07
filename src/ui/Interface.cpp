#include "ui/Interface.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

Interface::Interface(ParticleSystem& ps, Renderer& r) 
    : particleSystem(ps), renderer(r) {
    // Initialize temp config
    tempConfig.newNumTypes = particleSystem.getConfig().numTypes;
    tempConfig.newParticlesPerType = particleSystem.getConfig().particlesPerType;
}

bool Interface::initialize() {
    return true;
}

void Interface::cleanup() {
    // Cleanup handled by main application
}

void Interface::render() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // *** MODERN FLOATING PANELS SYSTEM ***
    // This should replace the old slide menu completely
    renderControlPanel();
    renderPerformanceHUD();
    renderVisualEffectsPanel();
    renderInteractionPanel();
    renderForceMatrixPanel();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Interface::renderControlPanel() {
    if (!showControlPanel) return;
    
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.95f);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("🚀 NEW MODERN CONTROLS - SUCCESS!", &showControlPanel, flags)) {
        // Make it obviously different from old menu
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "*** SUCCESS: FLOATING PANEL SYSTEM WORKING ***");
        ImGui::Separator();
        
        ImGui::Text("🎉 The old slide menu has been REPLACED!");
        ImGui::Text("🎨 This is the new modern floating interface!");
        
        ImGui::Separator();
        
        // Essential controls
        auto& config = particleSystem.getConfig();
        
        ImGui::Text("🔄 Simulation");
        if (ImGui::Button(config.isPaused ? "▶ Resume" : "⏸ Pause", ImVec2(120, 30))) {
            config.isPaused = !config.isPaused;
        }
        
        if (ImGui::Button("🎲 Reset", ImVec2(120, 30))) {
            particleSystem.reset();
        }
        
        ImGui::Separator();
        ImGui::Text("⚡ Physics");
        ImGui::SliderFloat("Speed", &config.maxSpeed, 0.001f, 0.05f, "%.3f");
        ImGui::SliderFloat("Force", &config.forceFactor, 0.1f, 2.0f, "%.1f");
        ImGui::SliderFloat("Friction", &config.friction, 0.9f, 0.999f, "%.3f");
        
        ImGui::Separator();
        ImGui::Text("🧵 Multi-threading");
        ImGui::Checkbox("Enable Threading", &config.enableMultiThreading);
        if (config.enableMultiThreading) {
            ImGui::SliderInt("Thread Count", &config.numThreads, 0, 16);
            if (config.numThreads == 0) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(auto)");
            }
            ImGui::SliderInt("Min Particles for Threading", &config.minParticlesForThreading, 50, 1000);
        }
        
        ImGui::Separator();
        ImGui::Text("🎨 Visual Effects");
        if (ImGui::Button("Show Visual Panel", ImVec2(150, 25))) {
            showVisualEffects = !showVisualEffects;
        }
        
        ImGui::Separator();
        ImGui::Text("⚛️ Particle & Forces");
        if (ImGui::Button("Show Particle Panel", ImVec2(150, 25))) {
            showInteraction = !showInteraction;
        }
        ImGui::SameLine();
        if (ImGui::Button("Force Matrix", ImVec2(100, 25))) {
            showForceMatrix = !showForceMatrix;
        }
    }
    ImGui::End();
}

void Interface::renderPerformanceHUD() {
    if (!showPerformanceHUD) return;
    
    ImGui::SetNextWindowPos(ImVec2(440, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(200, 140), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.8f);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("📊 Performance", &showPerformanceHUD, flags)) {
        const auto& metrics = particleSystem.getPerformanceMetrics();
        const auto& particles = particleSystem.getParticles();
        
        float fps = metrics.averageFPS;
        ImVec4 fpsColor = fps > 50 ? ImVec4(0,1,0,1) : fps > 30 ? ImVec4(1,1,0,1) : ImVec4(1,0,0,1);
        
        ImGui::TextColored(fpsColor, "FPS: %.1f", fps);
        ImGui::Text("Particles: %zu", particles.size());
        ImGui::Text("Update: %.2fms", metrics.updateTimeMs);
        ImGui::Text("Render: %.2fms", metrics.renderTimeMs);
    }
    ImGui::End();
}

// All old functions disabled
void Interface::renderStatus() {}
void Interface::renderMainControls() {}
void Interface::renderPresets() {}
void Interface::renderStructure() {}
void Interface::renderPhysics() {}
void Interface::renderSpecialEffects() {}
void Interface::renderVisual() {}
void Interface::renderMouseInteraction() {}
void Interface::renderPerformance() {}
void Interface::renderForceMatrix() {}
void Interface::renderColorLegend() {}
void Interface::renderVisualEffectsPanel() {
    if (!showVisualEffects) return;
    
    ImGui::SetNextWindowPos(ImVec2(440, 180), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);
    
    if (ImGui::Begin("🎨 Visual Effects Panel", &showVisualEffects)) {
        auto& renderConfig = renderer.getConfig();
        
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), "🌟 Advanced Particle Effects");
        ImGui::Separator();
        
        ImGui::Text("📏 Particle Sizing");
        ImGui::SliderFloat("Base Size", &renderConfig.particleSize, 1.0f, 20.0f, "%.1f");
        ImGui::SliderFloat("Min Size", &renderConfig.minParticleSize, 0.5f, 8.0f, "%.1f");
        ImGui::Checkbox("Size by Speed", &renderConfig.sizeBySpeed);
        if (renderConfig.sizeBySpeed) {
            ImGui::SliderFloat("Max Size", &renderConfig.maxParticleSize, 8.0f, 40.0f, "%.1f");
        }
        
        ImGui::Separator();
        ImGui::Text("✨ Dynamic Effects");
        ImGui::Checkbox("Enable Pulsation", &renderConfig.enablePulsation);
        if (renderConfig.enablePulsation) {
            ImGui::SliderFloat("Pulse Speed", &renderConfig.pulsationSpeed, 0.5f, 5.0f, "%.1f");
            ImGui::SliderFloat("Pulse Amount", &renderConfig.pulsationAmount, 0.1f, 1.0f, "%.2f");
        }
        
        ImGui::Checkbox("Enable Trails", &renderConfig.enableTrails);
        if (renderConfig.enableTrails) {
            ImGui::SliderFloat("Trail Fade", &renderConfig.trailIntensity, 0.8f, 0.99f, "%.3f");
        }
        
        ImGui::Separator();
        ImGui::Text("🎨 Color Schemes");
        ImGui::Checkbox("Velocity Colors", &renderConfig.useVelocityColors);
        if (!renderConfig.useVelocityColors) {
            ImGui::Checkbox("Type Colors", &renderConfig.useTypeColors);
        }
        ImGui::Checkbox("Enable Glow", &renderConfig.enableGlow);
        
        ImGui::Separator();
        ImGui::Text("🎭 Quick Presets");
        if (ImGui::Button("Minimal", ImVec2(90, 0))) {
            renderConfig.particleSize = 3.0f;
            renderConfig.enablePulsation = false;
            renderConfig.enableTrails = false;
            renderConfig.enableGlow = false;
            renderConfig.useVelocityColors = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Standard", ImVec2(90, 0))) {
            renderConfig.particleSize = 6.0f;
            renderConfig.enablePulsation = true;
            renderConfig.pulsationSpeed = 2.0f;
            renderConfig.useVelocityColors = true;
            renderConfig.enableGlow = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Extreme", ImVec2(90, 0))) {
            renderConfig.particleSize = 12.0f;
            renderConfig.enablePulsation = true;
            renderConfig.pulsationSpeed = 4.0f;
            renderConfig.enableTrails = true;
            renderConfig.sizeBySpeed = true;
            renderConfig.useVelocityColors = true;
            renderConfig.enableGlow = true;
        }
        
        ImGui::Separator();
        ImGui::Text("⚙️ Rendering Options");
        ImGui::Checkbox("Show Grid", &renderConfig.showGrid);
        ImGui::Checkbox("Show Center", &renderConfig.showCenter);
    }
    ImGui::End();
}
void Interface::renderInteractionPanel() {
    if (!showInteraction) return;
    
    ImGui::SetNextWindowPos(ImVec2(760, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 450), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);
    
    if (ImGui::Begin("⚛️ Particle Management", &showInteraction)) {
        auto& config = particleSystem.getConfig();
        int currentParticleCount = static_cast<int>(particleSystem.getParticles().size());
        
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.8f, 1), "🎛️ Dynamic Particle Control");
        ImGui::Separator();
        
        // Particle count controls
        ImGui::Text("📊 Particle Count: %d", currentParticleCount);
        
        static int targetCount = currentParticleCount;
        ImGui::SliderInt("Target Count", &targetCount, 0, 2000);
        
        if (ImGui::Button("Apply Count", ImVec2(100, 0))) {
            particleSystem.setParticleCount(targetCount);
        }
        ImGui::SameLine();
        if (ImGui::Button("+100", ImVec2(60, 0))) {
            particleSystem.addParticles(100);
            targetCount = static_cast<int>(particleSystem.getParticles().size());
        }
        ImGui::SameLine();
        if (ImGui::Button("-100", ImVec2(60, 0))) {
            particleSystem.removeParticles(100);
            targetCount = static_cast<int>(particleSystem.getParticles().size());
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear", ImVec2(60, 0))) {
            particleSystem.setParticleCount(0);
            targetCount = 0;
        }
        
        ImGui::Separator();
        
        // Particle type management
        ImGui::Text("🎨 Particle Types: %d", config.numTypes);
        
        static int targetTypes = config.numTypes;
        ImGui::SliderInt("Number of Types", &targetTypes, 1, 8);
        
        if (ImGui::Button("Apply Types", ImVec2(100, 0))) {
            particleSystem.setParticleTypes(targetTypes);
        }
        ImGui::SameLine();
        if (ImGui::Button("Randomize Forces", ImVec2(140, 0))) {
            particleSystem.generateRandomForces();
        }
        
        ImGui::Separator();
        
        // Quick particle spawning
        ImGui::Text("⚡ Quick Spawn");
        static int spawnType = 0;
        ImGui::SliderInt("Spawn Type", &spawnType, 0, config.numTypes - 1);
        
        if (ImGui::Button("Spawn 10", ImVec2(80, 0))) {
            particleSystem.addParticles(10, spawnType);
            targetCount = static_cast<int>(particleSystem.getParticles().size());
        }
        ImGui::SameLine();
        if (ImGui::Button("Spawn 50", ImVec2(80, 0))) {
            particleSystem.addParticles(50, spawnType);
            targetCount = static_cast<int>(particleSystem.getParticles().size());
        }
        ImGui::SameLine();
        if (ImGui::Button("Spawn Random", ImVec2(100, 0))) {
            particleSystem.addParticles(25, -1); // -1 for random type
            targetCount = static_cast<int>(particleSystem.getParticles().size());
        }
        
        ImGui::Separator();
        
        // Particle distribution by type
        ImGui::Text("📈 Particle Distribution");
        std::vector<int> typeCounts(config.numTypes, 0);
        for (const auto& particle : particleSystem.getParticles()) {
            if (particle.type >= 0 && particle.type < config.numTypes) {
                typeCounts[particle.type]++;
            }
        }
        
        for (int i = 0; i < config.numTypes; ++i) {
            ImGui::Text("Type %d: %d particles", i, typeCounts[i]);
        }
        
        ImGui::Separator();
        
        // Performance info
        ImGui::Text("⚙️ Threading Info");
        bool threadingActive = config.enableMultiThreading && 
                              currentParticleCount >= config.minParticlesForThreading;
        ImGui::TextColored(threadingActive ? ImVec4(0,1,0,1) : ImVec4(1,1,0,1), 
                          "Threading: %s", threadingActive ? "ACTIVE" : "DISABLED");
        
        if (threadingActive) {
            int threads = config.numThreads > 0 ? config.numThreads : std::thread::hardware_concurrency();
            ImGui::Text("Using %d threads", threads);
        }
        
        // Update target count to current count
        if (targetCount != currentParticleCount) {
            targetCount = currentParticleCount;
        }
    }
    ImGui::End();
}
void Interface::renderPresetsPanel() {}
void Interface::renderForceMatrixPanel() {
    if (!showForceMatrix) return;
    
    ImGui::SetNextWindowPos(ImVec2(1100, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);
    
    if (ImGui::Begin("🌐 Force Matrix Editor", &showForceMatrix)) {
        auto& config = particleSystem.getConfig();
        
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1), "🔗 Interaction Forces");
        ImGui::Separator();
        
        ImGui::Text("Edit forces between particle types:");
        ImGui::Text("Positive = Attraction, Negative = Repulsion");
        
        ImGui::Separator();
        
        // Force matrix grid
        for (int from = 0; from < config.numTypes; ++from) {
            ImGui::Text("Type %d Forces:", from);
            ImGui::Indent();
            
            for (int to = 0; to < config.numTypes; ++to) {
                float currentForce = particleSystem.getForce(from, to);
                float newForce = currentForce;
                
                // Color-code the force type
                ImVec4 sliderColor;
                if (currentForce > 0.1f) {
                    sliderColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f); // Green for attraction
                } else if (currentForce < -0.1f) {
                    sliderColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f); // Red for repulsion
                } else {
                    sliderColor = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Gray for neutral
                }
                
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, sliderColor);
                ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, sliderColor);
                
                char label[64];
                snprintf(label, sizeof(label), "→ Type %d", to);
                
                if (ImGui::SliderFloat(label, &newForce, -2.0f, 2.0f, "%.2f")) {
                    particleSystem.setForce(from, to, newForce);
                }
                
                ImGui::PopStyleColor(2);
            }
            
            ImGui::Unindent();
            ImGui::Separator();
        }
        
        ImGui::Spacing();
        
        // Quick presets
        ImGui::Text("🎚️ Quick Presets:");
        
        if (ImGui::Button("🌟 Mutual Attraction", ImVec2(-1, 0))) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    particleSystem.setForce(i, j, 0.5f);
                }
            }
        }
        
        if (ImGui::Button("💥 Mutual Repulsion", ImVec2(-1, 0))) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    particleSystem.setForce(i, j, -0.5f);
                }
            }
        }
        
        if (ImGui::Button("⚖️ Self-Repel Only", ImVec2(-1, 0))) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    if (i == j) {
                        particleSystem.setForce(i, j, -0.8f); // Self-repulsion
                    } else {
                        particleSystem.setForce(i, j, 0.3f);  // Cross-attraction
                    }
                }
            }
        }
        
        if (ImGui::Button("🎲 Randomize All", ImVec2(-1, 0))) {
            particleSystem.generateRandomForces();
        }
        
        if (ImGui::Button("🔄 Reset to Zero", ImVec2(-1, 0))) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    particleSystem.setForce(i, j, 0.0f);
                }
            }
        }
        
        ImGui::Separator();
        
        // Visual force representation
        ImGui::Text("📊 Force Matrix Visualization");
        
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 canvasPos = ImGui::GetCursorScreenPos();
        float cellSize = 30.0f;
        
        for (int from = 0; from < config.numTypes; ++from) {
            for (int to = 0; to < config.numTypes; ++to) {
                float force = particleSystem.getForce(from, to);
                
                ImVec2 min = ImVec2(canvasPos.x + to * cellSize, canvasPos.y + from * cellSize);
                ImVec2 max = ImVec2(min.x + cellSize - 1, min.y + cellSize - 1);
                
                // Color based on force strength
                ImU32 color;
                if (force > 0.1f) {
                    // Green for attraction
                    uint8_t intensity = static_cast<uint8_t>(std::min(255.0f, force * 127.5f + 127.5f));
                    color = IM_COL32(0, intensity, 0, 255);
                } else if (force < -0.1f) {
                    // Red for repulsion
                    uint8_t intensity = static_cast<uint8_t>(std::min(255.0f, -force * 127.5f + 127.5f));
                    color = IM_COL32(intensity, 0, 0, 255);
                } else {
                    // Gray for neutral
                    color = IM_COL32(64, 64, 64, 255);
                }
                
                drawList->AddRectFilled(min, max, color);
                drawList->AddRect(min, max, IM_COL32(128, 128, 128, 255));
            }
        }
        
        ImGui::Dummy(ImVec2(config.numTypes * cellSize, config.numTypes * cellSize));
        
        ImGui::Text("Green = Attraction | Red = Repulsion | Gray = Neutral");
    }
    ImGui::End();
}
void Interface::renderColorLegendPanel() {}