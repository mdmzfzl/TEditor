# Text Editor

This is a simple text editor written in C using the SDL (Simple DirectMedia Layer) library for graphics rendering. The text editor just allows you to insert and delete text for now(work in progress) and displays it on the screen with a custom font.

## Features

- Basic text editing (inserting and deleting characters).
- Custom font rendering.
- Cursor position visualization.

## Requirements

- C Compiler (e.g., GCC)
- [SDL2 library](https://www.libsdl.org/)
- [stb_image library](https://github.com/nothings/stb)

## Getting Started

1. Install SDL2 library:
   - Visit the SDL website (https://www.libsdl.org/) and download the SDL development libraries for your operating system.
   - Follow the installation instructions to set up SDL2 on your machine.

2. Download the project files:
   - Clone or download this repository to your local machine.

3. Build the text editor:
   - Open a terminal or command prompt and navigate to the project directory.
   - Run `make` to compile the project using the provided Makefile.
   - This will create an executable named `te` in the project directory.

4. Run the text editor:
   - Use `./textEditor` command to run the text editor executable.
   - The text editor window should appear, and you can start typing and editing text.

5. Text Editor Controls:
   - Use the Backspace key to delete characters.
   - Start typing to insert characters at the current cursor position.

