#include "string_buffer.h"
#include <stdlib.h> // For malloc, realloc, free
#include <string.h> // For strlen, strcat, strcpy, memcpy
#include <stdarg.h> // For va_list, va_start, va_end

// --- Helper function to resize the buffer ---
static int sb_resize(StringBuffer *sb, size_t new_capacity) {
    // Ensure new capacity is at least current length + 1 for null terminator
    if (new_capacity < sb->length + 1) {
        new_capacity = sb->length + 1;
    }

    char *new_buffer = (char *)realloc(sb->buffer, new_capacity);
    if (new_buffer == NULL) {
        return -1; // Memory allocation failed
    }
    sb->buffer = new_buffer;
    sb->capacity = new_capacity;
    return 0; // Success
}

// --- Public API Functions ---

int sb_init(StringBuffer *sb) {
    if (sb == NULL) return -1;

    sb->buffer = (char *)malloc(SB_INITIAL_CAPACITY);
    if (sb->buffer == NULL) {
        return -1; // Memory allocation failed
    }
    sb->buffer[0] = '\0'; // Initialize with an empty string
    sb->length = 0;
    sb->capacity = SB_INITIAL_CAPACITY;
    return 0; // Success
}

int sb_append(StringBuffer *sb, const char *str, unsigned int n) {
    if (sb == NULL || str == NULL) return -1;

    size_t required_capacity = sb->length + n + 1; // +1 for null terminator

    if (required_capacity > sb->capacity) {
        // Calculate new capacity: double it until it's enough, or just enough if doubling isn't sufficient
        size_t new_capacity = sb->capacity * 2;
        while (new_capacity < required_capacity) {
            new_capacity *= 2;
        }
        if (sb_resize(sb, new_capacity) != 0) {
            return -1; // Resize failed
        }
    }

    // Append the string and update length
    memcpy(sb->buffer + sb->length, str, n);
    sb->length += n;
    sb->buffer[sb->length] = '\0'; // Ensure null termination
    return 0; // Success
}

int sb_append_char(StringBuffer *sb, char c) {
    if (sb == NULL) return -1;

    size_t required_capacity = sb->length + 1 + 1; // +1 for char, +1 for null terminator

    if (required_capacity > sb->capacity) {
        size_t new_capacity = sb->capacity * 2;
        while (new_capacity < required_capacity) {
            new_capacity *= 2;
        }
        if (sb_resize(sb, new_capacity) != 0) {
            return -1; // Resize failed
        }
    }

    sb->buffer[sb->length] = c;
    sb->length++;
    sb->buffer[sb->length] = '\0'; // Ensure null termination
    return 0; // Success
}

int sb_append_printf(StringBuffer *sb, const char *format, ...) {
    if (sb == NULL || format == NULL) return -1;

    // Try to append, assuming current capacity is enough
    va_list args;
    va_start(args, format);
    // Use vsnprintf to determine required size
    // A -1 return indicates an error or overflow depending on C standard.
    // We assume C99 behavior where it returns the number of characters that *would* have been written.
    int chars_written = vsnprintf(sb->buffer + sb->length, sb->capacity - sb->length, format, args);
    va_end(args);

    if (chars_written < 0) {
        return -1; // An encoding error or other issue occurred
    }

    // If there wasn't enough space, resize and try again
    if ((size_t)chars_written >= (sb->capacity - sb->length)) {
        size_t required_capacity = sb->length + (size_t)chars_written + 1;
        size_t new_capacity = sb->capacity * 2;
        while (new_capacity < required_capacity) {
            new_capacity *= 2;
        }
        if (sb_resize(sb, new_capacity) != 0) {
            return -1; // Resize failed
        }

        va_start(args, format);
        chars_written = vsnprintf(sb->buffer + sb->length, sb->capacity - sb->length, format, args);
        va_end(args);

        if (chars_written < 0 || (size_t)chars_written >= (sb->capacity - sb->length)) {
            // Should not happen after resize, but a final check for safety
            return -1;
        }
    }

    sb->length += chars_written;
    sb->buffer[sb->length] = '\0'; // Ensure null termination
    return 0; // Success
}

const char* sb_get_string(const StringBuffer *sb) {
    if (sb == NULL) return NULL;
    return sb->buffer;
}

size_t sb_get_length(const StringBuffer *sb) {
    if (sb == NULL) return 0;
    return sb->length;
}

void sb_free(StringBuffer *sb) {
    if (sb == NULL) return;
    free(sb->buffer);
    sb->buffer = NULL;
    sb->length = 0;
    sb->capacity = 0;
}
