#!/bin/bash
# Script to run openvim QML GUI application
# openvim - agentic development platform

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "Error: build directory not found. Please run 'mkdir build && cd build && cmake .. && make' first."
    exit 1
fi

cd build

echo "openvim - agentic development platform (QML)"
echo "==========================================="

# Set QML import path for Qt modules
export QML_IMPORT_PATH=/usr/lib/x86_64-linux-gnu/qt5/qml:$QML_IMPORT_PATH

# Check display availability first
if [ -z "$DISPLAY" ] && [ -z "$WAYLAND_DISPLAY" ]; then
    echo "No display detected. Using minimal Qt platform..."
    export QT_QPA_PLATFORM=minimal
    echo "Note: QML GUI will run but won't be visible. Use in GUI environment to see the interface."
else
    echo "Display detected ($DISPLAY). Testing QML compatibility..."

    # Test for library conflicts (common in snap environments)
    if ./openvim --test 2>&1 | grep -q "symbol lookup error"; then
        echo "Library conflicts detected. Using minimal Qt platform to avoid crashes..."
        export QT_QPA_PLATFORM=minimal
        echo "Note: QML components will initialize but won't be visible."
        echo ""
    echo "To see the GUI, try these options:"
    echo "  1. Virtual display: ./run_gui_virtual.sh"
    echo "  2. Exit snap environment: exit (then run ./run_gui.sh)"
    echo "  3. Use xvfb manually: xvfb-run -a ./run_gui.sh"
    else
        echo "QML appears compatible. Running in GUI mode..."
    fi
fi

./openvim "$@"
