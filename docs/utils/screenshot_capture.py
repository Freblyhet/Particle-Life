#!/usr/bin/env python3
"""
Particle Life Screenshot Capture Utility
==========================================

Automatically captures screenshots of the running Particle Life simulation
and saves them to the ParticleLifeScreenshots folder for easy README updates.

Features:
- Automatic window detection and capture
- Timed capture mode for creating sequences
- Interactive selection mode
- Automatic timestamping and organization
"""

import subprocess
import time
import os
import sys
from datetime import datetime
from pathlib import Path
import argparse

class ScreenCapture:
    def __init__(self, output_dir="ParticleLifeScreenshots"):
        # Get project root (parent of current directory)
        self.project_root = Path(__file__).parent.parent
        self.output_dir = self.project_root / output_dir
        
        # Create output directory if it doesn't exist
        self.output_dir.mkdir(exist_ok=True)
        print(f"📁 Output directory: {self.output_dir}")
    
    def get_timestamp(self):
        """Generate timestamp string for filename"""
        return datetime.now().strftime("%Y%m%d_%H%M%S_%f")[:-3]
    
    def capture_particle_life_window(self):
        """Capture the Particle Life window specifically"""
        timestamp = self.get_timestamp()
        filename = f"particle_life_{timestamp}.png"
        filepath = self.output_dir / filename
        
        print(f"🎯 Capturing Particle Life window...")
        
        try:
            # macOS: Try to find and capture the ParticleLife window
            cmd = [
                "screencapture", 
                "-l$(osascript -e 'tell app \"System Events\" to id of processes whose name contains \"ParticleLife\"')",
                str(filepath)
            ]
            
            # Run the command
            result = subprocess.run(" ".join(cmd), shell=True, capture_output=True, text=True)
            
            if result.returncode == 0:
                print(f"✅ Screenshot saved: {filename}")
                return str(filepath)
            else:
                print("⚠️  Window capture failed, trying interactive selection...")
                return self.capture_interactive_selection()
                
        except Exception as e:
            print(f"❌ Error capturing window: {e}")
            return None
    
    def capture_interactive_selection(self):
        """Interactive selection capture"""
        timestamp = self.get_timestamp()
        filename = f"selection_{timestamp}.png"
        filepath = self.output_dir / filename
        
        print("🖱️  Click and drag to select the area to capture...")
        
        try:
            cmd = ["screencapture", "-i", str(filepath)]
            result = subprocess.run(cmd, check=True)
            print(f"✅ Selection screenshot saved: {filename}")
            return str(filepath)
        except subprocess.CalledProcessError:
            print("❌ Interactive capture cancelled or failed")
            return None
    
    def capture_fullscreen(self):
        """Capture entire screen"""
        timestamp = self.get_timestamp()
        filename = f"fullscreen_{timestamp}.png"
        filepath = self.output_dir / filename
        
        print("🖥️  Capturing full screen...")
        
        try:
            cmd = ["screencapture", str(filepath)]
            subprocess.run(cmd, check=True)
            print(f"✅ Full screen screenshot saved: {filename}")
            return str(filepath)
        except subprocess.CalledProcessError as e:
            print(f"❌ Full screen capture failed: {e}")
            return None
    
    def timed_capture(self, interval_seconds, total_captures):
        """Capture screenshots at timed intervals"""
        print(f"⏰ Starting timed capture: {total_captures} screenshots every {interval_seconds} seconds")
        print("   Make sure Particle Life is running and visible!")
        
        captured_files = []
        
        for i in range(total_captures):
            print(f"\n📸 Capture {i + 1} of {total_captures}")
            
            filepath = self.capture_particle_life_window()
            if filepath:
                captured_files.append(filepath)
            
            if i < total_captures - 1:
                print(f"⏳ Waiting {interval_seconds} seconds...")
                time.sleep(interval_seconds)
        
        print(f"\n🎉 Timed capture complete! {len(captured_files)} screenshots saved.")
        return captured_files
    
    def capture_sequence_with_instructions(self):
        """Capture a sequence with user instructions for different states"""
        print("🎬 Guided Screenshot Sequence")
        print("============================")
        print("This will help you capture different states of the simulation.")
        
        sequences = [
            ("initial", "Initial state with default forces"),
            ("interaction", "Mouse interaction (click and drag particles)"),
            ("force_matrix", "Force Matrix panel open"),
            ("visual_effects", "Visual Effects panel with trails enabled"),
            ("chaos", "Random forces applied (chaos mode)")
        ]
        
        captured_files = []
        
        for seq_name, instruction in sequences:
            print(f"\n📋 {seq_name.upper()}: {instruction}")
            input("   Set up the simulation as described, then press Enter to capture...")
            
            timestamp = self.get_timestamp()
            filename = f"{seq_name}_{timestamp}.png"
            filepath = self.output_dir / filename
            
            try:
                cmd = ["screencapture", "-i", str(filepath)]
                subprocess.run(cmd, check=True)
                print(f"✅ Captured: {filename}")
                captured_files.append(str(filepath))
            except subprocess.CalledProcessError:
                print(f"❌ Skipped: {seq_name}")
        
        print(f"\n🎉 Sequence complete! {len(captured_files)} screenshots saved.")
        return captured_files

def update_readme_with_screenshots(screenshot_dir):
    """Generate markdown for README with screenshots"""
    screenshot_path = Path(screenshot_dir)
    
    if not screenshot_path.exists():
        print("❌ Screenshot directory not found")
        return
    
    # Find all PNG files in the screenshot directory
    screenshots = list(screenshot_path.glob("*.png"))
    
    if not screenshots:
        print("❌ No screenshots found in directory")
        return
    
    # Sort by modification time (newest first)
    screenshots.sort(key=lambda x: x.stat().st_mtime, reverse=True)
    
    print(f"\n📝 Generating README markdown for {len(screenshots)} screenshots...")
    
    markdown = "## Screenshots\n\n"
    
    for screenshot in screenshots[:6]:  # Limit to 6 most recent
        relative_path = screenshot.relative_to(screenshot_path.parent)
        name = screenshot.stem.replace("_", " ").title()
        
        markdown += f"### {name}\n"
        markdown += f"![{name}]({relative_path})\n\n"
    
    output_file = screenshot_path.parent / "SCREENSHOTS.md"
    with open(output_file, "w") as f:
        f.write(markdown)
    
    print(f"✅ Markdown generated: {output_file}")
    print("\n📋 Copy this into your README.md:")
    print("=" * 40)
    print(markdown)

def main():
    parser = argparse.ArgumentParser(
        description="Particle Life Screenshot Capture Utility",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python screenshot_capture.py                    # Capture window
  python screenshot_capture.py --select           # Interactive selection
  python screenshot_capture.py --timed 5 10       # 10 captures, 5 seconds apart
  python screenshot_capture.py --sequence         # Guided sequence capture
  python screenshot_capture.py --update-readme    # Generate README markdown
        """
    )
    
    parser.add_argument("--select", action="store_true", help="Interactive selection capture")
    parser.add_argument("--fullscreen", action="store_true", help="Capture entire screen")
    parser.add_argument("--timed", nargs=2, type=int, metavar=("INTERVAL", "COUNT"),
                       help="Timed capture: interval(s) and count")
    parser.add_argument("--sequence", action="store_true", help="Guided sequence capture")
    parser.add_argument("--update-readme", action="store_true", help="Generate README markdown")
    
    args = parser.parse_args()
    
    capture = ScreenCapture()
    
    if args.select:
        capture.capture_interactive_selection()
    elif args.fullscreen:
        capture.capture_fullscreen()
    elif args.timed:
        interval, count = args.timed
        capture.timed_capture(interval, count)
    elif args.sequence:
        capture.capture_sequence_with_instructions()
    elif args.update_readme:
        update_readme_with_screenshots(capture.output_dir)
    else:
        # Default: capture window
        capture.capture_particle_life_window()

if __name__ == "__main__":
    main()