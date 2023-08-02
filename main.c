#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include "./la.h" // Including the custom linear algebra header file.

#define STB_IMAGE_IMPLEMENTATION
#include "./stb_image.h" // Including the stb_image library for image loading.

// Constants for the font display.
#define FONT_WIDTH 128
#define FONT_HEIGHT 64
#define FONT_COLS 18
#define FONT_ROWS 7
#define FONT_CHAR_WIDTH  (FONT_WIDTH  / FONT_COLS)
#define FONT_CHAR_HEIGHT (FONT_HEIGHT / FONT_ROWS)
#define FONT_SCALE 5

// Function to check and handle SDL errors related to integers.
void scc(int code) {
    if (code < 0) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
}

// Function to check and handle SDL errors related to pointers.
void *scp(void *ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
        exit(1);
    }
    return ptr;
}

// Function to load an SDL surface from an image file using the stb_image library.
SDL_Surface *surface_from_file(const char *file_path) {
    int width, height, n;
    unsigned char *pixels = stbi_load(file_path, &width, &height, &n, STBI_rgb_alpha);
    if (pixels == NULL) {
        fprintf(stderr, "ERROR: could not load file %s: %s\n", file_path, stbi_failure_reason());
        exit(1);
    }

    // Define masks and depth for SDL surface creation based on byte order (endianness).
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    const Uint32 rmask = 0xff000000;
    const Uint32 gmask = 0x00ff0000;
    const Uint32 bmask = 0x0000ff00;
    const Uint32 amask = 0x000000ff;
#else // little endian, like x86
    const Uint32 rmask = 0x000000ff;
    const Uint32 gmask = 0x0000ff00;
    const Uint32 bmask = 0x00ff0000;
    const Uint32 amask = 0xff000000;
#endif

    // Calculate pitch (the number of bytes in a row of pixel data) for the SDL surface.
    const int depth = 32;
    const int pitch = 4 * width;

    // Create and return an SDL surface from the loaded image data.
    return scp(SDL_CreateRGBSurfaceFrom(
        (void*)pixels, width, height, depth, pitch, rmask, gmask, bmask, amask));
}

// Constants defining the ASCII character range for display.
#define ASCII_DISPLAY_LOW 32
#define ASCII_DISPLAY_HIGH 126

// Structure to store font information.
typedef struct {
    SDL_Texture *spritesheet; // Texture containing the font spritesheet.
    SDL_Rect glyph_table[ASCII_DISPLAY_HIGH - ASCII_DISPLAY_LOW + 1]; // Table to store character positions in the spritesheet.
} Font;

// Function to load a font from a file and create the glyph table.
Font font_load_from_file(SDL_Renderer *renderer, const char *file_path) {
    Font font = {0};

    // Load the font surface from the file using stb_image.h.
    SDL_Surface *font_surface = surface_from_file(file_path);

    // Create an SDL texture from the font surface.
    font.spritesheet = scp(SDL_CreateTextureFromSurface(renderer, font_surface));

    // Free the font surface as it is no longer needed.
    SDL_FreeSurface(font_surface);

    // Fill the glyph table with the positions of each character in the spritesheet.
    for (size_t ascii = ASCII_DISPLAY_LOW; ascii <= ASCII_DISPLAY_HIGH; ++ascii) {
        const size_t index = ascii - ASCII_DISPLAY_LOW;
        const size_t col = index % FONT_COLS;
        const size_t row = index / FONT_COLS;
        font.glyph_table[index] = (SDL_Rect) {
            .x = col * FONT_CHAR_WIDTH,
            .y = row * FONT_CHAR_HEIGHT,
            .w = FONT_CHAR_WIDTH,
            .h = FONT_CHAR_HEIGHT,
        };
    }

    return font;
}

// Function to render a single character on the screen using the font.
void render_char(SDL_Renderer *renderer, Font *font, char c, Vec2f pos, float scale) {
    // Calculate the destination rectangle for the character based on position and scale.
    const SDL_Rect dst = {
        .x = (int)floorf(pos.x),
        .y = (int)floorf(pos.y),
        .w = (int)floorf(FONT_CHAR_WIDTH * scale),
        .h = (int)floorf(FONT_CHAR_HEIGHT * scale),
    };

    // Ensure the character is within the displayable ASCII range.
    assert(c >= ASCII_DISPLAY_LOW);
    assert(c <= ASCII_DISPLAY_HIGH);

    // Get the index in the glyph table corresponding to the character.
    const size_t index = c - ASCII_DISPLAY_LOW;

    // Render the character to the screen using SDL_RenderCopy.
    scc(SDL_RenderCopy(renderer, font->spritesheet, &font->glyph_table[index], &dst));
}

// Function to render text on the screen with a specific color and scale.
void render_text_sized(SDL_Renderer *renderer, Font *font, const char *text, size_t text_size, Vec2f pos, Uint32 color, float scale) {
    // Set the color modulation of the font spritesheet texture for rendering with the given color.
    scc(SDL_SetTextureColorMod(
        font->spritesheet,
        (color >> (8 * 0)) & 0xff,
        (color >> (8 * 1)) & 0xff,
        (color >> (8 * 2)) & 0xff));

    // Set the alpha modulation of the font spritesheet texture for rendering with the given color alpha value.
    scc(SDL_SetTextureAlphaMod(font->spritesheet, (color >> (8 * 3)) & 0xff));

    // Initialize a pen position to start rendering text at the given position.
    Vec2f pen = pos;
    for (size_t i = 0; i < text_size; ++i) {
        // Render each character on the screen at the current pen position.
        render_char(renderer, font, text[i], pen, scale);
        // Move the pen to the right for the next character based on the font character width and scale.
        pen.x += FONT_CHAR_WIDTH * scale;
    }
}

// Function to render text on the screen with a specific color and default scale.
void render_text(SDL_Renderer *renderer, Font *font, const char *text, Vec2f pos, Uint32 color, float scale) {
    // Call the render_text_sized function with the text size equal to the length of the string.
    render_text_sized(renderer, font, text, strlen(text), pos, color, scale);
}

// Constants for the text editor buffer.
#define BUFFER_CAPACITY 1024

char buffer[BUFFER_CAPACITY];
size_t buffer_cursor = 0;
size_t buffer_size = 0;

#define UNHEX(color) \
    ((color) >> (8 * 0)) & 0xFF, \
    ((color) >> (8 * 1)) & 0xFF, \
    ((color) >> (8 * 2)) & 0xFF, \
    ((color) >> (8 * 3)) & 0xFF

// Function to render the cursor on the screen with a specific color.
void render_cursor(SDL_Renderer *renderer, Uint32 color) {
    // Calculate the rectangle for the cursor position based on the buffer cursor and font scale.
    const SDL_Rect rect = {
        .x = (int)floorf(buffer_cursor * FONT_CHAR_WIDTH * FONT_SCALE),
        .y = 0,
        .w = FONT_CHAR_WIDTH * FONT_SCALE,
        .h = FONT_CHAR_HEIGHT * FONT_SCALE,
    };

    // Set the render draw color to the specified color.
    scc(SDL_SetRenderDrawColor(renderer, UNHEX(color)));

    // Fill the cursor rectangle with the specified color.
    scc(SDL_RenderFillRect(renderer, &rect));
}

// Main function
int main(void) {
    // Initialize SDL.
    scc(SDL_Init(SDL_INIT_VIDEO));

    // Create the SDL window and renderer.
    SDL_Window *window =
        scp(SDL_CreateWindow("Text Editor", 0, 0, 800, 600, SDL_WINDOW_RESIZABLE));
    SDL_Renderer *renderer =
        scp(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED));

    // Load the font from the spritesheet image.
    Font font = font_load_from_file(renderer, "./charmap_white.png");

    bool quit = false;
    while (!quit) {
        // SDL event handling loop.
        SDL_Event event = {0};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT: {
                // Exit the main loop when the window is closed.
                quit = true;
            }
            break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                case SDLK_BACKSPACE: {
                    // Handle Backspace key press to delete characters from the buffer.
                    if (buffer_size > 0) {
                        buffer_size -= 1;
                        buffer_cursor = buffer_size;
                    }
                }
                break;
                }
            }
            break;

            case SDL_TEXTINPUT: {
                // Handle text input events to add characters to the buffer.
                size_t text_size = strlen(event.text.text);
                const size_t free_space = BUFFER_CAPACITY - buffer_size;
                if (text_size > free_space) {
                    text_size = free_space;
                }
                memcpy(buffer + buffer_size, event.text.text, text_size);
                buffer_size += text_size;
                buffer_cursor = buffer_size;
            }
            break;
            }
        }

        // Clear the screen.
        scc(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
        scc(SDL_RenderClear(renderer));

        // Render the text on the screen using the font.
        render_text_sized(renderer, &font, buffer, buffer_size, vec2f(0.0, 0.0), 0xFFFFFFFF, FONT_SCALE);

        // Render the cursor on the screen with the specified color.
        render_cursor(renderer, 0xFFFFFFFF);

        // Update the display.
        SDL_RenderPresent(renderer);
    }

    // Clean up SDL resources and quit.
    SDL_Quit();

    return 0;
}
