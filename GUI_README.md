# openvim - agentic development platform
# Qt GUI User Guide

## GUI Status: ✅ WORKING

The Qt GUI is fully implemented and functional! The application includes:
- **Code Editor**: Syntax-highlighting editor with dark theme
- **AI Chat**: Integrated chat interface with LLM services
- **File Browser**: Tree view for project navigation
- **Tool Output**: Tabbed panels for tool results
- **Permission Dialogs**: Modal dialogs for user confirmations

## Running the GUI

### Option 1: Smart Auto-Detection (Recommended)
```bash
./run_gui.sh
```
- Automatically detects display availability
- Uses minimal Qt platform to avoid library conflicts
- Provides clear instructions if GUI can't be displayed

### Option 2: Virtual Display (For Servers/Headless)
```bash
./run_gui_virtual.sh
```
- Creates virtual X11 display using Xvfb
- Shows GUI in memory (accessible via VNC if needed)
- Requires: `sudo apt install xvfb`

### Option 3: Minimal Platform (Safe Mode)
```bash
./run_gui_force.sh
```
- Uses Qt's minimal platform plugin
- No display dependencies, no crashes
- GUI components initialize but not visible

### Option 4: Native Display (Outside Snap)
```bash
exit  # Exit snap environment first
./run_gui.sh
```
- Run outside of snap for full GUI experience
- Requires native Linux environment

## Troubleshooting

### "symbol lookup error" with libpthread
**Cause**: Library conflicts in snap environments
**Solution**: Use `./run_gui.sh` (auto-detects and uses safe mode)

### "No display detected"
**Cause**: Running in headless environment
**Solution**: Use `./run_gui_virtual.sh` for virtual display

### GUI doesn't show
**Cause**: Using minimal/offscreen platform
**Solution**: Exit snap environment or use virtual display

## Technical Details

- **Qt Version**: 5.15+ (cross-platform GUI framework)
- **Platform Plugins**: xcb (X11), wayland, minimal, offscreen
- **Theme**: Neovim-inspired dark theme (#1e1e1e backgrounds)
- **Resolution**: 1400x900 default window size
- **Backend**: Fully integrated with existing services (database, LLM, sessions)

## Development

The GUI consists of:
- `src/gui/main_window.*` - Main application window
- `src/gui/code_editor.*` - Syntax highlighting editor
- `src/gui/chat_widget.*` - AI chat interface
- `src/gui/file_browser.*` - File tree view
- `src/gui/tool_output.*` - Tool output panels
- `src/gui/permission_dialog.*` - Permission requests

Build with: `mkdir build && cd build && cmake .. && make`