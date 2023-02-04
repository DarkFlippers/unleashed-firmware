# Intro

Nice to see you reading this document, we really appreciate it.

As all documents of this kind it's unable to cover everything.
But it will cover general rules that we are enforcing on PR review.

Also, we already have automatic rules checking and formatting,
but it got its limitations and this guide is still mandatory.

Some part of this project do have its own naming and coding guides.
For example: assets. Take a look into `ReadMe.md` in assets folder for more details.

Also, 3rd party libraries are none of our concern.

And yes, this set is not final and we are open to discussion.
If you want to add/remove/change something here please feel free to open new ticket.

# Inspiration

Our guide is inspired by, but not claiming to be compatible with:

- https://www.kernel.org/doc/html/v4.10/process/coding-style.html
- https://docs.unrealengine.com/en-US/Programming/Development/CodingStandard
- https://webkit.org/code-style-guidelines/

# General rules

## Readability and Simplicity first

Code we write is intended to be public.
Avoid one-liners from hell and keep code complexity under control.
Try to make code self-explanatory and add comments if needed.
Leave references to standards that you are implementing.
Use project wiki to document new/reverse engineered standards.

## Variable and function names must clearly define what it's doing

It's ok if it will be long, but it should clearly state what it's doing, without need to dive into code.
This also applies to function/method's code.
Try to avoid one letter variables.

## Encapsulation

Don't expose raw data, provide methods to work with it.
Almost everything in flipper firmware is built around this concept.

# C coding style

- Tab is 4 spaces
- Use `fbt format` to reformat source code and check style guide before commit

## Naming

### Type names are PascalCase

Examples:

	FuriHalUsb
	Gui
	SubGhzKeystore


### Functions are snake_case

	furi_hal_usb_init
	gui_add_view_port
	subghz_keystore_read

### File and Package name is a prefix for it's content

This rule makes easier to locate types, functions and sources.

For example:

We have abstraction that we call `SubGhz Keystore`, so there will be:
file `subghz_keystore.h` we have type `SubGhzKeystore` and function `subghz_keystore_read`.

### File names

- Directories: `^[0-9A-Za-z_]+$`
- File names: `^[0-9A-Za-z_]+\.[a-z]+$`
- File extensions: `[ ".h", ".c", ".cpp", ".cxx", ".hpp" ]`

Enforced by linter.

### Standard function/method names

Suffixes:

- `alloc` - allocate and init instance. C style constructor. Returns pointer to instance.
- `free` - de-init and release instance. C style destructor. Takes pointer to instance.

# C++ coding style

Work In Progress. Use C style guide as a base.

# Python coding style

- Tab is 4 spaces
- Use [black](https://pypi.org/project/black/) to reformat source code before commit
