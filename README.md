ShockPlus - System Shock +, with extra Cheese!
==============================================

Based on the source code of [Shockolate project](https://github.com/Interrupt/systemshock) which based on code for
PowerPC released by Night Dive Studios, Incorporated.

GENERAL NOTES
=============

ShockPlus is a cross platform source port of System Shock, using SDL2 and features of C++17. This runs well on OSX,
Linux, and Windows right now, with some missing features that need reviving due to not being included in the source
code that was released.

Prerequisites
=============
  - Original cd-rom or SS:EE assets in a `res/data` folder next to the executable
    - Floppy disk assets are an older version that we can't load currently


Running
=======

## From source code

See [COMPILING.md](COMPILING.md).

Control modifications
=====================

## Movement

ShockPlus replaces the original game's movement with WASD controls, and uses `F` as the mouselook toggle hotkey. This
differs from the Enhanced Edition's usage of `E` as the mouselook hotkey, but allows us to keep `Q` and `E` available
for leaning.

## Additional hotkeys

* `Ctrl+G` cycles between graphics rendering modes
* `Ctrl+F` to enable full screen mode
* `Ctrl+D` to disable full screen mode 
