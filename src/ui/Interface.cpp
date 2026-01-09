#include "ui/Interface.h"
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <algorithm>
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
    // Set modern ImGui style
    setupModernStyle();
    
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    // Render unified side panel
    renderMainControlPanel();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Interface::setupModernStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Modern dark theme with accent colors
    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;
    style.PopupBorderSize = 1.0f;
    
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(8, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 12.0f;
    
    // Modern color scheme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.15f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.18f, 0.8f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.15f, 0.98f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.3f, 1.0f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.2f, 0.25f, 0.8f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.3f, 0.9f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.4f, 0.7f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.1f, 0.1f, 0.12f, 0.8f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1f, 0.1f, 0.12f, 0.8f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.4f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.4f, 0.45f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.2f, 0.8f, 0.4f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.3f, 0.6f, 1.0f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.4f, 0.7f, 1.0f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.3f, 0.8f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.5f, 0.8f, 0.9f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.4f, 0.6f, 0.9f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.3f, 0.8f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.5f, 0.8f, 0.9f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.4f, 0.6f, 0.9f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.3f, 0.3f, 0.35f, 1.0f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.4f, 0.4f, 0.45f, 1.0f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.5f, 0.5f, 0.55f, 1.0f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.25f, 0.25f, 0.3f, 0.8f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.3f, 0.5f, 0.8f, 0.9f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4f, 0.6f, 0.9f, 1.0f);
    colors[ImGuiCol_Tab] = ImVec4(0.2f, 0.2f, 0.25f, 0.8f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.3f, 0.5f, 0.8f, 0.9f);
    colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.45f, 0.75f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.18f, 0.8f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.35f, 0.6f, 0.8f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.43f, 0.35f, 1.0f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.9f, 0.7f, 0.0f, 1.0f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.3f, 0.5f, 0.8f, 0.5f);
}

void Interface::renderMainControlPanel() {
    if (!showControlPanel) return;

    // Size/position the panel based on the *current* display size.
    // This keeps the panel docked to the right in windowed mode and on HiDPI screens.
    const float sidebarWidth = 350.0f;
    ImGuiIO& io = ImGui::GetIO();
    const float screenW = io.DisplaySize.x;
    const float screenH = io.DisplaySize.y;
    const float viewportW = std::max(1.0f, screenW - sidebarWidth);

    // Create fixed side panel docked to the right
    ImGui::SetNextWindowPos(ImVec2(viewportW, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(sidebarWidth, screenH), ImGuiCond_Always);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
                            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar;
    if (ImGui::Begin("Control Panel", &showControlPanel, flags)) {
        
        // Menu bar with quick actions
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Performance", nullptr, &showPerformanceHUD);
                ImGui::MenuItem("Force Matrix", nullptr, &showForceMatrix);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Presets")) {
                if (ImGui::MenuItem("Life-like")) {
                    // Apply life-like preset
                    for (int i = 0; i < particleSystem.getConfig().numTypes; ++i) {
                        for (int j = 0; j < particleSystem.getConfig().numTypes; ++j) {
                            float force = (i == j) ? -0.3f : 0.2f;
                            particleSystem.setForce(i, j, force);
                        }
                    }
                }
                if (ImGui::MenuItem("Chaos")) {
                    particleSystem.generateRandomForces();
                }
                if (ImGui::MenuItem("Reset")) {
                    particleSystem.reset();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        auto& config = particleSystem.getConfig();
        
        // Main simulation controls section
        if (ImGui::CollapsingHeader("Simulation Control", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushItemWidth(-120);
            
            // Pause/Resume with status
            ImVec4 statusColor = config.isPaused ? ImVec4(1.0f, 0.6f, 0.2f, 1.0f) : ImVec4(0.2f, 1.0f, 0.4f, 1.0f);
            ImGui::TextColored(statusColor, config.isPaused ? "PAUSED" : "RUNNING");
            ImGui::SameLine();
            if (ImGui::Button(config.isPaused ? "Resume" : "Pause", ImVec2(80, 25))) {
                config.isPaused = !config.isPaused;
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset", ImVec2(60, 25))) {
                particleSystem.reset();
            }
            
            ImGui::Separator();
            
            // Particle count with live feedback
            ImGui::Text("Particles: %d total", (int)particleSystem.getParticles().size());
            if (ImGui::SliderInt("Per Type", &tempConfig.newParticlesPerType, 0, 1000, "%d particles")) {
                if (tempConfig.newParticlesPerType != config.particlesPerType) {
                    particleSystem.setParticleCount(tempConfig.newParticlesPerType * config.numTypes);
                }
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Set to 0 to create an empty simulation");
            }
            
            // Particle types with visual feedback
            ImGui::Text("Types: %d colors active", config.numTypes);
            if (ImGui::SliderInt("Particle Types", &tempConfig.newNumTypes, 2, 8, "%d types")) {
                if (tempConfig.newNumTypes != config.numTypes) {
                    particleSystem.setNumTypes(tempConfig.newNumTypes);
                    tempConfig.newNumTypes = config.numTypes; // Sync back
                }
            }
            
            ImGui::PopItemWidth();
        }
        
        // Physics parameters section
        if (ImGui::CollapsingHeader("Physics Parameters", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushItemWidth(-120);
            
            ImGui::SliderFloat("Force Strength", &config.forceFactor, 0.1f, 2.0f, "%.2f");
            ImGui::SliderFloat("Max Speed", &config.maxSpeed, 0.001f, 0.05f, "%.3f");
            ImGui::SliderFloat("Friction", &config.friction, 0.9f, 0.999f, "%.3f");
            ImGui::SliderFloat("Interaction Range", &config.interactionRadius, 0.1f, 0.5f, "%.2f");
            
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 1.0f, 1.0f), "Mouse Interaction");
            ImGui::SliderFloat("Mouse Force", &config.mouseForce, 0.001f, 0.2f, "%.3f");
            ImGui::SliderFloat("Mouse Radius", &config.mouseRadius, 0.1f, 0.8f, "%.2f");
            
            // Show current mouse status
            if (config.mousePressed) {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.4f, 1.0f), "Mouse: ACTIVE (%.2f, %.2f)", config.mouseX, config.mouseY);
            } else {
                ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Mouse: Click and drag to attract");
            }
            
            // Particle spawning controls
            ImGui::Separator();
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Click Mode");
            
            // Mouse mode selection
            ImGui::RadioButton("🎯 Spawn Mode", &config.mouseMode, 0);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Left-click spawns single particles");
            }
            ImGui::SameLine();
            ImGui::RadioButton("💫 Interact Mode", &config.mouseMode, 1);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Left-click attracts particles with force");
            }
            
            // Show current mode status
            if (config.mouseMode == 0) {
                ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "🎯 Click anywhere to spawn particles");
            } else {
                if (particleSystem.getParticles().empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "⚠️ No particles to interact with - auto-spawn mode");
                } else {
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "💫 Click and drag to attract particles");
                }
            }
            
            ImGui::Separator();
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Advanced Spawning");
            
            ImGui::Checkbox("Enable Multi-Spawn Mode", &config.enableParticleSpawning);
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Right-click to spawn multiple particles at once");
            }
            
            if (config.enableParticleSpawning) {
                ImGui::SliderInt("Spawn Count", &config.spawnCount, 1, 50, "%d particles");
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Number of particles to spawn with right-click");
                }
            }
            
            // Particle type selection
            ImGui::SliderInt("Spawn Type", &config.spawnParticleType, 0, config.numTypes - 1, "Type %d");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Choose which particle type to spawn (affects color and behavior)");
            }
            
            ImGui::PopItemWidth();
        }
        
        // Multi-threading controls
        if (ImGui::CollapsingHeader("Performance Settings")) {
            ImGui::Checkbox("Enable Multi-threading", &config.enableMultiThreading);
            if (config.enableMultiThreading) {
                ImGui::PushItemWidth(-120);
                ImGui::SliderInt("Thread Count", &config.numThreads, 0, 16);
                if (config.numThreads == 0) {
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(auto-detect)");
                }
                ImGui::SliderInt("Min Particles for Threading", &config.minParticlesForThreading, 50, 1000);
                ImGui::PopItemWidth();
            }
        }
        
        // Quick panel access
        ImGui::Separator();
        ImGui::Text("Panel Access");
        
        ImVec2 buttonSize(ImGui::GetContentRegionAvail().x * 0.48f, 30);
        
        if (ImGui::Button("Visual Effects", buttonSize)) {
            showVisualEffects = !showVisualEffects;
        }
        ImGui::SameLine();
        if (ImGui::Button("Force Editor", buttonSize)) {
            showForceMatrix = !showForceMatrix;
        }
        
        if (ImGui::Button("Performance", buttonSize)) {
            showPerformanceHUD = !showPerformanceHUD;
        }
        ImGui::SameLine();
        if (ImGui::Button("Interactions", buttonSize)) {
            showInteraction = !showInteraction;
        }
    }
    
    // Integrated Performance Section
    if (ImGui::CollapsingHeader("Performance Monitor", showPerformanceHUD ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        const auto& metrics = particleSystem.getPerformanceMetrics();
        const auto& particles = particleSystem.getParticles();
        
        float fps = metrics.averageFPS;
        ImVec4 fpsColor = fps > 50 ? ImVec4(0,1,0,1) : fps > 30 ? ImVec4(1,1,0,1) : ImVec4(1,0,0,1);
        
        ImGui::TextColored(fpsColor, "FPS: %.1f", fps);
        ImGui::Text("Particles: %zu", particles.size());
        ImGui::Text("Update: %.2fms", metrics.updateTimeMs);
        ImGui::Text("Render: %.2fms", metrics.renderTimeMs);
    }
    
    // Integrated Visual Effects Section  
    if (ImGui::CollapsingHeader("Visual Effects", showVisualEffects ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        auto& renderConfig = renderer.getConfig();
        
        ImGui::PushItemWidth(-100);
        
        ImGui::Checkbox("Particle Trails", &renderConfig.enableTrails);
        if (renderConfig.enableTrails) {
            ImGui::SliderFloat("Trail Fade", &renderConfig.trailLength, 0.5f, 0.99f, "%.2f");
            ImGui::SliderFloat("Trail Intensity", &renderConfig.trailIntensity, 0.1f, 1.0f, "%.2f");
        }
        
        ImGui::Separator();
        
        ImGui::Checkbox("Particle Glow", &renderConfig.enableGlow);
        ImGui::Checkbox("Color by Speed", &renderConfig.colorBySpeed);
        if (renderConfig.colorBySpeed) {
            ImGui::SliderFloat("Max Speed", &renderConfig.maxSpeed, 0.001f, 0.1f, "%.3f");
        }
        
        ImGui::PopItemWidth();
    }
    
    // Integrated Force Matrix Section
    if (ImGui::CollapsingHeader("Force Matrix Editor", showForceMatrix ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        auto& config = particleSystem.getConfig();
        
        ImGui::Text("Edit forces between particle types:");
        ImGui::Text("Positive = Attraction, Negative = Repulsion");
        ImGui::Separator();
        
        // Force matrix grid
        for (int from = 0; from < config.numTypes; ++from) {
            ImGui::Text("Type %d Forces:", from);
            ImGui::Indent();
            
            for (int to = 0; to < config.numTypes; ++to) {
                ImGui::PushID(from * 10 + to);
                
                float force = particleSystem.getForce(from, to);
                ImGui::PushItemWidth(60);
                if (ImGui::SliderFloat(("##force_" + std::to_string(from) + "_" + std::to_string(to)).c_str(), 
                                     &force, -1.0f, 1.0f, "%.2f")) {
                    particleSystem.setForce(from, to, force);
                }
                ImGui::PopItemWidth();
                
                ImGui::SameLine();
                ImGui::Text("→ Type %d", to);
                
                ImGui::PopID();
            }
            
            ImGui::Unindent();
        }
    }
    
    // Integrated Quick Presets Section
    if (ImGui::CollapsingHeader("Quick Presets", showInteraction ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        auto& config = particleSystem.getConfig();
        ImVec2 buttonSize(ImGui::GetContentRegionAvail().x * 0.48f, 25);
        
        if (ImGui::Button("Life Pattern", buttonSize)) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    if (i == j) {
                        particleSystem.setForce(i, j, -0.4f);
                    } else if (abs(i - j) == 1 || (i == 0 && j == config.numTypes - 1) || 
                              (i == config.numTypes - 1 && j == 0)) {
                        particleSystem.setForce(i, j, 0.3f);
                    } else {
                        particleSystem.setForce(i, j, -0.1f);
                    }
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Random", buttonSize)) {
            particleSystem.generateRandomForces();
        }
        
        if (ImGui::Button("Reset All", buttonSize)) {
            particleSystem.reset();
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Forces", buttonSize)) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    particleSystem.setForce(i, j, 0.0f);
                }
            }
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
    if (ImGui::Begin("Performance", &showPerformanceHUD, flags)) {
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


void Interface::renderForceMatrixPanel() {
    if (!showForceMatrix) return;
    
    ImGui::SetNextWindowPos(ImVec2(1100, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);
    
    if (ImGui::Begin("Force Matrix Editor", &showForceMatrix)) {
        auto& config = particleSystem.getConfig();
        
        ImGui::Text("Interaction Forces");
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
                snprintf(label, sizeof(label), "Type %d → Type %d", from, to);
                
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
        ImGui::Text("Quick Presets:");
        
        if (ImGui::Button("Mutual Attraction", ImVec2(-1, 0))) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    particleSystem.setForce(i, j, 0.5f);
                }
            }
        }
        
        if (ImGui::Button("Mutual Repulsion", ImVec2(-1, 0))) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    particleSystem.setForce(i, j, -0.5f);
                }
            }
        }
        
        if (ImGui::Button("Self-Repel Only", ImVec2(-1, 0))) {
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
        
        if (ImGui::Button("Randomize All", ImVec2(-1, 0))) {
            particleSystem.generateRandomForces();
        }
        
        if (ImGui::Button("Reset to Zero", ImVec2(-1, 0))) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    particleSystem.setForce(i, j, 0.0f);
                }
            }
        }
        
        ImGui::Separator();
        
        // Visual force representation
        ImGui::Text("Force Matrix Visualization");
        
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

void Interface::renderAdvancedSettingsPanel() {
    if (!showVisualEffects) return;
    
    ImGui::SetNextWindowPos(ImVec2(420, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 380), ImGuiCond_FirstUseEver);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("Advanced Settings", &showVisualEffects, flags)) {
        auto& renderConfig = renderer.getConfig();
        
        if (ImGui::CollapsingHeader("Visual Effects", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushItemWidth(-100);
            
            ImGui::Checkbox("Particle Trails", &renderConfig.enableTrails);
            if (renderConfig.enableTrails) {
                ImGui::SliderFloat("Trail Fade", &renderConfig.trailLength, 0.5f, 0.99f, "%.2f");
                ImGui::SliderFloat("Trail Intensity", &renderConfig.trailIntensity, 0.1f, 1.0f, "%.2f");
            }
            
            ImGui::Separator();
            
            ImGui::Checkbox("Particle Glow", &renderConfig.enableGlow);
            
            ImGui::SliderFloat("Particle Size", &renderConfig.particleSize, 2.0f, 20.0f, "%.1f px");
            
            ImGui::Checkbox("Size by Speed", &renderConfig.sizeBySpeed);
            if (renderConfig.sizeBySpeed) {
                ImGui::SliderFloat("Min Size", &renderConfig.minParticleSize, 1.0f, 10.0f, "%.1f");
                ImGui::SliderFloat("Max Size", &renderConfig.maxParticleSize, 5.0f, 25.0f, "%.1f");
            }
            
            ImGui::Separator();
            
            ImGui::Checkbox("Color by Speed", &renderConfig.colorBySpeed);
            ImGui::Checkbox("Show Velocity Vectors", &renderConfig.showVelocityVectors);
            
            ImGui::PopItemWidth();
        }
        
        if (ImGui::CollapsingHeader("Rendering Options")) {
            ImGui::Checkbox("Show Grid", &renderConfig.showGrid);
            ImGui::Checkbox("Show Center", &renderConfig.showCenter);
            
            ImGui::Separator();
            ImGui::Text("Color Schemes:");
            if (ImGui::RadioButton("Type Colors", renderConfig.useTypeColors)) {
                renderConfig.useTypeColors = true;
                renderConfig.useVelocityColors = false;
            }
            if (ImGui::RadioButton("Velocity Colors", renderConfig.useVelocityColors)) {
                renderConfig.useTypeColors = false;
                renderConfig.useVelocityColors = true;
            }
        }
        
        if (ImGui::CollapsingHeader("Animation Effects")) {
            ImGui::Checkbox("Pulse Particles", &renderConfig.enablePulsation);
            if (renderConfig.enablePulsation) {
                ImGui::SliderFloat("Pulse Speed", &renderConfig.pulsationSpeed, 0.5f, 5.0f, "%.1f");
                ImGui::SliderFloat("Pulse Amount", &renderConfig.pulsationAmount, 0.1f, 1.0f, "%.2f");
            }
        }
    }
    ImGui::End();
}

void Interface::renderQuickActionsPanel() {
    if (!showInteraction) return;
    
    ImGui::SetNextWindowPos(ImVec2(420, 420), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(320, 200), ImGuiCond_FirstUseEver);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin("Particle Management", &showInteraction, flags)) {
        auto& config = particleSystem.getConfig();
        
        if (ImGui::CollapsingHeader("Quick Presets", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImVec2 buttonSize(ImGui::GetContentRegionAvail().x * 0.48f, 35);
            
            if (ImGui::Button("Life Pattern", buttonSize)) {
                // Create life-like behavior
                for (int i = 0; i < config.numTypes; ++i) {
                    for (int j = 0; j < config.numTypes; ++j) {
                        if (i == j) {
                            particleSystem.setForce(i, j, -0.4f); // Self-repulsion
                        } else if (abs(i - j) == 1 || (i == 0 && j == config.numTypes - 1) || (i == config.numTypes - 1 && j == 0)) {
                            particleSystem.setForce(i, j, 0.3f); // Neighbor attraction
                        } else {
                            particleSystem.setForce(i, j, -0.1f); // Weak repulsion
                        }
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Chaos Mode", buttonSize)) {
                particleSystem.generateRandomForces();
            }
            
            if (ImGui::Button("Mutual Attraction", buttonSize)) {
                for (int i = 0; i < config.numTypes; ++i) {
                    for (int j = 0; j < config.numTypes; ++j) {
                        particleSystem.setForce(i, j, 0.4f);
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Mutual Repulsion", buttonSize)) {
                for (int i = 0; i < config.numTypes; ++i) {
                    for (int j = 0; j < config.numTypes; ++j) {
                        particleSystem.setForce(i, j, -0.4f);
                    }
                }
            }
            
            if (ImGui::Button("Reset Forces", buttonSize)) {
                for (int i = 0; i < config.numTypes; ++i) {
                    for (int j = 0; j < config.numTypes; ++j) {
                        particleSystem.setForce(i, j, 0.0f);
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset All", buttonSize)) {
                particleSystem.reset();
            }
        }
        
        if (ImGui::CollapsingHeader("Live Adjustments")) {
            ImGui::PushItemWidth(-100);
            
            // Quick particle count adjustment
            int currentParticles = particleSystem.getParticles().size();
            int targetParticles = currentParticles;
            if (ImGui::SliderInt("Live Particle Count", &targetParticles, 100, 5000)) {
                int particlesPerType = targetParticles / config.numTypes;
                particleSystem.setParticleCount(particlesPerType * config.numTypes);
                tempConfig.newParticlesPerType = particlesPerType;
            }
            
            // Quick force scaling
            ImGui::SliderFloat("Global Force Scale", &config.forceFactor, 0.0f, 3.0f, "%.2f");
            
            ImGui::PopItemWidth();
        }
    }
    ImGui::End();
}