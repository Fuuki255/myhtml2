#ifndef MYHTML_STREAM_H
#define MYHTML_STREAM_H

#ifndef MYHTML_METHODS_H
#include "MyHtml_methods.h"
#endif


/* Callbacks */

typedef int (*HTML_Callback_PutChar)(int, void*);
typedef int (*HTML_Callback_PutString)(const char*, void*);

typedef int (*HTML_Callback_GetChar)(void* data);
typedef size_t (*HTML_Callback_Read)(char* buf, size_t size, size_t n, void* data);

typedef void (*HTML_Callback_Seek)(void* data, long where, int seek);
typedef size_t (*HTML_Callback_GetPosition)(void* data);



/* Structures */

// A structed string for better write string
typedef struct HTML_String {
    char* buffer;
    size_t length;
    size_t memsize;
} HTML_String;


typedef struct HTML_Stream {
	void* data;
	
	HTML_Callback_PutChar putc;
	HTML_Callback_PutString puts;

    HTML_Callback_GetChar getc;
    HTML_Callback_Read read;

	HTML_Callback_Seek seek;
	HTML_Callback_GetPosition getPosition;
} HTML_Stream;





/* HTML_String Methods */

#define _HTML_CreateString(memsize)    (HTML_String){(char*)malloc(memsize), 0, memsize}

// To sure the string have enough size to insert new string
// If currentSize + expandSize >= memorySize, string buffer will expand to memorySize * 2 + expandSize
void HTML_ExpandString(HTML_String* str, size_t expand) {
    if (str->length + expand >= str->memsize) {
        size_t newSize = str->memsize * 2 + expand;

        char* newBuffer = (char*)realloc((void*)str->buffer, newSize);
        HTML_Error_NoEnoughMemory(newBuffer, "Failed to expand string!", );

        str->buffer = newBuffer;
        str->memsize = newSize;
    }
}


int HTML_PutCharToString(int c, HTML_String* str) {
    // Expand size
    HTML_ExpandString(str, 1);
    str->buffer[str->length++] = c;
    return 0;
}

int HTML_PutCStringToString(const char* putstr, HTML_String* str) {
    size_t putLength = strlen(putstr);

    // Expand string size
    HTML_ExpandString(str, putLength);

    // Lcoate write position (Visual Studio annoying debug)
    char* part = str->buffer + str->length;
    if (!part) {
        fprintf(stderr, "ERROR: %s: Lcoate write position!\n", __func__);
        return -1;
    }

    // Write string
    HTML_strcpy_s(part, str->memsize - str->length, putstr);
    str->length += putLength;
    return 0;
}

const char* HTML_ConvertStringToCString(HTML_String* str) {
    HTML_ExpandString(str, 1);
    str->buffer[str->length] = 0;
    return str->buffer;
}


void HTML_DestroyString(HTML_String* str) {
    if (str->buffer) {
        free(str->buffer);
    }
}





// char* HTML_CString_Cat(char* str, const char* putstr) {
//     size_t strLength = strlen(str);
//     size_t putLength = strlen(putstr);

//     // Expand string size
//     char* newString = (char*)realloc(str, strLength + putLength + 1);
//     HTML_Error_NoEnoughMemory(newString, "Failed to cat string", str);

//     // Lcoate write position (Visual Studio annoying debug)
//     char* part = newString + strLength;
//     if (!part) {
//         fprintf(stderr, "ERROR: %s: Failed to lcoate write position!\n", __func__);
//         return NULL;
//     }

//     // Write string
//     HTML_strcpy_s(part, str->memsize - str->length, putstr);
//     return newString;
// }




/* HTML_Stream Methods */

#define HTML_PutCharToStream(stream, c)		stream->putc(c, stream->data)
#define HTML_WriteStream(stream, str)		stream->puts(str, stream->data)

#define HTML_GetCharFromStream(stream)			stream->getc(stream->data)
#define HTML_ReadStream(buf, size, n, stream)	stream->read(buf, size, n, stream->data)

#define HTML_SeekStream(stream, offset, place)   stream->seek(stream->data, offset, place)
#define HTML_GetStreamPosition(stream)			stream->getPosition(stream->data)



#endif /* MYHTML_STREAM_H */