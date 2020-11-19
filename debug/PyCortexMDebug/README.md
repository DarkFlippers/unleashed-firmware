PyCortexMDebug
==============

*A set of GDB/Python-based utilities to make life debugging ARM Cortex-M processors a bit easier*

It will consist of several modules which will hopefully become integrated as they evolve. Presently, there is only one:

## SVD
ARM defines an SVD (System View Description) file format in its CMSIS
standard as a means for Cortex-M-based chip manufacturers to provide a
common description of peripherals, registers, and register fields. You
can download SVD files for different manufacturers
[here](http://www.arm.com/products/processors/cortex-m/cortex-microcontroller-software-interface-standard.php).

My implementation so far has only tested STM32 chips but should hold for others. If others are like those from ST,
expect plenty of errors in the file. Like GPIOA having a register named GPIOB_OSPEEDR and lots of 16-bit registers
that are listed as 32!

The implementation consists of two components -- An lxml-based parser module (pysvd) and a GDB file (gdb_svd).
I haven't yet worked out a perfect workflow for this, though it's quite easy to use when
you already tend to have a GDB initialization file for starting up OpenOCD and the like.
However your workflow works, just make sure to, in GDB:

    source gdb_svd.py
    svd_load [your_svd_file].svd

These files can be huge so it might take a second or two. Anyways, after that, you can do

    svd

to list available peripherals with descriptions. Or you can do

    svd [some_peripheral_name]

to see all of the registers (with their values) for a given peripheral. For more details, run

    svd [some_peripheral_name] [some_register_name]

to see all of the field values with descriptions.

You can add format modifiers like:

* `svd/x` will display values in hex
* `svd/o` will display values in octal
* `svd/t` or `svd/b` will display values in binary
* `svd/a` will display values in hex and try to resolve symbols from the values

All field values are displayed at the correct lengths as provided by the SVD files.
Also, tab completion exists for nearly everything! When in doubt, run `svd help`.

### TODO

Enable writing to registers and individual fields

### Bugs

There are probably a few. All planning, writing, and testing of this was done in an afternoon. There may be
some oddities in working with non-STM32 parts. I'll play with this when I start working with other
controllers again. If something's giving you trouble, describe the problem and it shall be fixed.

## DWT
The ARM Data Watchpoint and Trace Unit (DWT) offers data watchpoints and a series of gated cycle counters. For now,
I only support the raw cycle counter but facilities are in place to make use of others. As this is independent of the
specific device under test, commands are simple and you can configure a clock speed to get real time values from
counters.

    dwt configclk 48000000
    
will set the current core clock speed. Then

    dwt cyccnt reset
    dwt cyccnt enable

will reset and start the cycle counter. At any point

    dwt cycnt

will then indicate the number of cycles and amount of time that has passed.

## ITM/ETM support

This is not implemented yet. I want to have more complete support for some of the nicer debug and trace features
on Cortex-M processors. Parts of this will probably be dependent on OpenOCD and possibly on specific interfaces.
I'll try to avoid this where possible but can't make any promises.
