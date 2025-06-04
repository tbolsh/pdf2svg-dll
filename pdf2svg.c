#include <assert.h>
#include <stdlib.h>
#include <glib.h>
#include <poppler.h>
#include <poppler-document.h>
#include <poppler-page.h>
#include <cairo.h>
#include <cairo-svg.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h> // For va_list, va_start, va_end

#include <windows.h> // Required for DLL_PROCESS_ATTACH, etc.

#include "stringbuffer.h"

StringBuffer *convertPage(PopplerPage *page)
{
	// Poppler stuff 
	double width, height;

	// Cairo stuff
	cairo_surface_t *surface;
	cairo_t *drawcontext;

	if (page == NULL) {
		fprintf(stderr, "Page does not exist\n");
		return NULL;
	}
	poppler_page_get_size (page, &width, &height);
        
        StringBuffer *buffer = malloc(sizeof(StringBuffer));
	sb_init(buffer);
	// Open the SVG file
	surface = cairo_svg_surface_create_for_stream(((cairo_write_func_t) sb_append), buffer, width, height);
	cairo_svg_surface_set_document_unit(surface, CAIRO_SVG_UNIT_PT);
	drawcontext = cairo_create(surface);

	// Render the PDF file into the SVG file
	poppler_page_render_for_printing(page, drawcontext);
	cairo_show_page(drawcontext);

	// Close the SVG file
	cairo_destroy(drawcontext);
	cairo_surface_destroy(surface);

	// Close the PDF file
	g_object_unref(page);
	
	return buffer;     
}

void doNothing(gpointer ptr){}

// A simple function to add two integers
__declspec(dllexport) HANDLE getSinglePage(HANDLE bytes, unsigned int length, int pageNumber) {
    if (bytes == NULL || length == 0) return NULL;
    if (pageNumber <= 1) pageNumber = 1;

    // https://poppler.freedesktop.org/api/glib/PopplerDocument.html
    //  create the PDF file
    PopplerDocument *pdffile;
    PopplerPage *page;

    pdffile = poppler_document_new_from_bytes(g_bytes_new_with_free_func((gconstpointer)bytes, (gsize)length, (GDestroyNotify)doNothing, NULL), NULL, NULL);
    if (pdffile == NULL) {
	fprintf(stderr, "Unable to create pdf doc\n");
	return NULL;
    }
    page = poppler_document_get_page(pdffile, pageNumber-1);
    StringBuffer *sb = convertPage(page);
    g_object_unref(pdffile);

    return (HANDLE)sb;
}

__declspec(dllexport) HANDLE getOnePage(HANDLE doc, unsigned int pageNumber) {
    if (doc == NULL) return NULL;
    if (pageNumber <= 1) pageNumber = 1;
    return (HANDLE)convertPage(poppler_document_get_page((PopplerDocument *)doc, pageNumber-1));
}

__declspec(dllexport) HANDLE getPDFDocument(HANDLE bytes, unsigned int length) {
   if (bytes == NULL || length == 0) return NULL;
   return (HANDLE)poppler_document_new_from_bytes(g_bytes_new_with_free_func((gconstpointer)bytes, (gsize)length, (GDestroyNotify)doNothing, NULL), NULL, NULL);
}

__declspec(dllexport) void freePDFDocument(HANDLE doc) {
   if (doc != NULL) g_object_unref((PopplerDocument *)doc);
}

__declspec(dllexport) HANDLE getSVGPtr(HANDLE svg) {
   if (svg != NULL) return ((StringBuffer *)svg)->buffer; 
   return NULL;
}

__declspec(dllexport) unsigned int getSVGSize(HANDLE svg) {
   if (svg != NULL) return ((StringBuffer *)svg)->length; 
   return 0;
}

__declspec(dllexport) void freeSVG(HANDLE svg) {
    if (svg == NULL) return;
   StringBuffer *sb = (StringBuffer *)svg;
   sb_free(sb);
   free(sb);
}

/*
int main(int argn, char *args[])
{
	// Poppler stuff
	PopplerDocument *pdffile;
	PopplerPage *page;

	// Initialise the GType library
	g_type_init ();

	// Get command line arguments
	if ((argn < 3)||(argn > 4)) {
		printf("Usage: pdf2svg <in file.pdf> <out file.svg> [<page no>]\n");
		return -2;
	}
	gchar *absoluteFileName = getAbsoluteFileName(args[1]);
	gchar *filename_uri = g_filename_to_uri(absoluteFileName, NULL, NULL);
	gchar *pageLabel = NULL;

	char* svgFilename = args[2];

	g_free(absoluteFileName);
	if (argn == 4) {
		// Get page number
		pageLabel = g_strdup(args[3]);
	}

	// Open the PDF file
	pdffile = poppler_document_new_from_file(filename_uri, NULL, NULL);
	g_free(filename_uri);
	if (pdffile == NULL) {
		fprintf(stderr, "Unable to open file\n");
		return -3;
	}

	int conversionErrors = 0;
	// Get the page

	if(pageLabel == NULL) {
		page = poppler_document_get_page(pdffile, 0);
		conversionErrors = convertPage(page, svgFilename);
	}
	else {
		if(strcmp(pageLabel, "all") == 0) {
			int curError;
			int pageCount = poppler_document_get_n_pages(pdffile);

			if(pageCount > 9999999) {
				fprintf(stderr, "Too many pages (>9,999,999)\n");
				return -5;
			}

			size_t svgFilenameBufLen = strlen(svgFilename) + 1;
			char *svgFilenameBuffer = (char*)malloc(svgFilenameBufLen);
			assert(svgFilenameBuffer != NULL);

			int pageInd;
			for(pageInd = 0; pageInd < pageCount; pageInd++) {
				while (1) {
					size_t _wr_len = snprintf(svgFilenameBuffer, svgFilenameBufLen, svgFilename, pageInd + 1);
					if (_wr_len >= svgFilenameBufLen) {
						svgFilenameBufLen = _wr_len + 1;
						svgFilenameBuffer = (char*)realloc(svgFilenameBuffer, svgFilenameBufLen);
						assert(svgFilenameBuffer != NULL);
						continue;
					}
					break;
				}

				page = poppler_document_get_page(pdffile, pageInd);
				curError = convertPage(page, svgFilenameBuffer);
				if(curError != 0)
					conversionErrors = -1;
			}
			free(svgFilenameBuffer);
		}
		else {
			page = poppler_document_get_page_by_label(pdffile, pageLabel);
			conversionErrors = convertPage(page, svgFilename);
			g_free(pageLabel);
		}
	}

	g_object_unref(pdffile);

	if(conversionErrors != 0) {
		return -4;
	}
	else {
		return 0;
	}

}
*/
// DllMain is the entry point for a DLL.
// It's called by the operating system when the DLL is loaded or unloaded.
BOOL APIENTRY DllMain(HMODULE hModule,
                       DWORD  ul_reason_call,
                       LPVOID lpReserved) {
    switch (ul_reason_call) {
    case DLL_PROCESS_ATTACH:
        // The DLL is being loaded into the virtual address space of the current process.
        // Perform one-time initialization here.
        break;
    case DLL_THREAD_ATTACH:
        // A new thread is being created in the current process.
        break;
    case DLL_THREAD_DETACH:
        // A thread is exiting cleanly.
        break;
    case DLL_PROCESS_DETACH:
        // The DLL is being unloaded from the virtual address space of the current process.
        // Perform cleanup here.
        break;
    }
    return TRUE; // Return TRUE to indicate successful initialization.
}

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
