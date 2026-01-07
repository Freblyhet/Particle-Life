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

// Application constants
const int SCREEN_WIDTH = 1400;
const int SCREEN_HEIGHT = 900;

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
        float mouseX = (2.0f * x) / SCREEN_WIDTH - 1.0f;
        float mouseY = 1.0f - (2.0f * y) / SCREEN_HEIGHT;
        g_app.particleSystem->setMousePosition(mouseX, mouseY);
    }
}

void mouseButtonCallback(GLFWwindow* /*window*/, int button, int action, int /*mods*/) {
    if (!g_app.particleSystem) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        g_app.particleSystem->setMousePressed(action == GLFW_PRESS);
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        // Right click to spawn particles
        auto& config = g_app.particleSystem->getConfig();
        if (config.enableParticleSpawning) {
            g_app.particleSystem->spawnParticlesAtMouse(config.spawnCount, config.spawnParticleType);
        }
    } else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        // Middle click to remove particles
        auto& config = g_app.particleSystem->getConfig();
        g_app.particleSystem->removeParticlesAtMouse(config.mouseRadius);
    }
}

void keyCallback(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action == GLFW_PRESS && g_app.particleSystem) {
        auto& config = g_app.particleSystem->getConfig();
        
        if (key == GLFW_KEY_SPACE) {
            config.paused = !config.paused;
        } else if (key == GLFW_KEY_R) {
            g_app.particleSystem->resetSimulation(true);
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
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
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
        
        // Render frame
        g_app.renderer->setupFrame();
        g_app.renderer->renderParticles(g_app.particleSystem->getParticles());
        
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