#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "simulation/ParticleSystem.h"
#include "rendering/Renderer.h"
#include "ui/Interface.h"

#include <iostream>
#include <memory>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <filesystem>

// Application constants
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 900;
const int SIDEBAR_WIDTH = 350;  // Width of the side panel

static void getSizes(GLFWwindow* window, int& fbW, int& fbH, int& viewportW, int& viewportH) {
    // Use framebuffer size (in pixels) so HiDPI and window resizing behave correctly.
    glfwGetFramebufferSize(window, &fbW, &fbH);
    viewportW = std::max(1, fbW - SIDEBAR_WIDTH);
    viewportH = std::max(1, fbH);
}

// Global application state
struct AppState {
    std::unique_ptr<ParticleSystem> particleSystem;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Interface> interface;
    GLFWwindow* window = nullptr;
} g_app;

// Callback functions
void mouseCallback(GLFWwindow* /*window*/, double x, double y) {
    if (g_app.particleSystem) {
        // Only handle mouse input in the simulation viewport (left side)
        int fbW = 0, fbH = 0, viewportW = 0, viewportH = 0;
        getSizes(g_app.window, fbW, fbH, viewportW, viewportH);

        if (x >= viewportW) {
            return; // Ignore mouse in side panel area
        }
        
        // Convert to viewport coordinates
        float mouseX = (2.0f * static_cast<float>(x)) / static_cast<float>(viewportW) - 1.0f;
        float mouseY = 1.0f - (2.0f * static_cast<float>(y)) / static_cast<float>(viewportH);
        g_app.particleSystem->setMousePosition(mouseX, mouseY);
    }
}

void mouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
    if (!g_app.particleSystem) return;
    
    auto& config = g_app.particleSystem->getConfig();
    
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Check mouse mode or particle count to determine behavior
        if (config.mouseMode == 0 || g_app.particleSystem->getParticles().empty()) {
            // Spawn mode or zero particles - spawn a single particle
            std::cout << "🎯 Spawning single particle at mouse position..." << std::endl;
            g_app.particleSystem->spawnParticlesAtMouse(1, config.spawnParticleType);
        } else {
            // Normal interaction mode - set mouse pressed for particle interaction
            g_app.particleSystem->setMousePressed(true);
        }
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        if (config.mouseMode != 0 && !g_app.particleSystem->getParticles().empty()) {
            g_app.particleSystem->setMousePressed(false);
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // Right click to spawn multiple particles
        if (config.enableParticleSpawning) {
            std::cout << "🌟 Spawning " << config.spawnCount << " particles at mouse position..." << std::endl;
            g_app.particleSystem->spawnParticlesAtMouse(config.spawnCount, config.spawnParticleType);
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        // Middle click to remove particles
        std::cout << "🗑️ Removing particles at mouse position..." << std::endl;
        g_app.particleSystem->removeParticlesAtMouse(config.mouseRadius);
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
    
    // Try focused window capture first
    std::string command1 = "screencapture -w \"" + filename + "\" 2>/dev/null";
    int result1 = std::system(command1.c_str());
    
    if (result1 == 0) {
        std::cout << "📸 Screenshot saved: " << filename << std::endl;
        return;
    }
    
    // Fallback to interactive selection
    std::cout << "⚠️  Auto-capture failed, opening selection mode..." << std::endl;
    std::string fallback_command = "screencapture -i \"" + filename + "\" 2>/dev/null";
    int result2 = std::system(fallback_command.c_str());
    
    if (result2 == 0) {
        std::cout << "📸 Interactive screenshot saved: " << filename << std::endl;
    } else {
        std::cout << "❌ Screenshot failed - please use external screenshot tool" << std::endl;
    }
}

void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_PRESS && g_app.particleSystem) {
        auto& config = g_app.particleSystem->getConfig();
        
        if (key == GLFW_KEY_SPACE) {
            config.paused = !config.paused;
            std::cout << "⏸️ Pause toggled: " << (config.paused ? "PAUSED" : "RESUMED") << std::endl;
        } else if (key == GLFW_KEY_R) {
            g_app.particleSystem->resetSimulation(true);
            std::cout << "🔄 Simulation reset" << std::endl;
        } else if (key == GLFW_KEY_P) {
            std::cout << "📸 P key pressed - taking screenshot..." << std::endl;
            takeScreenshot();
        }
    }
}

bool initializeOpenGL() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Create window
    g_app.window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Particle Life", nullptr, nullptr);
    if (!g_app.window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(g_app.window);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return false;
    }
    
    // Set callbacks
    glfwSetCursorPosCallback(g_app.window, mouseCallback);
    glfwSetMouseButtonCallback(g_app.window, mouseButtonCallback);
    glfwSetKeyCallback(g_app.window, keyCallback);
    
    return true;
}

bool initializeImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Modern dark theme with improved colors and spacing
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Modern rounded corners
    style.WindowRounding = 8.0f;
    style.ChildRounding = 6.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 6.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 6.0f;
    
    // Improved spacing
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(6, 4);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 12.0f;
    
    // Modern color scheme
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border]                 = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
    
    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(g_app.window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        return false;
    }
    
    if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
        std::cerr << "Failed to initialize ImGui OpenGL backend" << std::endl;
        return false;
    }
    
    return true;
}

bool initializeApplication() {
    // Initialize OpenGL and window
    if (!initializeOpenGL()) {
        return false;
    }
    
    // Initialize ImGui
    if (!initializeImGui()) {
        return false;
    }
    
    // Create application components
    g_app.particleSystem = std::make_unique<ParticleSystem>();
    g_app.renderer = std::make_unique<Renderer>();
    g_app.interface = std::make_unique<Interface>(*g_app.particleSystem, *g_app.renderer);
    
    // Initialize components
    if (!g_app.renderer->initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }
    
    if (!g_app.interface->initialize()) {
        std::cerr << "Failed to initialize interface" << std::endl;
        return false;
    }
    
    std::cout << "\n=== Particle Life Simulation ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  SPACE - Pause/Resume" << std::endl;
    std::cout << "  R - Randomize forces" << std::endl;
    std::cout << "  Left Click + Drag - Repel particles" << std::endl;
    std::cout << "  Right Click - Spawn particles" << std::endl;
    std::cout << "  Middle Click - Remove particles" << std::endl;
    std::cout << "\nNew Features:" << std::endl;
    std::cout << "  ✓ Real-time performance monitoring" << std::endl;
    std::cout << "  ✓ Interactive particle editing" << std::endl;
    std::cout << "  ✓ Enhanced visual effects" << std::endl;
    std::cout << "  ✓ Improved color-by-speed rendering" << std::endl;
    std::cout << "\nOptimizations:" << std::endl;
    std::cout << "  ✓ Balanced force values (±0.5 max)" << std::endl;
    std::cout << "  ✓ Reduced initial velocities (10x)" << std::endl;
    std::cout << "  ✓ Lower default force factor (0.3)" << std::endl;
    std::cout << "  ✓ Reduced max speed (0.01)" << std::endl;
    std::cout << "  ✓ Tighter particle clustering" << std::endl;
    std::cout << "  ✓ Modular architecture" << std::endl;
    std::cout << "\n💡 Tip: Most interesting behavior happens with:" << std::endl;
    std::cout << "   - Force Factor: 0.1 - 0.3" << std::endl;
    std::cout << "   - Force Matrix values: ±0.1 to ±0.4" << std::endl;
    std::cout << "   - Max Speed: 0.005 - 0.01" << std::endl;
    
    return true;
}

void cleanup() {
    // Cleanup application components
    if (g_app.interface) {
        g_app.interface->cleanup();
        g_app.interface.reset();
    }
    
    if (g_app.renderer) {
        g_app.renderer->cleanup();
        g_app.renderer.reset();
    }
    
    g_app.particleSystem.reset();
    
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    // Cleanup GLFW
    if (g_app.window) {
        glfwDestroyWindow(g_app.window);
    }
    glfwTerminate();
}

int main() {
    if (!initializeApplication()) {
        cleanup();
        return -1;
    }
    
    // Main application loop
    while (!glfwWindowShouldClose(g_app.window)) {
        // Update simulation
        g_app.particleSystem->update();

        // Use the actual framebuffer size (windowed mode + HiDPI safe).
        int fbW = 0, fbH = 0, viewportW = 0, viewportH = 0;
        getSizes(g_app.window, fbW, fbH, viewportW, viewportH);

        // Set viewport to simulation area only (left side)
        glViewport(0, 0, viewportW, viewportH);
        
        // Render frame
        g_app.renderer->setupFrame();
        g_app.renderer->renderParticles(g_app.particleSystem->getParticles());
        
        // Reset viewport for ImGui rendering
        glViewport(0, 0, fbW, fbH);
        
        // Render UI
        g_app.interface->render();
        
        // Present frame
        glfwSwapBuffers(g_app.window);
        glfwPollEvents();
    }
    
    cleanup();
    
    std::cout << "\nSimulation ended" << std::endl;
    return 0;
}