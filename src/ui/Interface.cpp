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

// Helper function to get particle type color for UI preview
ImVec4 Interface::getParticleTypeColor(int type, int totalTypes) {
    // Generate colors based on HSV color wheel
    float hue = (float)type / (float)totalTypes;
    
    // Convert HSV to RGB
    float h = hue * 6.0f;
    float c = 1.0f; // Full saturation
    float x = 1.0f - fabs(fmod(h, 2.0f) - 1.0f);
    
    float r, g, b;
    if (h < 1.0f) { r = c; g = x; b = 0; }
    else if (h < 2.0f) { r = x; g = c; b = 0; }
    else if (h < 3.0f) { r = 0; g = c; b = x; }
    else if (h < 4.0f) { r = 0; g = x; b = c; }
    else if (h < 5.0f) { r = x; g = 0; b = c; }
    else { r = c; g = 0; b = x; }
    
    return ImVec4(r, g, b, 1.0f);
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
    colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);  // Fully opaque
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.0f);   // Fully opaque
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
    // Panel starts after simulation area + gap and fills remaining space.
    // ImGui uses logical pixels (same as window coordinates)
    const float uiWidthLogical = 350.0f;
    const float uiGapLogical = 20.0f;
    ImGuiIO& io = ImGui::GetIO();
    const float screenW = io.DisplaySize.x;  // This is in logical pixels
    const float screenH = io.DisplaySize.y;
    const float simulationWidth = screenW - uiWidthLogical - uiGapLogical;
    const float uiStartX = simulationWidth + uiGapLogical;
    const float uiWidth = uiWidthLogical;

    // Create UI panel with gap between simulation
    ImGui::SetNextWindowPos(ImVec2(uiStartX, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(uiWidth, screenH), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(1.0f);  // Solid background
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
                            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar |
                            ImGuiWindowFlags_NoBringToFrontOnFocus;
    // Don't pass a close pointer: this removes the (X) and prevents the panel being hidden.
    if (ImGui::Begin("Control Panel", nullptr, flags)) {
        
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
                    particleSystem.randomizeForces();
                }
                if (ImGui::MenuItem("Reset")) {
                    particleSystem.resetSimulation(true);
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        auto& config = particleSystem.getConfig();
        
        // Flattened section headers (no collapsing "folder" widgets)
        ImGui::SeparatorText("🎮 Simulation Control");
        ImGui::PushItemWidth(-120);

        // Status indicator with icon
        ImVec4 statusColor = config.paused ? ImVec4(1.0f, 0.6f, 0.2f, 1.0f) : ImVec4(0.2f, 1.0f, 0.4f, 1.0f);
        const char* statusIcon = config.paused ? "⏸️" : "▶️";
        const char* statusText = config.paused ? " PAUSED" : " RUNNING";
        ImGui::TextColored(statusColor, "%s%s", statusIcon, statusText);
        ImGui::SameLine();
        
        // Larger, more prominent control buttons
        ImGui::PushStyleColor(ImGuiCol_Button, config.paused ? ImVec4(0.2f, 0.8f, 0.3f, 0.8f) : ImVec4(1.0f, 0.5f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, config.paused ? ImVec4(0.3f, 0.9f, 0.4f, 1.0f) : ImVec4(1.0f, 0.6f, 0.3f, 1.0f));
        if (ImGui::Button(config.paused ? "▶️ Resume" : "⏸️ Pause", ImVec2(100, 30))) {
            config.paused = !config.paused;
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Pause/Resume the simulation [SPACE]");
        }
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
        if (ImGui::Button("🔄 Reset", ImVec2(80, 30))) {
            particleSystem.resetSimulation(true);
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Reset simulation with new random positions");
        }

        // Particle count display with visual indicator
        int totalParticles = (int)particleSystem.getParticles().size();
        ImGui::Spacing();
        ImGui::Text("📊 Total Particles: %d", totalParticles);
        
        // Visual progress bar for particle density
        float particleDensity = totalParticles / 5000.0f; // Assuming 5000 is "full"
        ImGui::ProgressBar(particleDensity, ImVec2(-1, 0), "");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Current particle density");
        }
        
        ImGui::Spacing();
        if (ImGui::SliderInt("Per Type", &tempConfig.newParticlesPerType, 0, 1000, "%d particles")) {
            if (tempConfig.newParticlesPerType != config.particlesPerType) {
                particleSystem.setParticleCount(tempConfig.newParticlesPerType * config.numTypes);
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Number of particles per type\nSet to 0 to create an empty simulation");
        }

        if (ImGui::SliderInt("🎨 Particle Types", &tempConfig.newNumTypes, 2, 8, "%d types")) {
            if (tempConfig.newNumTypes != config.numTypes) {
                particleSystem.setNumTypes(tempConfig.newNumTypes);
                tempConfig.newNumTypes = config.numTypes; // Sync back
            }
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Number of different particle types (colors)\nEach type can have unique interaction rules");
        }
        ImGui::PopItemWidth();
        
        ImGui::SeparatorText("⚙️ Physics Parameters");
        ImGui::PushItemWidth(-120);

        if (ImGui::SliderFloat("💪 Force Strength", &config.forceFactor, 0.1f, 2.0f, "%.2f")) {
            // Live update
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Global multiplier for all interaction forces");
        }
        
        if (ImGui::SliderFloat("⚡ Max Speed", &config.maxSpeed, 0.001f, 0.05f, "%.3f")) {
            // Live update
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Speed limit for particles");
        }
        
        if (ImGui::SliderFloat("🌬️ Friction", &config.friction, 0.9f, 0.999f, "%.3f")) {
            // Live update
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Higher = less friction (smoother motion)\nLower = more friction (slower particles)");
        }
        
        if (ImGui::SliderFloat("🎯 Interaction Range", &config.interactionRadius, 0.1f, 0.5f, "%.2f")) {
            // Live update
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Maximum distance at which particles can interact");
        };

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::SeparatorText("🖱️ Mouse Interaction");
        
        if (ImGui::SliderFloat("Mouse Force", &config.mouseForce, 0.001f, 0.2f, "%.3f")) {
            // Live update
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Strength of mouse attraction/repulsion");
        }
        
        if (ImGui::SliderFloat("Mouse Radius", &config.mouseRadius, 0.1f, 0.8f, "%.2f")) {
            // Live update
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Range of mouse influence on particles");
        }
            
        // Show current mouse status with better visual feedback
        ImGui::Spacing();
        if (config.mousePressed) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.4f, 1.0f));
            ImGui::TextWrapped("🟢 Mouse ACTIVE at (%.2f, %.2f)", config.mouseX, config.mouseY);
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::TextWrapped("⚪ Click and drag to attract particles");
            ImGui::PopStyleColor();
        }
            
        // Particle spawning controls
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::SeparatorText("✨ Click Mode");
        
        // Mouse mode selection with better visual style
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.3f, 0.8f, 1.0f, 1.0f));
        if (ImGui::RadioButton("🎯 Spawn Mode", &config.mouseMode, 0)) {
            // Mode changed
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Left-click spawns single particles at cursor location");
        }
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.6f, 0.3f, 1.0f));
        if (ImGui::RadioButton("💫 Interact Mode", &config.mouseMode, 1)) {
            // Mode changed
        }
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Left-click and drag to attract particles with force");
        }
            
        // Show current mode status with enhanced visuals
        ImGui::Spacing();
        if (config.mouseMode == 0) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f));
            ImGui::TextWrapped("🎯 Click anywhere to spawn particles");
            ImGui::PopStyleColor();
        } else {
            if (particleSystem.getParticles().empty()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.3f, 1.0f));
                ImGui::TextWrapped("⚠️ No particles to interact with");
                ImGui::Text("Auto-switching to spawn mode...");
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.8f, 1.0f, 1.0f));
                ImGui::TextWrapped("💫 Click and drag to attract");
                ImGui::PopStyleColor();
            }
        }
            
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("🌟 Advanced Spawning");
        
        if (ImGui::Checkbox("Enable Multi-Spawn", &config.enableParticleSpawning)) {
            // Spawn mode toggled
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Right-click to spawn multiple particles at once");
        }
            
        if (config.enableParticleSpawning) {
            ImGui::Indent();
            if (ImGui::SliderInt("Spawn Count", &config.spawnCount, 1, 50, "%d particles")) {
                // Live update
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Number of particles spawned per right-click");
            }
            ImGui::Unindent();
        }

        // Particle type selection with visual indicator
        ImGui::Spacing();
        char typeLabel[64];
        snprintf(typeLabel, sizeof(typeLabel), "🎨 Spawn Type ##spawntype");
        if (ImGui::SliderInt(typeLabel, &config.spawnParticleType, 0, config.numTypes - 1, "Type %d")) {
            // Live update
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Choose which particle type to spawn\nEach type has unique color and interaction behavior");
        }
        
        // Visual preview of selected type color
        ImVec4 typeColor = getParticleTypeColor(config.spawnParticleType, config.numTypes);
        ImGui::SameLine();
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 colorBoxPos = ImGui::GetCursorScreenPos();
        drawList->AddRectFilled(
            colorBoxPos,
            ImVec2(colorBoxPos.x + 20, colorBoxPos.y + 20),
            ImGui::ColorConvertFloat4ToU32(typeColor)
        );
        drawList->AddRect(
            colorBoxPos,
            ImVec2(colorBoxPos.x + 20, colorBoxPos.y + 20),
            IM_COL32(255, 255, 255, 255),
            0.0f,
            0,
            1.5f
        );
        ImGui::Dummy(ImVec2(20, 20));
        
        ImGui::PopItemWidth();
        
        // Performance settings: removed multi-threading toggles.
        // The current simulation update path is single-threaded, so exposing these options
        // would be misleading.
        
        // No separate "Panel Access" buttons: this UI is a single, unified control panel.
    }
    
    // Integrated Performance Section with enhanced visuals
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("📊 Performance Monitor", showPerformanceHUD ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        const auto& metrics = particleSystem.getMetrics();
        const auto& particles = particleSystem.getParticles();
        
        float fps = metrics.averageFPS;
        ImVec4 fpsColor = fps > 50 ? ImVec4(0.2f, 1.0f, 0.3f, 1.0f) : 
                          fps > 30 ? ImVec4(1.0f, 1.0f, 0.2f, 1.0f) : 
                          ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
        
        // FPS display with visual indicator
        ImGui::Text("Frame Rate:");
        ImGui::SameLine();
        ImGui::TextColored(fpsColor, "%.1f FPS", fps);
        
        // FPS progress bar
        float fpsNormalized = std::min(fps / 60.0f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, fpsColor);
        ImGui::ProgressBar(fpsNormalized, ImVec2(-1, 0), "");
        ImGui::PopStyleColor();
        
        ImGui::Separator();
        ImGui::Text("🔢 Particle Count: %zu", particles.size());
        ImGui::Text("⚙️ Update Time: %.2f ms", metrics.updateTimeMs);
        ImGui::Text("🎨 Render Time: %.2f ms", metrics.renderTimeMs);
        
        float totalTime = metrics.updateTimeMs + metrics.renderTimeMs;
        ImGui::Spacing();
        ImGui::Text("Total Frame: %.2f ms", totalTime);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Target: 16.67ms for 60 FPS");
        }
    }
    
    // Integrated Visual Effects Section with icons
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("✨ Visual Effects", showVisualEffects ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        auto& renderConfig = renderer.getConfig();

        ImGui::PushID("IntegratedVisualEffects");
        ImGui::PushItemWidth(-100);

        if (ImGui::Checkbox("🌟 Particle Glow", &renderConfig.enableGlow)) {
            // Glow toggled
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Enable soft glow effect around particles");
        }
        
        if (ImGui::Checkbox("🌈 Color by Speed", &renderConfig.colorBySpeed)) {
            // Color mode toggled
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Change particle colors based on their velocity\nFaster particles = Different colors");
        }
        
        if (renderConfig.colorBySpeed) {
            ImGui::Indent();
            ImGui::Text("Speed Range:");
            if (ImGui::SliderFloat("##MaxSpeed", &renderConfig.maxSpeed, 0.001f, 0.1f, "%.3f")) {
                // Live update
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Maximum speed for color mapping");
            }
            ImGui::Unindent();
        }
        
        ImGui::PopItemWidth();
        ImGui::PopID();
    }
    
    // Integrated Force Matrix Section with enhanced visuals
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("🎛️ Force Matrix Editor", showForceMatrix ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        auto& config = particleSystem.getConfig();
        
        ImGui::TextWrapped("Edit interaction forces between particle types");
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.3f, 1.0f));
        ImGui::Text("🟢 Positive = Attraction");
        ImGui::PopStyleColor();
        ImGui::SameLine(200);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.2f, 1.0f));
        ImGui::Text("🔴 Negative = Repulsion");
        ImGui::PopStyleColor();
        ImGui::Separator();
        
        // Force matrix grid
        for (int from = 0; from < config.numTypes; ++from) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 1.0f, 1.0f));
            ImGui::Text("Type %d →", from);
            ImGui::PopStyleColor();
            ImGui::Indent();
            
            for (int to = 0; to < config.numTypes; ++to) {
                ImGui::PushID(from * 10 + to);
                
                float force = particleSystem.getForce(from, to);
                ImGui::PushItemWidth(60);
                
                // Color-coded slider based on force value
                ImVec4 sliderColor;
                if (force > 0.1f) {
                    sliderColor = ImVec4(0.2f, 1.0f, 0.2f, 1.0f);
                } else if (force < -0.1f) {
                    sliderColor = ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                } else {
                    sliderColor = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
                }
                
                ImGui::PushStyleColor(ImGuiCol_SliderGrab, sliderColor);
                ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, sliderColor);
                
                if (ImGui::SliderFloat(("##force_" + std::to_string(from) + "_" + std::to_string(to)).c_str(), 
                                     &force, -1.0f, 1.0f, "%.2f")) {
                    particleSystem.setForce(from, to, force);
                }
                ImGui::PopStyleColor(2);
                ImGui::PopItemWidth();
                
                ImGui::SameLine();
                ImGui::Text("Type %d", to);
                if (ImGui::IsItemHovered()) {
                    const char* forceType = force > 0.1f ? "Attraction" : 
                                          force < -0.1f ? "Repulsion" : "Neutral";
                    ImGui::SetTooltip("Type %d → Type %d: %s (%.2f)", from, to, forceType, force);
                }
                
                ImGui::PopID();
            }
            
            ImGui::Unindent();
        }
    }
    
    // Integrated Quick Presets Section with better organization
    ImGui::Spacing();
    if (ImGui::CollapsingHeader("🎭 Quick Presets", showInteraction ? ImGuiTreeNodeFlags_DefaultOpen : 0)) {
        auto& config = particleSystem.getConfig();
        ImVec2 buttonSize(ImGui::GetContentRegionAvail().x * 0.48f, 30);
        
        ImGui::Text("Pattern Presets:");
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.4f, 1.0f));
        if (ImGui::Button("🌱 Life Pattern", buttonSize)) {
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
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Self-repulsion with neighbor attraction\nCreates organic, life-like patterns");
        }
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.4f, 0.2f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.5f, 0.3f, 1.0f));
        if (ImGui::Button("🎲 Random", buttonSize)) {
            particleSystem.randomizeForces();
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Randomize all interaction forces\nDiscovery mode!");
        }
        
        ImGui::Spacing();
        ImGui::Text("Control Actions:");
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.3f, 0.3f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.4f, 0.4f, 1.0f));
        if (ImGui::Button("🔄 Reset All", buttonSize)) {
            particleSystem.resetSimulation(true);
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Reset entire simulation\nNew positions, forces intact");
        }
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        if (ImGui::Button("🧹 Clear Forces", buttonSize)) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    particleSystem.setForce(i, j, 0.0f);
                }
            }
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Zero all forces\nParticles drift freely");
        }
        
        ImGui::Spacing();
        ImGui::Text("Motion Control:");
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.7f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.6f, 0.8f, 1.0f));
        if (ImGui::Button("❄️ Freeze All", buttonSize)) {
            auto& particles = particleSystem.getParticles();
            for (auto& p : particles) {
                p.vx = 0.0f;
                p.vy = 0.0f;
            }
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Stop all particle motion\nForces still active");
        }
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.3f, 0.6f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.4f, 0.7f, 1.0f));
        if (ImGui::Button("🛑 Zero + Freeze", buttonSize)) {
            for (int i = 0; i < config.numTypes; ++i) {
                for (int j = 0; j < config.numTypes; ++j) {
                    particleSystem.setForce(i, j, 0.0f);
                }
            }
            auto& particles = particleSystem.getParticles();
            for (auto& p : particles) {
                p.vx = 0.0f;
                p.vy = 0.0f;
            }
        }
        ImGui::PopStyleColor(2);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Clear forces AND freeze motion\nComplete stop");
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
        const auto& metrics = particleSystem.getMetrics();
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
            particleSystem.randomizeForces();
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
    // Disabled: this panel duplicates the controls already present in the main control
    // window, and the duplicated labels create ImGui ID conflicts when both code paths
    // are reachable. If you want this window back, give it its own toggle flag and/or
    // wrap its widgets in a unique PushID scope.
    return;
    
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
                particleSystem.randomizeForces();
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
                particleSystem.resetSimulation(true);
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