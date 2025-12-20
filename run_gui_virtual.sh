#!/bin/bash
# Script to run openvim GUI with virtual display
# Creates a virtual X11 display to show the GUI

cd build

echo "openvim - agentic development platform (VIRTUAL DISPLAY)"
echo "======================================================="

# Check if xvfb-run is available
if ! command -v xvfb-run &> /dev/null; then
    echo "Error: xvfb-run not found. Install with: sudo apt install xvfb"
    exit 1
fi

echo "Starting virtual display and running GUI..."
xvfb-run -a -s "-screen 0 1400x900x24" ./openvim "$@"