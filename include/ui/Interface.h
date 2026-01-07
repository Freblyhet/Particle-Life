#pragma once

#include "simulation/ParticleSystem.h"
#include "rendering/Renderer.h"
#include <imgui.h>
#include <glm/glm.hpp>
#include <memory>

class Interface {
private:
    ParticleSystem& particleSystem;
    Renderer& renderer;
    
    // Temporary state for structure changes
    struct TempConfig {
        int newNumTypes;
        int newParticlesPerType;
    } tempConfig;
    
    // Panel visibility flags
    bool showForceMatrix = false;
    bool showControlPanel = true;
    bool showPerformanceHUD = true;
    bool showVisualEffects = false;
    bool showInteraction = false;
    
    // Modern panel rendering functions
    void renderControlPanel();
    void renderPerformanceHUD();
    void renderVisualEffectsPanel();
    void renderInteractionPanel();
    void renderPresetsPanel();
    void renderForceMatrixPanel();
    void renderColorLegendPanel();
    
    // Legacy functions (keeping for compatibility)
    void renderStatus();
    void renderMainControls();
    void renderPresets();
    void renderStructure();
    void renderPhysics();
    void renderSpecialEffects();
    void renderVisual();
    void renderMouseInteraction();
    void renderPerformance();
    void renderForceMatrix();
    void renderColorLegend();

public:
    Interface(ParticleSystem& ps, Renderer& r);
    ~Interface() = default;
    
    bool initialize();
    void cleanup();
    void render();
};