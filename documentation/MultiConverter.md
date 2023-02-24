# MultiConverter 

## Author: [theisolinearchip](https://github.com/theisolinearchip/flipperzero_stuff/tree/main/applications/multi_converter)

An expanded version of my previous __Dec/Hex Converter__, this time allowing more units and a _(probably poorly made from a design-point-of-view)_ selector mode
to swap between different unit groups.

I wrote it with the idea of _expanding the unit list_ on mind, so adding new ones it's a matter of increasing an array of constants + defining the proper conversion functions.

(Actually the whole project is more about "making the framework" rather than providing _ALL_ of the possible units : D)

![Img 1](http://albertgonzalez.coffee/projects/flipperzero/multi_converter/img/1_small.png) ![Img 2](http://albertgonzalez.coffee/projects/flipperzero/multi_converter/img/2_small.png)

## Current conversions

- `Decimal / Hexadecimal / Binary`
- `Celsius / Fahrenheit / Kelvin`
- `Kilometers / Meters / Centimeters / Miles / Feet / Inches`
- `Degree / Radian`

## Usage

Base keyboard allows numbers from `0` to `F`, being disabled (or not) according to the current selected unit.

Long press on `0` toggles a __negative__ value; long press on `1` sets a __decimal point__ (only if allowed by the current selected unit).

`<` removes the last character; `#` changes to __Unit Select Mode__.

### Unit Select Mode

`Left` and `Right` to swap between __origin unit__ and __destination unit__ (notice the _destination_ will change according to the current selected _origin_).

`Ok` to save the changes and go back to the __Display Mode__; `Back` to go back without changing any unit.

## Adding new units

1. Add the new units in the `MultiConverterUnitType` enum on `multi_converter_definitions.h` (basic definitions header). Notice each enum element will be used as an array index later.

2. Increase the `MULTI_CONVERTER_AVAILABLE_UNITS` constant on `multi_converter_units.h` (units main header file).

3. Set a pair of functions for __converting__ units and to __check__ if a target unit is allowed to work with the destination unit (both on `multi_converter_units.h`
and `multi_converter_units.c`; follow the already built-in units for more info).

4. Add the proper `MultiConverterUnit` structs for each new unit.

5. Add each new struct to the main `multi_converter_available_units` array.

And that's it! The system will fetch the new units and display it!

## Known issues, TODO-list, etc.

This is an initial release, so expect some bugs and issues (also I don't work with C that much, so there're probably lots of things that can be improved and/or changed!).

- I've noticed some small decimal variations when "going deep" with some units (like converting __miles__ to __centimeters__ and things like that); probably due to the precision-level required. Need to check that.
- Pending: improve overflow checks.
- The way some long numbers are shown could probably be improved to look fancier.
- Both _origin_ and _destination buffers_ are the same. The destination one could probably be longer in order to avoid certain _overflow scenarios_.
- The GUI needs improvement too: there's a whole __widget/views system__ built in the Flipper that allows things like setting up keys, showing "Save/Back/Cancel" messages with
callbacks and stuff like that. Didn't know anything about them, so I moved on with something more basic (which is probably fine since it's not a "very big project"); but
a more "standard" way with the regular GUI stuff provided by the firmware will be interesting...
- More GUI stuff: the _long click buttons_ for adding a decimal point / negative number aren't very clear on the view itself (I tried to add a small dot / dash symbol, but I think those are small enough to be a little bit confusing)
