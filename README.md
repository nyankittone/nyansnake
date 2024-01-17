# nyansnake - A simple snake clone that uses the curses library

nyansnake is my little attempt to write a clone of Snake.
Right now, it's rather lackluster in terms of features; pretty much just the core parts of the game have been implemented. I plan on adding more things to this project in the future.

Written in good old C :3

## How to Play
Simply run the executable from the command line, and the game will immediately start. Use the arrow keys or WASD to move your snake around. Collect red pellets to make your snake grow. The longer it gets, the trickier the game gets, as you have to figure out how to navigate around yourself! Once you collide with a wall or your own snake, the game ends. Get your snake to be as long as possible!

## Supported Platforms
This program is designed to run on any generic POSIX-compliant platform (Linux, macOS, BSD, etc.) Windows is **not** supported, and almost certainly
**never will be**.

## Installation Instructions
You will need GNU Make installed, and either gcc or clang for a compiler. Those are
the only ones officially supported, although other compilers that are C99-compliant
may work just fine.

You will also need the header files for a curses implimentation (e.g. ncurses)
installed as well.

### Commands for installing dependencies

**Debian (and derivatives)**: `sudo apt install gcc make libncurses-dev`
**Arch Linux (and derivatives)**: `sudo pacman -S gcc make ncurses`
**Fedora (and similar)**: `sudo dnf install gcc make ncurses-devel`

Once you have all the dependencies, you'll need to of course compile the application. Simply:
+ `cd` into the directory you cloned the git repo into
+ run `make`

After that, you should have a binary file called "nyansnake" inside that directory!

## Planned Features / Tweaks
+ Add an in-game display for the snake's length, pellets consumed, and time elapsed
+ Make the playfield re-center itself when the terminal window gets resized
+ Add a pause feature
+ Add a menu interface into the game
+ Allow for customization of the color palette used for objects in the game
+ Allow for customizable keybinds
+ Fix the code not compiling on macOS without a slight tweak to nyansnake.c

