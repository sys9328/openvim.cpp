#!/bin/bash
# Script to run openvim GUI application
# openvim - agentic development platform

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Error: build directory not found. Please run 'mkdir build && cd build && cmake .. && make' first."
    exit 1
fi

cd build

echo "openvim - agentic development platform"
echo "======================================"

# Check display availability first
if [ -z "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ]; then
    echo "No display detected. Using offscreen mode..."
    export QT_QPA_PLATFORM=offscreen
    echo "Note: GUI will run but won't be visible. Use in GUI environment to see the interface."
else
    echo "Display detected ($DISPLAY). Testing GUI compatibility..."

    # Test for library conflicts (common in snap environments)
    if ./openvim --test 2>&1 | grep -q "symbol lookup error"; then
        echo "Library conflicts detected. Using minimal Qt platform to avoid crashes..."
        export QT_QPA_PLATFORM=minimal
        echo "Note: GUI components will initialize but won't be visible."
        echo "This is a Qt platform that provides minimal functionality."
        echo ""
        echo "To see the GUI, try running outside of snap:"
        echo "  1. Exit snap environment: exit"
        echo "  2. Run: ./run_gui.sh"
        echo "  Or use a virtual display: xvfb-run -a ./run_gui.sh"
    else
        echo "GUI appears compatible. Running in GUI mode..."
    fi
fi

./openvim "$@"
