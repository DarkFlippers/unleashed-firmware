# DuckyScript v3 Visual Generator

A block-based drag & drop editor for creating DuckyScript v3 scripts for Flipper Zero.

![Screenshot placeholder]

## Features

- **Visual Block Editor**: Drag and drop blocks to build scripts
- **Full DuckyScript v3 Support**: Variables, loops, conditionals, all commands
- **Real-time Preview**: See generated code as you build
- **Project Save/Load**: Save your work as JSON projects
- **Export to .txt**: Export ready-to-use DuckyScript files

## Installation

```bash
# Install dependencies
pip install customtkinter

# Or use requirements.txt
pip install -r requirements.txt
```

## Usage

```bash
cd tools/duckyscript_generator
python duckyscript_generator.py
```

### Building a Script

1. **Select a block** from the left palette by clicking on it
2. **Click "Add to Script"** or click in the workspace to add the block
3. **Configure parameters** in the block's input fields
4. **For loops/conditionals**: Click inside the container to add nested blocks
5. **View the generated code** in the right panel

### Block Categories

| Category | Color | Description |
|----------|-------|-------------|
| Text Output | Green | STRING, STRINGLN |
| Timing | Orange | DELAY, DEFAULT_DELAY |
| Basic Keys | Blue | ENTER, TAB, ESCAPE, arrows, etc. |
| Key Combos | Purple | CTRL+, ALT+, GUI+, SHIFT+ |
| Variables | Red | VAR definitions and expressions |
| Loops | Gold | LOOP, WHILE containers |
| Conditionals | Teal | IF/ELSE containers |
| Control Flow | Pink | BREAK, CONTINUE |
| Comments | Gray | REM |
| HID Config | Dark Purple | USB/BLE settings |

### File Types

- **`.ducky.json`**: Project files (save/open) - preserves block structure
- **`.txt`**: Export files - ready-to-use DuckyScript for Flipper

## Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl+N | New script |
| Ctrl+O | Open project |
| Ctrl+S | Save project |
| Ctrl+E | Export .txt |

## Example Workflow

1. Start with REM block for a comment describing your script
2. Add DELAY at the start (gives time to focus the right window)
3. Build your logic with STRING, key presses, and control flow
4. Test generated code in the preview panel
5. Export to .txt and copy to Flipper's `/ext/badusb/` folder

## Supported Commands

### Basic Commands
- STRING, STRINGLN, DELAY, DEFAULT_DELAY
- ENTER, TAB, SPACE, BACKSPACE, DELETE, ESCAPE
- Arrow keys, HOME, END, INSERT, PAGEUP, PAGEDOWN
- F1-F12, CAPSLOCK, NUMLOCK, SCROLLLOCK
- PRINTSCREEN, PAUSE, MENU/APP

### Key Combinations
- CTRL + key
- ALT + key
- SHIFT + key
- GUI/WIN + key
- CTRL+ALT, CTRL+SHIFT, ALT+SHIFT, GUI+SHIFT combinations

### DuckyScript v3 Features
- **VAR**: Define variables (`VAR $name = value`)
- **Expressions**: `VAR $x = ($y + 1)`
- **LOOP/END_LOOP**: Repeat N times
- **WHILE/END_WHILE**: Loop with condition
- **IF/ELSE/END_IF**: Conditional execution
- **BREAK/CONTINUE**: Loop control
- **$_RANDOM, $_LINE**: Built-in variables

### HID Configuration
- ID (USB VID:PID)
- MFR_NAME (USB manufacturer)
- PROD_NAME (USB product)
- BLE_NAME (Bluetooth name)
- BLE_MAC (Bluetooth MAC address)

## Requirements

- Python 3.8+
- customtkinter 5.0+

## License

GPL-3.0 (same as Unleashed firmware)
