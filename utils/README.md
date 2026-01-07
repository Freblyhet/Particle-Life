# Screenshot Utilities

This folder contains utilities for automatically capturing screenshots of the Particle Life simulation.

## Quick Start

### 1. Simple Usage (Interactive Menu)
```bash
./utils/capture.sh
```

This will show you a menu with these options:
1. **Capture Window** - Automatically finds and captures the Particle Life window
2. **Interactive Selection** - Click and drag to select what to capture
3. **Timed Capture** - Takes 5 screenshots, 3 seconds apart (great for animations)
4. **Guided Sequence** - Step-by-step capture of different simulation states
5. **Generate README** - Creates markdown code for your screenshots

### 2. Direct Commands
```bash
# Capture the running Particle Life window
python3 utils/screenshot_capture.py

# Interactive selection (click and drag)
python3 utils/screenshot_capture.py --select

# Timed capture: 10 screenshots, 5 seconds apart
python3 utils/screenshot_capture.py --timed 5 10

# Guided sequence for documenting features
python3 utils/screenshot_capture.py --sequence

# Generate markdown for README
python3 utils/screenshot_capture.py --update-readme
```

## Features

### 🎯 Automatic Window Detection
The utility can automatically find and capture your running Particle Life simulation window.

### ⏰ Timed Capture
Perfect for creating sequences showing the simulation evolving over time:
- Set interval between captures
- Specify total number of screenshots
- Automatic timestamping

### 🎬 Guided Sequence Mode
Interactive mode that walks you through capturing different states:
- Initial state with default forces
- Mouse interaction demonstration
- Force Matrix panel open
- Visual effects with trails
- Chaos mode with random forces

### 📝 README Generation
Automatically generates markdown code for your screenshots:
- Finds all screenshots in the folder
- Sorts by newest first
- Creates properly formatted markdown
- Ready to copy into your README.md

## Output

Screenshots are saved to `ParticleLifeScreenshots/` with timestamped filenames:
- `particle_life_20241201_143022_456.png` - Window captures
- `selection_20241201_143022_456.png` - Interactive selections
- `initial_20241201_143022_456.png` - Guided sequence captures

## Platform Support

Currently optimized for **macOS** using the built-in `screencapture` command. The Python version is designed to be easily adaptable for other platforms.

## Requirements

- Python 3.x
- macOS (for automatic window detection)
- Running Particle Life simulation

## Troubleshooting

**Window capture not working?**
- Make sure Particle Life is running and visible
- Try the interactive selection mode instead
- Check that the window title contains "ParticleLife"

**Python script not found?**
- Make sure you're running the script from the project root
- Check that the utils folder exists and contains the Python script

**Permission errors?**
- Run `chmod +x utils/capture.sh` to make the script executable
- You might need to grant screen recording permissions to Terminal in System Preferences