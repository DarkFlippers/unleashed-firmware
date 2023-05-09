PyCortexMDebug
==============

## SVD

ARM defines an SVD (System View Description) file format in its CMSIS standard as a means for Cortex-M-based chip manufacturers to provide a common description of peripherals, registers, and register fields. You can download SVD files for different manufacturers [here](http://www.arm.com/products/processors/cortex-m/cortex-microcontroller-software-interface-standard.php).

The implementation consists of two components -- An lxml-based parser module (pysvd) and a GDB file (gdb_svd). I haven't yet worked out a perfect workflow for this, though it's quite easy to use when you already tend to have a GDB initialization file for starting up OpenOCD and the like. However your workflow works, just make sure to, in GDB:

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
