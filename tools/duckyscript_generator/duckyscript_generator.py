#!/usr/bin/env python3
"""
DuckyScript v3 Visual Script Generator
A block-based drag & drop editor for creating DuckyScript v3 scripts.
"""

import customtkinter as ctk
from tkinter import filedialog, messagebox
import json
import os
from typing import Optional, List, Dict, Any
from dataclasses import dataclass, field
import uuid

# Set appearance
ctk.set_appearance_mode("dark")
ctk.set_default_color_theme("blue")

# Block color scheme
BLOCK_COLORS = {
    "basic": "#4A90D9",      # Blue - basic commands
    "string": "#50C878",     # Green - string/text
    "delay": "#FFB347",      # Orange - timing
    "key": "#9B59B6",        # Purple - key combos
    "variable": "#E74C3C",   # Red - variables
    "loop": "#F39C12",       # Yellow/Gold - loops
    "condition": "#1ABC9C",  # Teal - conditionals
    "control": "#E91E63",    # Pink - break/continue
    "comment": "#95A5A6",    # Gray - comments
    "config": "#8E44AD",     # Dark purple - HID config
}


@dataclass
class Block:
    """Represents a single script block."""
    id: str
    block_type: str
    category: str
    params: Dict[str, Any] = field(default_factory=dict)
    children: List['Block'] = field(default_factory=list)
    else_children: List['Block'] = field(default_factory=list)  # For IF/ELSE

    def to_dict(self) -> dict:
        return {
            "id": self.id,
            "block_type": self.block_type,
            "category": self.category,
            "params": self.params,
            "children": [c.to_dict() for c in self.children],
            "else_children": [c.to_dict() for c in self.else_children],
        }

    @classmethod
    def from_dict(cls, data: dict) -> 'Block':
        block = cls(
            id=data["id"],
            block_type=data["block_type"],
            category=data["category"],
            params=data.get("params", {}),
        )
        block.children = [cls.from_dict(c) for c in data.get("children", [])]
        block.else_children = [cls.from_dict(c) for c in data.get("else_children", [])]
        return block


# Block definitions
BLOCK_DEFINITIONS = {
    # Basic commands
    "STRING": {"category": "string", "params": {"text": ""}, "label": "STRING", "container": False},
    "STRINGLN": {"category": "string", "params": {"text": ""}, "label": "STRINGLN", "container": False},
    "DELAY": {"category": "delay", "params": {"ms": 1000}, "label": "DELAY", "container": False},
    "ENTER": {"category": "basic", "params": {}, "label": "ENTER", "container": False},
    "TAB": {"category": "basic", "params": {}, "label": "TAB", "container": False},
    "SPACE": {"category": "basic", "params": {}, "label": "SPACE", "container": False},
    "BACKSPACE": {"category": "basic", "params": {}, "label": "BACKSPACE", "container": False},
    "DELETE": {"category": "basic", "params": {}, "label": "DELETE", "container": False},
    "ESCAPE": {"category": "basic", "params": {}, "label": "ESCAPE", "container": False},
    "HOME": {"category": "basic", "params": {}, "label": "HOME", "container": False},
    "END": {"category": "basic", "params": {}, "label": "END", "container": False},
    "INSERT": {"category": "basic", "params": {}, "label": "INSERT", "container": False},
    "PAGEUP": {"category": "basic", "params": {}, "label": "PAGEUP", "container": False},
    "PAGEDOWN": {"category": "basic", "params": {}, "label": "PAGEDOWN", "container": False},
    "UPARROW": {"category": "basic", "params": {}, "label": "UP", "container": False},
    "DOWNARROW": {"category": "basic", "params": {}, "label": "DOWN", "container": False},
    "LEFTARROW": {"category": "basic", "params": {}, "label": "LEFT", "container": False},
    "RIGHTARROW": {"category": "basic", "params": {}, "label": "RIGHT", "container": False},
    "CAPSLOCK": {"category": "basic", "params": {}, "label": "CAPSLOCK", "container": False},
    "NUMLOCK": {"category": "basic", "params": {}, "label": "NUMLOCK", "container": False},
    "SCROLLLOCK": {"category": "basic", "params": {}, "label": "SCROLLLOCK", "container": False},
    "PRINTSCREEN": {"category": "basic", "params": {}, "label": "PRINTSCREEN", "container": False},
    "PAUSE": {"category": "basic", "params": {}, "label": "PAUSE", "container": False},
    "MENU": {"category": "basic", "params": {}, "label": "MENU/APP", "container": False},

    # Function keys
    "F1": {"category": "basic", "params": {}, "label": "F1", "container": False},
    "F2": {"category": "basic", "params": {}, "label": "F2", "container": False},
    "F3": {"category": "basic", "params": {}, "label": "F3", "container": False},
    "F4": {"category": "basic", "params": {}, "label": "F4", "container": False},
    "F5": {"category": "basic", "params": {}, "label": "F5", "container": False},
    "F6": {"category": "basic", "params": {}, "label": "F6", "container": False},
    "F7": {"category": "basic", "params": {}, "label": "F7", "container": False},
    "F8": {"category": "basic", "params": {}, "label": "F8", "container": False},
    "F9": {"category": "basic", "params": {}, "label": "F9", "container": False},
    "F10": {"category": "basic", "params": {}, "label": "F10", "container": False},
    "F11": {"category": "basic", "params": {}, "label": "F11", "container": False},
    "F12": {"category": "basic", "params": {}, "label": "F12", "container": False},

    # Key combinations
    "CTRL": {"category": "key", "params": {"key": ""}, "label": "CTRL +", "container": False},
    "ALT": {"category": "key", "params": {"key": ""}, "label": "ALT +", "container": False},
    "SHIFT": {"category": "key", "params": {"key": ""}, "label": "SHIFT +", "container": False},
    "GUI": {"category": "key", "params": {"key": ""}, "label": "GUI/WIN +", "container": False},
    "CTRL_ALT": {"category": "key", "params": {"key": ""}, "label": "CTRL+ALT +", "container": False},
    "CTRL_SHIFT": {"category": "key", "params": {"key": ""}, "label": "CTRL+SHIFT +", "container": False},
    "ALT_SHIFT": {"category": "key", "params": {"key": ""}, "label": "ALT+SHIFT +", "container": False},
    "GUI_SHIFT": {"category": "key", "params": {"key": ""}, "label": "GUI+SHIFT +", "container": False},

    # Variables
    "VAR": {"category": "variable", "params": {"name": "myvar", "value": "0"}, "label": "VAR $", "container": False},
    "VAR_EXPR": {"category": "variable", "params": {"name": "myvar", "expr": "$var + 1"}, "label": "VAR $ = (expr)", "container": False},

    # Loops
    "LOOP": {"category": "loop", "params": {"count": 10}, "label": "LOOP", "container": True},
    "WHILE": {"category": "loop", "params": {"condition": "$i < 10"}, "label": "WHILE", "container": True},

    # Conditionals
    "IF": {"category": "condition", "params": {"condition": "$x == 1"}, "label": "IF", "container": True, "has_else": True},

    # Control
    "BREAK": {"category": "control", "params": {}, "label": "BREAK", "container": False},
    "CONTINUE": {"category": "control", "params": {}, "label": "CONTINUE", "container": False},

    # Comments
    "REM": {"category": "comment", "params": {"text": ""}, "label": "REM", "container": False},

    # HID Config
    "ID": {"category": "config", "params": {"vid": "1234", "pid": "5678"}, "label": "USB ID", "container": False},
    "MFR_NAME": {"category": "config", "params": {"name": ""}, "label": "USB Manufacturer", "container": False},
    "PROD_NAME": {"category": "config", "params": {"name": ""}, "label": "USB Product", "container": False},
    "BLE_NAME": {"category": "config", "params": {"name": ""}, "label": "BLE Name", "container": False},
    "BLE_MAC": {"category": "config", "params": {"mac": ""}, "label": "BLE MAC", "container": False},

    # Special
    "WAIT_FOR_BUTTON_PRESS": {"category": "control", "params": {}, "label": "WAIT FOR BUTTON", "container": False},
    "DEFAULT_DELAY": {"category": "delay", "params": {"ms": 100}, "label": "DEFAULT DELAY", "container": False},
    "REPEAT": {"category": "basic", "params": {"count": 2}, "label": "REPEAT", "container": False},
}


class BlockWidget(ctk.CTkFrame):
    """Visual representation of a block in the workspace."""

    def __init__(self, parent, block: Block, app: 'DuckyScriptGenerator', nested_level: int = 0):
        self.block = block
        self.app = app
        self.nested_level = nested_level
        self.definition = BLOCK_DEFINITIONS.get(block.block_type, {})
        self.color = BLOCK_COLORS.get(self.definition.get("category", "basic"), "#4A90D9")

        super().__init__(parent, fg_color=self.color, corner_radius=8)

        self.child_widgets: List[BlockWidget] = []
        self.else_child_widgets: List[BlockWidget] = []

        self._create_ui()

        # Bind drag events
        self.bind("<Button-1>", self._on_drag_start)
        self.bind("<B1-Motion>", self._on_drag_motion)
        self.bind("<ButtonRelease-1>", self._on_drag_end)

    def _create_ui(self):
        """Create the block's UI elements."""
        # Header frame
        header = ctk.CTkFrame(self, fg_color="transparent")
        header.pack(fill="x", padx=5, pady=3)

        # Block label
        label_text = self.definition.get("label", self.block.block_type)
        label = ctk.CTkLabel(header, text=label_text, font=("Arial", 12, "bold"),
                            text_color="white")
        label.pack(side="left")

        # Delete button
        delete_btn = ctk.CTkButton(header, text="×", width=20, height=20,
                                   fg_color="transparent", hover_color="#c0392b",
                                   command=self._delete_block)
        delete_btn.pack(side="right")

        # Parameter inputs
        params_frame = ctk.CTkFrame(self, fg_color="transparent")
        params_frame.pack(fill="x", padx=5, pady=2)

        self.param_entries = {}
        for param_name, default_value in self.definition.get("params", {}).items():
            param_frame = ctk.CTkFrame(params_frame, fg_color="transparent")
            param_frame.pack(fill="x", pady=1)

            # Get current value or default
            current_value = self.block.params.get(param_name, default_value)

            if param_name in ["text", "expr", "condition"]:
                # Larger text entry
                entry = ctk.CTkEntry(param_frame, width=200,
                                    placeholder_text=param_name)
                entry.insert(0, str(current_value))
            else:
                # Smaller entry with label
                lbl = ctk.CTkLabel(param_frame, text=f"{param_name}:", width=60,
                                  font=("Arial", 10))
                lbl.pack(side="left")
                entry = ctk.CTkEntry(param_frame, width=100,
                                    placeholder_text=param_name)
                entry.insert(0, str(current_value))

            entry.pack(side="left", fill="x", expand=True)
            entry.bind("<KeyRelease>", lambda e, p=param_name: self._on_param_change(p))
            self.param_entries[param_name] = entry

        # Container for child blocks (for loops/conditionals)
        if self.definition.get("container", False):
            # Children container
            self.children_frame = ctk.CTkFrame(self, fg_color=self._darken_color(self.color),
                                               corner_radius=5)
            self.children_frame.pack(fill="x", padx=10, pady=5)

            children_label = ctk.CTkLabel(self.children_frame,
                                         text="↓ Drop blocks here ↓",
                                         font=("Arial", 9), text_color="#aaa")
            children_label.pack(pady=10)

            # Bind drop events to children frame
            self.children_frame.bind("<Button-1>", lambda e: self._on_children_click())

            # ELSE container for IF blocks
            if self.definition.get("has_else", False):
                else_header = ctk.CTkLabel(self, text="ELSE", font=("Arial", 11, "bold"),
                                          text_color="white")
                else_header.pack(pady=(5, 0))

                self.else_frame = ctk.CTkFrame(self, fg_color=self._darken_color(self.color),
                                               corner_radius=5)
                self.else_frame.pack(fill="x", padx=10, pady=5)

                else_label = ctk.CTkLabel(self.else_frame,
                                         text="↓ Drop blocks here (optional) ↓",
                                         font=("Arial", 9), text_color="#aaa")
                else_label.pack(pady=10)

            # End label
            end_label = ctk.CTkLabel(self, text=f"END_{self.block.block_type}",
                                    font=("Arial", 10), text_color="#ddd")
            end_label.pack(pady=(0, 3))

        self._refresh_children()

    def _darken_color(self, hex_color: str) -> str:
        """Darken a hex color."""
        r = int(hex_color[1:3], 16)
        g = int(hex_color[3:5], 16)
        b = int(hex_color[5:7], 16)
        factor = 0.7
        r = int(r * factor)
        g = int(g * factor)
        b = int(b * factor)
        return f"#{r:02x}{g:02x}{b:02x}"

    def _refresh_children(self):
        """Refresh child block widgets."""
        if not self.definition.get("container", False):
            return

        # Clear existing child widgets
        for widget in self.child_widgets:
            widget.destroy()
        self.child_widgets.clear()

        # Recreate child widgets
        for child in self.block.children:
            widget = BlockWidget(self.children_frame, child, self.app, self.nested_level + 1)
            widget.pack(fill="x", padx=5, pady=2)
            self.child_widgets.append(widget)

        # Handle else children
        if self.definition.get("has_else", False):
            for widget in self.else_child_widgets:
                widget.destroy()
            self.else_child_widgets.clear()

            for child in self.block.else_children:
                widget = BlockWidget(self.else_frame, child, self.app, self.nested_level + 1)
                widget.pack(fill="x", padx=5, pady=2)
                self.else_child_widgets.append(widget)

    def _on_param_change(self, param_name: str):
        """Handle parameter value change."""
        entry = self.param_entries.get(param_name)
        if entry:
            self.block.params[param_name] = entry.get()
            self.app.update_code_preview()

    def _on_drag_start(self, event):
        """Start dragging the block."""
        self.app.drag_data = {
            "widget": self,
            "block": self.block,
            "start_x": event.x,
            "start_y": event.y,
        }

    def _on_drag_motion(self, event):
        """Handle drag motion."""
        pass  # Visual feedback could be added here

    def _on_drag_end(self, event):
        """End drag operation."""
        self.app.drag_data = None

    def _on_children_click(self):
        """Handle click on children container - for dropping blocks."""
        if self.app.selected_block_type:
            self.app.add_block_to_container(self.block, self.block.children)

    def _delete_block(self):
        """Delete this block."""
        self.app.delete_block(self.block)


class BlockPalette(ctk.CTkScrollableFrame):
    """Palette of available blocks to drag into workspace."""

    def __init__(self, parent, app: 'DuckyScriptGenerator'):
        super().__init__(parent, width=220, label_text="Block Palette")
        self.app = app
        self._create_categories()

    def _create_categories(self):
        """Create categorized block buttons."""
        categories = {
            "Text Output": ["STRING", "STRINGLN"],
            "Timing": ["DELAY", "DEFAULT_DELAY"],
            "Basic Keys": ["ENTER", "TAB", "SPACE", "BACKSPACE", "DELETE", "ESCAPE"],
            "Navigation": ["HOME", "END", "INSERT", "PAGEUP", "PAGEDOWN",
                          "UPARROW", "DOWNARROW", "LEFTARROW", "RIGHTARROW"],
            "Function Keys": ["F1", "F2", "F3", "F4", "F5", "F6",
                             "F7", "F8", "F9", "F10", "F11", "F12"],
            "Key Combos": ["CTRL", "ALT", "SHIFT", "GUI",
                          "CTRL_ALT", "CTRL_SHIFT", "ALT_SHIFT", "GUI_SHIFT"],
            "Variables": ["VAR", "VAR_EXPR"],
            "Loops": ["LOOP", "WHILE"],
            "Conditionals": ["IF"],
            "Control Flow": ["BREAK", "CONTINUE", "WAIT_FOR_BUTTON_PRESS"],
            "Comments": ["REM"],
            "HID Config": ["ID", "MFR_NAME", "PROD_NAME", "BLE_NAME", "BLE_MAC"],
            "Other": ["REPEAT", "CAPSLOCK", "NUMLOCK", "PRINTSCREEN", "PAUSE", "MENU"],
        }

        for category_name, block_types in categories.items():
            # Category header
            header = ctk.CTkLabel(self, text=category_name,
                                 font=("Arial", 11, "bold"),
                                 text_color="#888")
            header.pack(fill="x", pady=(10, 5))

            # Block buttons in a grid
            btn_frame = ctk.CTkFrame(self, fg_color="transparent")
            btn_frame.pack(fill="x")

            col = 0
            row_frame = None
            for block_type in block_types:
                if col == 0:
                    row_frame = ctk.CTkFrame(btn_frame, fg_color="transparent")
                    row_frame.pack(fill="x", pady=1)

                definition = BLOCK_DEFINITIONS.get(block_type, {})
                color = BLOCK_COLORS.get(definition.get("category", "basic"), "#4A90D9")
                label = definition.get("label", block_type)

                btn = ctk.CTkButton(
                    row_frame,
                    text=label[:12],  # Truncate long labels
                    width=95,
                    height=28,
                    fg_color=color,
                    hover_color=self._lighten_color(color),
                    font=("Arial", 10),
                    command=lambda bt=block_type: self.app.select_block_type(bt)
                )
                btn.pack(side="left", padx=2, pady=1)

                col = (col + 1) % 2

    def _lighten_color(self, hex_color: str) -> str:
        """Lighten a hex color."""
        r = int(hex_color[1:3], 16)
        g = int(hex_color[3:5], 16)
        b = int(hex_color[5:7], 16)
        factor = 1.2
        r = min(255, int(r * factor))
        g = min(255, int(g * factor))
        b = min(255, int(b * factor))
        return f"#{r:02x}{g:02x}{b:02x}"


class Workspace(ctk.CTkScrollableFrame):
    """Main workspace where blocks are arranged."""

    def __init__(self, parent, app: 'DuckyScriptGenerator'):
        super().__init__(parent, label_text="Script Workspace")
        self.app = app
        self.block_widgets: List[BlockWidget] = []

        # Instructions label
        self.instructions = ctk.CTkLabel(
            self,
            text="Click a block in the palette, then click here to add it.\n"
                 "Or click 'Add to Script' after selecting a block.",
            font=("Arial", 11),
            text_color="#888"
        )
        self.instructions.pack(pady=20)

        # Bind click to add blocks
        self.bind("<Button-1>", self._on_click)

    def _on_click(self, event):
        """Handle click on workspace."""
        if self.app.selected_block_type:
            self.app.add_block_to_workspace()

    def refresh(self):
        """Refresh all block widgets."""
        # Clear existing widgets
        for widget in self.block_widgets:
            widget.destroy()
        self.block_widgets.clear()

        # Hide instructions if we have blocks
        if self.app.blocks:
            self.instructions.pack_forget()
        else:
            self.instructions.pack(pady=20)

        # Create new widgets
        for block in self.app.blocks:
            widget = BlockWidget(self, block, self.app)
            widget.pack(fill="x", padx=5, pady=3)
            self.block_widgets.append(widget)


class CodePreview(ctk.CTkFrame):
    """Shows the generated DuckyScript code."""

    def __init__(self, parent, app: 'DuckyScriptGenerator'):
        super().__init__(parent)
        self.app = app

        # Header
        header = ctk.CTkFrame(self, fg_color="transparent")
        header.pack(fill="x", padx=5, pady=5)

        label = ctk.CTkLabel(header, text="Generated Script",
                            font=("Arial", 12, "bold"))
        label.pack(side="left")

        copy_btn = ctk.CTkButton(header, text="Copy", width=60,
                                command=self._copy_code)
        copy_btn.pack(side="right", padx=5)

        # Text area
        self.text = ctk.CTkTextbox(self, font=("Consolas", 11), wrap="none")
        self.text.pack(fill="both", expand=True, padx=5, pady=5)

    def update_code(self, code: str):
        """Update the displayed code."""
        self.text.delete("1.0", "end")
        self.text.insert("1.0", code)

    def _copy_code(self):
        """Copy code to clipboard."""
        code = self.text.get("1.0", "end").strip()
        self.clipboard_clear()
        self.clipboard_append(code)
        messagebox.showinfo("Copied", "Script copied to clipboard!")


class DuckyScriptGenerator(ctk.CTk):
    """Main application window."""

    def __init__(self):
        super().__init__()

        self.title("DuckyScript v3 Generator")
        self.geometry("1200x800")

        self.blocks: List[Block] = []
        self.selected_block_type: Optional[str] = None
        self.drag_data: Optional[dict] = None
        self.current_file: Optional[str] = None

        self._create_menu()
        self._create_ui()
        self.update_code_preview()

    def _create_menu(self):
        """Create the menu bar."""
        # Using CTk buttons as menu since CTk doesn't have native menu
        menu_frame = ctk.CTkFrame(self, height=40)
        menu_frame.pack(fill="x", padx=5, pady=5)
        menu_frame.pack_propagate(False)

        ctk.CTkButton(menu_frame, text="New", width=70,
                     command=self.new_script).pack(side="left", padx=2)
        ctk.CTkButton(menu_frame, text="Open", width=70,
                     command=self.open_script).pack(side="left", padx=2)
        ctk.CTkButton(menu_frame, text="Save", width=70,
                     command=self.save_script).pack(side="left", padx=2)
        ctk.CTkButton(menu_frame, text="Export .txt", width=90,
                     command=self.export_script).pack(side="left", padx=2)

        ctk.CTkLabel(menu_frame, text="│", text_color="#555").pack(side="left", padx=5)

        # Selection indicator
        self.selection_label = ctk.CTkLabel(menu_frame, text="Select a block →",
                                           font=("Arial", 11))
        self.selection_label.pack(side="left", padx=10)

        self.add_btn = ctk.CTkButton(menu_frame, text="Add to Script", width=100,
                                    state="disabled", command=self.add_block_to_workspace)
        self.add_btn.pack(side="left", padx=5)

        ctk.CTkButton(menu_frame, text="Clear All", width=80,
                     fg_color="#c0392b", hover_color="#e74c3c",
                     command=self.clear_all).pack(side="right", padx=5)

    def _create_ui(self):
        """Create the main UI layout."""
        # Main container
        main_frame = ctk.CTkFrame(self, fg_color="transparent")
        main_frame.pack(fill="both", expand=True, padx=5, pady=5)

        # Left: Block palette
        self.palette = BlockPalette(main_frame, self)
        self.palette.pack(side="left", fill="y", padx=(0, 5))

        # Middle: Workspace
        workspace_frame = ctk.CTkFrame(main_frame)
        workspace_frame.pack(side="left", fill="both", expand=True, padx=5)

        self.workspace = Workspace(workspace_frame, self)
        self.workspace.pack(fill="both", expand=True)

        # Right: Code preview
        preview_frame = ctk.CTkFrame(main_frame, width=350)
        preview_frame.pack(side="right", fill="y", padx=(5, 0))
        preview_frame.pack_propagate(False)

        self.code_preview = CodePreview(preview_frame, self)
        self.code_preview.pack(fill="both", expand=True)

    def select_block_type(self, block_type: str):
        """Select a block type to add."""
        self.selected_block_type = block_type
        definition = BLOCK_DEFINITIONS.get(block_type, {})
        label = definition.get("label", block_type)
        self.selection_label.configure(text=f"Selected: {label}")
        self.add_btn.configure(state="normal")

    def add_block_to_workspace(self):
        """Add the selected block type to the workspace."""
        if not self.selected_block_type:
            return

        definition = BLOCK_DEFINITIONS.get(self.selected_block_type, {})
        block = Block(
            id=str(uuid.uuid4()),
            block_type=self.selected_block_type,
            category=definition.get("category", "basic"),
            params=dict(definition.get("params", {})),
        )

        self.blocks.append(block)
        self.workspace.refresh()
        self.update_code_preview()

        # Clear selection
        self.selected_block_type = None
        self.selection_label.configure(text="Select a block →")
        self.add_btn.configure(state="disabled")

    def add_block_to_container(self, parent_block: Block, children_list: List[Block]):
        """Add the selected block to a container block."""
        if not self.selected_block_type:
            return

        definition = BLOCK_DEFINITIONS.get(self.selected_block_type, {})
        block = Block(
            id=str(uuid.uuid4()),
            block_type=self.selected_block_type,
            category=definition.get("category", "basic"),
            params=dict(definition.get("params", {})),
        )

        children_list.append(block)
        self.workspace.refresh()
        self.update_code_preview()

        # Clear selection
        self.selected_block_type = None
        self.selection_label.configure(text="Select a block →")
        self.add_btn.configure(state="disabled")

    def delete_block(self, block: Block):
        """Delete a block from the script."""
        # Remove from main list
        if block in self.blocks:
            self.blocks.remove(block)
        else:
            # Search in nested blocks
            self._remove_from_nested(self.blocks, block)

        self.workspace.refresh()
        self.update_code_preview()

    def _remove_from_nested(self, block_list: List[Block], target: Block) -> bool:
        """Recursively remove a block from nested structures."""
        for block in block_list:
            if target in block.children:
                block.children.remove(target)
                return True
            if target in block.else_children:
                block.else_children.remove(target)
                return True
            if self._remove_from_nested(block.children, target):
                return True
            if self._remove_from_nested(block.else_children, target):
                return True
        return False

    def update_code_preview(self):
        """Generate and display the script code."""
        code = self._generate_code(self.blocks, 0)
        self.code_preview.update_code(code)

    def _generate_code(self, blocks: List[Block], indent: int) -> str:
        """Generate DuckyScript code from blocks."""
        lines = []
        prefix = "    " * indent

        for block in blocks:
            bt = block.block_type
            params = block.params

            if bt == "STRING":
                lines.append(f"{prefix}STRING {params.get('text', '')}")
            elif bt == "STRINGLN":
                lines.append(f"{prefix}STRINGLN {params.get('text', '')}")
            elif bt == "DELAY":
                lines.append(f"{prefix}DELAY {params.get('ms', 1000)}")
            elif bt == "DEFAULT_DELAY":
                lines.append(f"{prefix}DEFAULT_DELAY {params.get('ms', 100)}")
            elif bt == "REM":
                lines.append(f"{prefix}REM {params.get('text', '')}")
            elif bt == "REPEAT":
                lines.append(f"{prefix}REPEAT {params.get('count', 2)}")
            elif bt in ["CTRL", "ALT", "SHIFT", "GUI"]:
                key = params.get('key', '')
                if key:
                    lines.append(f"{prefix}{bt} {key}")
                else:
                    lines.append(f"{prefix}{bt}")
            elif bt == "CTRL_ALT":
                key = params.get('key', '')
                if key:
                    lines.append(f"{prefix}CTRL ALT {key}")
                else:
                    lines.append(f"{prefix}CTRL ALT")
            elif bt == "CTRL_SHIFT":
                key = params.get('key', '')
                if key:
                    lines.append(f"{prefix}CTRL SHIFT {key}")
                else:
                    lines.append(f"{prefix}CTRL SHIFT")
            elif bt == "ALT_SHIFT":
                key = params.get('key', '')
                if key:
                    lines.append(f"{prefix}ALT SHIFT {key}")
                else:
                    lines.append(f"{prefix}ALT SHIFT")
            elif bt == "GUI_SHIFT":
                key = params.get('key', '')
                if key:
                    lines.append(f"{prefix}GUI SHIFT {key}")
                else:
                    lines.append(f"{prefix}GUI SHIFT")
            elif bt == "VAR":
                name = params.get('name', 'x')
                value = params.get('value', '0')
                # Check if value is a string (needs quotes) or number
                try:
                    int(value)
                    lines.append(f"{prefix}VAR ${name} = {value}")
                except ValueError:
                    if value.startswith('"') or value.startswith('$'):
                        lines.append(f"{prefix}VAR ${name} = {value}")
                    else:
                        lines.append(f'{prefix}VAR ${name} = "{value}"')
            elif bt == "VAR_EXPR":
                name = params.get('name', 'x')
                expr = params.get('expr', '$x + 1')
                lines.append(f"{prefix}VAR ${name} = ({expr})")
            elif bt == "LOOP":
                count = params.get('count', 10)
                lines.append(f"{prefix}LOOP {count}")
                lines.append(self._generate_code(block.children, indent + 1))
                lines.append(f"{prefix}END_LOOP")
            elif bt == "WHILE":
                cond = params.get('condition', '$i < 10')
                lines.append(f"{prefix}WHILE ({cond})")
                lines.append(self._generate_code(block.children, indent + 1))
                lines.append(f"{prefix}END_WHILE")
            elif bt == "IF":
                cond = params.get('condition', '$x == 1')
                lines.append(f"{prefix}IF ({cond})")
                lines.append(self._generate_code(block.children, indent + 1))
                if block.else_children:
                    lines.append(f"{prefix}ELSE")
                    lines.append(self._generate_code(block.else_children, indent + 1))
                lines.append(f"{prefix}END_IF")
            elif bt == "ID":
                vid = params.get('vid', '1234')
                pid = params.get('pid', '5678')
                lines.append(f"{prefix}ID {vid}:{pid}")
            elif bt == "MFR_NAME":
                lines.append(f"{prefix}MFR_NAME {params.get('name', '')}")
            elif bt == "PROD_NAME":
                lines.append(f"{prefix}PROD_NAME {params.get('name', '')}")
            elif bt == "BLE_NAME":
                lines.append(f"{prefix}BLE_NAME {params.get('name', '')}")
            elif bt == "BLE_MAC":
                lines.append(f"{prefix}BLE_MAC {params.get('mac', '')}")
            elif bt in BLOCK_DEFINITIONS:
                # Simple command with no params
                if bt == "MENU":
                    lines.append(f"{prefix}APP")
                else:
                    lines.append(f"{prefix}{bt}")

        return "\n".join(line for line in lines if line.strip())

    def new_script(self):
        """Create a new empty script."""
        if self.blocks:
            if not messagebox.askyesno("New Script",
                                       "Clear current script and start new?"):
                return

        self.blocks.clear()
        self.current_file = None
        self.workspace.refresh()
        self.update_code_preview()

    def clear_all(self):
        """Clear all blocks."""
        if self.blocks:
            if not messagebox.askyesno("Clear All",
                                       "Remove all blocks from workspace?"):
                return

        self.blocks.clear()
        self.workspace.refresh()
        self.update_code_preview()

    def open_script(self):
        """Open a saved script project."""
        filepath = filedialog.askopenfilename(
            title="Open Script Project",
            filetypes=[("DuckyScript Project", "*.ducky.json"), ("All Files", "*.*")]
        )
        if not filepath:
            return

        try:
            with open(filepath, 'r') as f:
                data = json.load(f)

            self.blocks = [Block.from_dict(b) for b in data.get("blocks", [])]
            self.current_file = filepath
            self.workspace.refresh()
            self.update_code_preview()
        except Exception as e:
            messagebox.showerror("Error", f"Failed to open file:\n{e}")

    def save_script(self):
        """Save the script project."""
        if not self.current_file:
            filepath = filedialog.asksaveasfilename(
                title="Save Script Project",
                defaultextension=".ducky.json",
                filetypes=[("DuckyScript Project", "*.ducky.json")]
            )
            if not filepath:
                return
            self.current_file = filepath

        try:
            data = {
                "version": "1.0",
                "blocks": [b.to_dict() for b in self.blocks]
            }
            with open(self.current_file, 'w') as f:
                json.dump(data, f, indent=2)

            messagebox.showinfo("Saved", f"Project saved to:\n{self.current_file}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to save file:\n{e}")

    def export_script(self):
        """Export as .txt DuckyScript file."""
        filepath = filedialog.asksaveasfilename(
            title="Export DuckyScript",
            defaultextension=".txt",
            filetypes=[("DuckyScript", "*.txt"), ("All Files", "*.*")]
        )
        if not filepath:
            return

        try:
            code = self._generate_code(self.blocks, 0)
            with open(filepath, 'w') as f:
                f.write(code)

            messagebox.showinfo("Exported", f"Script exported to:\n{filepath}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to export file:\n{e}")


def main():
    app = DuckyScriptGenerator()
    app.mainloop()


if __name__ == "__main__":
    main()
