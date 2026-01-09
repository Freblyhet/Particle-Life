#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <sstream>

class ScreenCapture {
public:
    ScreenCapture(const std::string& outputDir) : outputDirectory(outputDir) {
        // Create screenshots directory if it doesn't exist
        std::filesystem::create_directories(outputDirectory);
    }
    
    void captureParticleLifeWindow() {
        std::string timestamp = getCurrentTimestamp();
        std::string filename = "particle_life_" + timestamp + ".png";
        std::string fullPath = outputDirectory + "/" + filename;
        
        // macOS screencapture command to capture specific window
        // First, try to find the Particle Life window
        std::string command = "screencapture -l$(osascript -e 'tell app \"System Events\" to id of processes whose name contains \"ParticleLife\"') \"" + fullPath + "\"";
        
        std::cout << "Capturing screenshot: " << filename << std::endl;
        int result = system(command.c_str());
        
        if (result == 0) {
            std::cout << "✅ Screenshot saved successfully: " << fullPath << std::endl;
        } else {
            // Fallback: capture interactive selection
            std::cout << "Window capture failed, falling back to interactive selection..." << std::endl;
            std::string fallbackCommand = "screencapture -i \"" + fullPath + "\"";
            system(fallbackCommand.c_str());
        }
    }
    
    void captureFullScreen() {
        std::string timestamp = getCurrentTimestamp();
        std::string filename = "fullscreen_" + timestamp + ".png";
        std::string fullPath = outputDirectory + "/" + filename;
        
        std::string command = "screencapture \"" + fullPath + "\"";
        
        std::cout << "Capturing full screen: " << filename << std::endl;
        int result = system(command.c_str());
        
        if (result == 0) {
            std::cout << "✅ Full screen screenshot saved: " << fullPath << std::endl;
        }
    }
    
    void captureInteractiveSelection() {
        std::string timestamp = getCurrentTimestamp();
        std::string filename = "selection_" + timestamp + ".png";
        std::string fullPath = outputDirectory + "/" + filename;
        
        std::cout << "Click and drag to select area to capture..." << std::endl;
        std::string command = "screencapture -i \"" + fullPath + "\"";
        
        int result = system(command.c_str());
        
        if (result == 0) {
            std::cout << "✅ Selection screenshot saved: " << fullPath << std::endl;
        }
    }
    
    void startTimedCapture(int intervalSeconds, int totalCaptures) {
        std::cout << "Starting timed capture: " << totalCaptures << " screenshots every " << intervalSeconds << " seconds" << std::endl;
        
        for (int i = 0; i < totalCaptures; ++i) {
            std::cout << "\nCapture " << (i + 1) << " of " << totalCaptures << std::endl;
            captureParticleLifeWindow();
            
            if (i < totalCaptures - 1) {
                std::cout << "Waiting " << intervalSeconds << " seconds..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));
            }
        }
        
        std::cout << "\n🎉 Timed capture complete! " << totalCaptures << " screenshots saved." << std::endl;
    }

private:
    std::string outputDirectory;
    
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
        ss << "_" << std::setfill('0') << std::setw(3) << ms.count();
        
        return ss.str();
    }
};

void printUsage(const char* programName) {
    std::cout << "Particle Life Screenshot Capture Utility\n";
    std::cout << "=========================================\n\n";
    std::cout << "Usage: " << programName << " [mode] [options]\n\n";
    std::cout << "Modes:\n";
    std::cout << "  window     - Capture Particle Life window (default)\n";
    std::cout << "  fullscreen - Capture entire screen\n";
    std::cout << "  select     - Interactive selection capture\n";
    std::cout << "  timed      - Timed automatic capture\n\n";
    std::cout << "Timed mode options:\n";
    std::cout << "  " << programName << " timed [interval_seconds] [total_captures]\n";
    std::cout << "  Example: " << programName << " timed 5 10\n";
    std::cout << "           (captures 10 screenshots, 5 seconds apart)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << programName << "                    # Capture Particle Life window\n";
    std::cout << "  " << programName << " select             # Interactive selection\n";
    std::cout << "  " << programName << " timed 3 5          # 5 captures, 3 seconds apart\n";
}

int main(int argc, char* argv[]) {
    // Get the project root directory
    std::string projectRoot = std::filesystem::current_path().parent_path().string();
    std::string screenshotDir = projectRoot + "/ParticleLifeScreenshots";
    
    ScreenCapture capture(screenshotDir);
    
    std::cout << "📸 Particle Life Screenshot Capture Utility\n";
    std::cout << "Output directory: " << screenshotDir << "\n\n";
    
    if (argc == 1) {
        // Default: capture window
        capture.captureParticleLifeWindow();
    } else if (argc == 2) {
        std::string mode = argv[1];
        
        if (mode == "help" || mode == "--help" || mode == "-h") {
            printUsage(argv[0]);
            return 0;
        } else if (mode == "window") {
            capture.captureParticleLifeWindow();
        } else if (mode == "fullscreen") {
            capture.captureFullScreen();
        } else if (mode == "select") {
            capture.captureInteractiveSelection();
        } else {
            std::cout << "❌ Unknown mode: " << mode << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    } else if (argc == 4 && std::string(argv[1]) == "timed") {
        int interval = std::atoi(argv[2]);
        int total = std::atoi(argv[3]);
        
        if (interval <= 0 || total <= 0) {
            std::cout << "❌ Invalid parameters. Interval and total must be positive integers." << std::endl;
            return 1;
        }
        
        capture.startTimedCapture(interval, total);
    } else {
        std::cout << "❌ Invalid arguments." << std::endl;
        printUsage(argv[0]);
        return 1;
    }
    
    return 0;
}