#ifndef STYLE_H
#define STYLE_H

// Terminal effects
#define CLEAR_SCREEN "\033c" // Only clear the shown screen (not really)
#define SOUND "\a"

// Cursor manipulation
#define HIDE_CURSOR "\x1b[?25l"
#define SHOW_CURSOR "\x1b[?25h"
#define BLOCK_CURSOR "\x1b[2 q"            // Blinking block
#define STEADY_BLOCK_CURSOR "\x1b[0 q"     // Steady block
#define UNDERLINE_CURSOR "\x1b[4 q"        // Blinking underline
#define STEADY_UNDERLINE_CURSOR "\x1b[3 q" // Steady underline
#define STEADY_BAR_CURSOR "\x1b[6 q"       // Blinking bar
#define BAR_CURSOR "\x1b[5 q"              // Steady bar

// Text formatting
#define RESET "\x1b[0m"         // Reset all styles
#define BOLD "\x1b[1m"          // Bold text
#define RESET_BOLD "\x1b[22m"   // Reset bold text
#define DIM "\x1b[2m"           // Dim text
#define ITALIC "\x1b[3m"        // Italic text
#define UNDERLINE "\x1b[4m"     // Underline text
#define BLINK "\x1b[5m"         // Blink text (COOL)
#define REVERSE "\x1b[7m"       // Reverse colors
#define HIDDEN "\x1b[8m"        // Hidden text
#define STRIKETHROUGH "\x1b[9m" // Strikethrough text
#define DEFAULT "\x1b[39m"      // Reset color

// Text colors
#define BLACK "\x1b[30m"
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define BLUE "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN "\x1b[36m"
#define WHITE "\x1b[37m"

// Bright text colors
#define BRIGHT_BLACK "\x1b[90m"
#define BRIGHT_RED "\x1b[91m"
#define BRIGHT_GREEN "\x1b[92m"
#define BRIGHT_YELLOW "\x1b[93m"
#define BRIGHT_BLUE "\x1b[94m"
#define BRIGHT_MAGENTA "\x1b[95m"
#define BRIGHT_CYAN "\x1b[96m"
#define BRIGHT_WHITE "\x1b[97m"

// Background colors
#define BG_BLACK "\x1b[40m"
#define BG_RED "\x1b[41m"
#define BG_GREEN "\x1b[42m"
#define BG_YELLOW "\x1b[43m"
#define BG_BLUE "\x1b[44m"
#define BG_MAGENTA "\x1b[45m"
#define BG_CYAN "\x1b[46m"
#define BG_WHITE "\x1b[47m"

// Bright background colors
#define BG_BRIGHT_BLACK "\x1b[100m"
#define BG_BRIGHT_RED "\x1b[101m"
#define BG_BRIGHT_GREEN "\x1b[102m"
#define BG_BRIGHT_YELLOW "\x1b[103m"
#define BG_BRIGHT_BLUE "\x1b[104m"
#define BG_BRIGHT_MAGENTA "\x1b[105m"
#define BG_BRIGHT_CYAN "\x1b[106m"
#define BG_BRIGHT_WHITE "\x1b[107m"

#endif