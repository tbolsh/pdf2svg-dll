#ifndef STRING_BUFFER_H
#define STRING_BUFFER_H

#include <stddef.h> // For size_t
#include <stdio.h>  // For va_list in sb_append_printf

// Define an initial capacity to avoid too many reallocations for small strings
#define SB_INITIAL_CAPACITY 128*1024

// Structure to hold our string buffer's state
typedef struct {
    char *buffer;      // Pointer to the character array
    size_t length;     // Current length of the string (excluding null terminator)
    size_t capacity;   // Total allocated capacity (including null terminator space)
} StringBuffer;

// --- Function Declarations ---

// Initializes a new string buffer. Returns 0 on success, -1 on failure.
int sb_init(StringBuffer *sb);

// Appends a string to the buffer. Returns 0 on success, -1 on failure.
int sb_append(StringBuffer *sb, const char *stri, unsigned int length);

// Appends a character to the buffer. Returns 0 on success, -1 on failure.
int sb_append_char(StringBuffer *sb, char c);

// Appends formatted text to the buffer (like sprintf). Returns 0 on success, -1 on failure.
int sb_append_printf(StringBuffer *sb, const char *format, ...);

// Gets the string from the buffer. The returned pointer is owned by the buffer;
// do not free it. It remains valid until the buffer is modified or freed.
const char* sb_get_string(const StringBuffer *sb);

// Gets the current length of the string in the buffer.
size_t sb_get_length(const StringBuffer *sb);

// Frees the memory allocated by the string buffer.
void sb_free(StringBuffer *sb);

#endif // STRING_BUFFER_H
