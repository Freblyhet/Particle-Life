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
void Interface::renderVisualEffectsPanel() {}
void Interface::renderInteractionPanel() {}
void Interface::renderPresetsPanel() {}
void Interface::renderForceMatrixPanel() {}
void Interface::renderColorLegendPanel() {}