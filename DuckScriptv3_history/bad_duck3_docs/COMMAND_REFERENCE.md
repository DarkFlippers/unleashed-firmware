# Bad Duck3 - DuckyScript 3.0 Command Reference

Complete reference for all commands supported by Bad Duck3.

---

## Table of Contents

1. [Basic Commands](#basic-commands)
2. [Control Flow](#control-flow)
3. [Variables](#variables)
4. [Expressions](#expressions)
5. [Special Keys](#special-keys)
6. [Modifier Keys](#modifier-keys)
7. [Flipper Extensions](#flipper-extensions)
8. [Configuration Commands](#configuration-commands)
9. [Mouse Commands](#mouse-commands)

---

## Basic Commands

### REM
Comment line. Ignored by interpreter.
```duckyscript
REM This is a comment
REM Author: dutchpatriot
```

### STRING
Type a string of characters.
```duckyscript
STRING Hello, World!
STRING The quick brown fox jumps over the lazy dog.
```

### STRINGLN
Type a string followed by ENTER.
```duckyscript
STRINGLN echo "Hello"
REM Equivalent to:
REM STRING echo "Hello"
REM ENTER
```

### DELAY
Pause execution for specified milliseconds.
```duckyscript
DELAY 1000
REM Waits 1 second
```

### DEFAULT_DELAY / DEFAULTDELAY
Set delay between every command.
```duckyscript
DEFAULT_DELAY 100
REM 100ms pause after each command
```

### DEFAULT_STRING_DELAY / DEFAULTSTRINGDELAY
Set delay between each character in STRING commands.
```duckyscript
DEFAULT_STRING_DELAY 20
REM 20ms between each keystroke
```

---

## Control Flow

### LOOP / END_LOOP
Execute a block of commands N times.

**Syntax:**
```duckyscript
LOOP <count>
    <commands>
END_LOOP
```

**Examples:**
```duckyscript
REM Press ENTER 10 times
LOOP 10
    ENTER
    DELAY 100
END_LOOP

REM Using variable for count
VAR $times = 50
LOOP $times
    STRING .
END_LOOP
```

**Nesting:** Up to 8 levels deep
```duckyscript
LOOP 3
    LOOP 3
        STRING X
    END_LOOP
    ENTER
END_LOOP
REM Output:
REM XXX
REM XXX
REM XXX
```

### WHILE / END_WHILE
Loop while a condition is true.

**Syntax:**
```duckyscript
WHILE (<condition>)
    <commands>
END_WHILE
```

**Examples:**
```duckyscript
VAR $counter = 0
WHILE ($counter < 10)
    STRING Count: 
    ENTER
    VAR $counter = ($counter + 1)
END_WHILE

REM Infinite loop (careful!)
WHILE (1 == 1)
    STRING Forever...
    DELAY 1000
END_WHILE
```

### IF / ELSE / END_IF
Conditional execution.

**Syntax:**
```duckyscript
IF (<condition>)
    <commands if true>
END_IF

IF (<condition>)
    <commands if true>
ELSE
    <commands if false>
END_IF
```

**Examples:**
```duckyscript
VAR $mode = 1

IF ($mode == 1)
    STRING Mode is ONE
    ENTER
END_IF

VAR $value = 42
IF ($value > 50)
    STRING Greater than 50
ELSE
    STRING 50 or less
END_IF
ENTER
```

**Nesting:**
```duckyscript
VAR $a = 5
VAR $b = 10

IF ($a > 0)
    IF ($b > 5)
        STRING Both conditions met
    END_IF
END_IF
```

### BREAK
Exit the current loop immediately.

```duckyscript
VAR $i = 0
LOOP 1000
    VAR $i = ($i + 1)
    IF ($i == 10)
        BREAK
    END_IF
    STRING .
END_LOOP
REM Only prints 9 dots
```

### CONTINUE
Skip to the next loop iteration.

```duckyscript
VAR $i = 0
LOOP 10
    VAR $i = ($i + 1)
    IF ($i == 5)
        CONTINUE
    END_IF
    STRING X
END_LOOP
REM Prints 9 X's (skips iteration 5)
```

---

## Variables

### VAR
Declare or assign a variable.

**Syntax:**
```duckyscript
VAR $name = <value>
VAR $name = <expression>
VAR $name = "<string>"
```

**Rules:**
- Variable names start with `$`
- Names can contain letters, numbers, underscore
- Case sensitive (`$Var` ≠ `$var`)
- Maximum 32 variables
- Maximum name length: 16 characters

**Examples:**
```duckyscript
REM Integer assignment
VAR $count = 100
VAR $delay_ms = 500

REM Expression assignment
VAR $double = ($count * 2)
VAR $sum = ($a + $b)

REM String assignment
VAR $name = "Flipper"
VAR $msg = "Hello World"

REM Using built-in variables
VAR $random_wait = ($_RANDOM % 1000)
```

### Built-in Variables

| Variable | Type | Description |
|----------|------|-------------|
| `$_RANDOM` | Integer | Random number 0-65535 |
| `$_LOOP_COUNT` | Integer | Current iteration (1-indexed) |
| `$_LINE` | Integer | Current script line number |

**Examples:**
```duckyscript
REM Random delay between 100-600ms
VAR $wait = ($_RANDOM % 500)
VAR $wait = ($wait + 100)
DELAY $wait

REM Access loop counter
LOOP 5
    REM $_LOOP_COUNT is 1, 2, 3, 4, 5
END_LOOP
```

---

## Expressions

Expressions are enclosed in parentheses and support:

### Arithmetic Operators
| Operator | Description | Example |
|----------|-------------|---------|
| `+` | Addition | `($a + $b)` |
| `-` | Subtraction | `($a - 5)` |
| `*` | Multiplication | `($x * 2)` |
| `/` | Division | `($total / $count)` |
| `%` | Modulo | `($_RANDOM % 100)` |

### Comparison Operators
| Operator | Description | Example |
|----------|-------------|---------|
| `<` | Less than | `($x < 10)` |
| `>` | Greater than | `($x > 0)` |
| `<=` | Less or equal | `($i <= $max)` |
| `>=` | Greater or equal | `($count >= 100)` |
| `==` | Equal | `($mode == 1)` |
| `!=` | Not equal | `($status != 0)` |

### Logical Operators
| Operator | Description | Example |
|----------|-------------|---------|
| `&&` | Logical AND | `($a > 0 && $b > 0)` |
| `\|\|` | Logical OR | `($x == 1 \|\| $x == 2)` |

**Note:** Complex expressions may require multiple VAR statements:
```duckyscript
REM Instead of: IF (($a > 0) && ($b < 10))
VAR $cond1 = ($a > 0)
VAR $cond2 = ($b < 10)
VAR $both = ($cond1 && $cond2)
IF ($both == 1)
    STRING Both conditions true
END_IF
```

---

## Special Keys

### Navigation
| Command | Key |
|---------|-----|
| `ENTER` | Enter/Return |
| `SPACE` | Spacebar |
| `TAB` | Tab |
| `BACKSPACE` / `BKSP` | Backspace |
| `DELETE` / `DEL` | Delete |
| `INSERT` | Insert |
| `HOME` | Home |
| `END` | End |
| `PAGEUP` / `PGUP` | Page Up |
| `PAGEDOWN` / `PGDN` | Page Down |

### Arrows
| Command | Key |
|---------|-----|
| `UP` / `UPARROW` | ↑ |
| `DOWN` / `DOWNARROW` | ↓ |
| `LEFT` / `LEFTARROW` | ← |
| `RIGHT` / `RIGHTARROW` | → |

### Function Keys
| Command | Key |
|---------|-----|
| `F1` - `F12` | Function keys |
| `F13` - `F24` | Extended function keys |

### System Keys
| Command | Key |
|---------|-----|
| `ESC` / `ESCAPE` | Escape |
| `PRINTSCREEN` | Print Screen |
| `SCROLLLOCK` | Scroll Lock |
| `PAUSE` / `BREAK` | Pause/Break |
| `CAPSLOCK` | Caps Lock |
| `NUMLOCK` | Num Lock |
| `MENU` / `APP` | Context menu |

---

## Modifier Keys

Can be combined with other keys using space or hyphen.

### Modifiers
| Command | Key |
|---------|-----|
| `CTRL` / `CONTROL` | Control |
| `SHIFT` | Shift |
| `ALT` | Alt |
| `GUI` / `WINDOWS` / `COMMAND` | Win/Cmd |

### Combinations
```duckyscript
REM Single modifier + key
CTRL c
ALT F4
GUI r
SHIFT TAB

REM Multiple modifiers
CTRL-SHIFT ESC
CTRL-ALT DELETE
GUI-SHIFT s

REM With hyphen or space
CTRL ALT t
CTRL-ALT-SHIFT F12
```

### Common Shortcuts
```duckyscript
REM Windows Run dialog
GUI r

REM Windows Task Manager
CTRL-SHIFT ESC

REM macOS Spotlight
GUI SPACE

REM Linux terminal
CTRL-ALT t

REM Copy/Paste
CTRL c
CTRL v
```

---

## Flipper Extensions

Commands specific to Flipper Zero (not in standard DuckyScript).

### ALTSTRING / ALTCODE
Type characters using ALT+numpad codes. Useful for special characters and non-US layouts.

```duckyscript
ALTSTRING Héllo Wörld
ALTCODE Special: ñ é ü
```

### SYSRQ
Send Linux SysRq (Magic SysRq) commands.

```duckyscript
SYSRQ b
REM Immediate reboot

SYSRQ s
REM Sync filesystems
```

### DELAY_RANDOM
Random delay within a range.

```duckyscript
DELAY_RANDOM 100 500
REM Waits between 100-500ms
```

### REPEAT
Repeat the previous command N times.

```duckyscript
STRING Hello
REPEAT 5
REM Types "Hello" 5 more times (6 total)
```

### HOLD / RELEASE
Hold and release keys explicitly.

```duckyscript
HOLD SHIFT
STRING hello
RELEASE SHIFT
REM Types "HELLO"
```

---

## Configuration Commands

### ID
Set custom USB Vendor ID, Product ID, and names.
**Must be first line in script.**

```duckyscript
ID 1234:5678 Logitech:Keyboard K120
REM VID:PID Manufacturer:Product
```

### Common USB IDs
```duckyscript
REM Generic keyboard
ID 0000:0000 Keyboard:USB Keyboard

REM Apple keyboard
ID 05ac:024f Apple:Keyboard

REM Logitech
ID 046d:c31c Logitech:Keyboard K120
```

---

## Mouse Commands

Bad Duck3 supports mouse emulation.

| Command | Description |
|---------|-------------|
| `LEFTCLICK` / `CLICK` | Left mouse button |
| `RIGHTCLICK` | Right mouse button |
| `MIDDLECLICK` | Middle mouse button |
| `MOUSEMOVE x y` | Move mouse relative |
| `MOUSESCROLL n` | Scroll wheel (+ down, - up) |

**Examples:**
```duckyscript
REM Click and drag
LEFTCLICK
DELAY 100
MOUSEMOVE 100 50
LEFTCLICK

REM Right-click context menu
RIGHTCLICK
DELAY 200
DOWN
DOWN
ENTER

REM Scroll down
MOUSESCROLL 5
```

---

## Complete Example

```duckyscript
REM Bad Duck3 Demo Script
REM Author: dutchpatriot
REM Description: Demonstrates DS3.0 features

ID 046d:c31c Logitech:Keyboard

REM Wait for target to be ready
DELAY 3000

REM Variables
VAR $max_loops = 5
VAR $counter = 0

REM Open notepad (Windows)
GUI r
DELAY 500
STRING notepad
ENTER
DELAY 1000

REM Type with loop
WHILE ($counter < $max_loops)
    VAR $counter = ($counter + 1)
    
    STRING Line number: 
    ENTER
    
    REM Random delay for realism
    VAR $wait = ($_RANDOM % 300)
    VAR $wait = ($wait + 100)
    DELAY $wait
END_WHILE

REM Conditional message
IF ($counter >= 5)
    ENTER
    STRING All done! Typed 5 lines.
    ENTER
END_IF

REM Demonstrate nested loop
STRING Grid:
ENTER
LOOP 3
    LOOP 10
        STRING #
    END_LOOP
    ENTER
END_LOOP

STRING Script complete!
ENTER
```

---

## Error Messages

| Error | Cause | Solution |
|-------|-------|----------|
| `Syntax error line N` | Invalid command | Check spelling/syntax |
| `Unmatched END_*` | Missing opening statement | Add LOOP/WHILE/IF |
| `Unmatched LOOP/WHILE/IF` | Missing END_* | Add closing statement |
| `Variable not found` | Undefined variable | Declare with VAR first |
| `Too many variables` | Exceeded 32 limit | Reuse or reduce variables |
| `Loop depth exceeded` | >8 nested loops | Reduce nesting |
| `Division by zero` | Divide by 0 | Check divisor value |

---

## Tips & Best Practices

1. **Start with delays** - Give target time to process
   ```duckyscript
   DELAY 2000
   ```

2. **Use variables for tuning** - Easy to adjust
   ```duckyscript
   VAR $KEY_DELAY = 50
   DEFAULT_DELAY $KEY_DELAY
   ```

3. **Add comments** - Future you will thank you
   ```duckyscript
   REM Open terminal on Linux
   CTRL-ALT t
   ```

4. **Random delays for stealth** - Avoid detection
   ```duckyscript
   DELAY_RANDOM 50 200
   ```

5. **Test incrementally** - Start small, build up

6. **Use BREAK for safety** - Emergency exit conditions
   ```duckyscript
   IF ($error != 0)
       BREAK
   END_IF
   ```

---

*Bad Duck3 - DuckyScript 3.0 for Flipper Zero*
