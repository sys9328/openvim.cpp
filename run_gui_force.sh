#!/bin/bash
# Script to run openvim GUI with minimal Qt platform
# This avoids X11/Wayland dependencies that cause library conflicts

cd build

echo "openvim - agentic development platform (MINIMAL QT PLATFORM)"
echo "============================================================"

echo "Using minimal Qt platform (no GUI display, but no crashes)..."
export QT_QPA_PLATFORM=minimal
./openvim "$@"