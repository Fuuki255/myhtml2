#ifndef MYHTML_WRITER_H
#define MYHTML_WRITER_H

#define MYHTML_WRITER_VERSION {1,0,1}

#ifndef MYHTML_STREAM_H
#include "MyHtml_stream.h"
#endif




int HTML_WriteAttributesToStream(HTML_Element* element, HTML_Stream* stream) {
    // Init Convert Map
    static char converts[93][3] = {};
    if (!converts[(size_t)'\\'][0]) {
        strcpy(converts[(size_t)'\\'], "\\\\");
        strcpy(converts[(size_t)'\"'], "\\\"");
        strcpy(converts[(size_t)'\''], "\\\'");
        strcpy(converts[(size_t)'\t'], "\\t");
        strcpy(converts[(size_t)'\r'], "\\r");
        strcpy(converts[(size_t)'\n'], "\\n");
        strcpy(converts[(size_t)'\a'], "\\a");
    }

    // Write Attributes
    int c;    
    HTML_ForeachElementAttributes(element, attr) {
        HTML_PutCharToStream(stream, ' ');
        HTML_WriteStream(stream, attr->name);

        if (!attr->value) {
            continue;
        }

        HTML_WriteStream(stream, "=\"");
        for (char* p = attr->value; (c = *p); p++) {
            if (c < 93) {
                char* ret = converts[c];

                if (ret[0]) {
                    HTML_WriteStream(stream, ret);
                    continue;
                }
            }

            HTML_PutCharToStream(stream, c);
            continue;
        }
        HTML_PutCharToStream(stream, '\"');
    }
    return 0;
}



int HTML_WriteElementToStream(HTML_Element* element, HTML_Stream* stream) {
    if (element->type == HTML_COMMENT) {
        HTML_WriteStream(stream, "<!--");
        if (element->text) {
            HTML_WriteStream(stream, element->text);
        }
        HTML_WriteStream(stream, "-->");
        return 0;
    }
    if (element->type == HTML_DOCTYPE) {
        HTML_WriteStream(stream, "<!DOCTYPE ");
        if (element->text) {
            HTML_WriteStream(stream, element->text);
        }
        HTML_PutCharToStream(stream, '>');
        return 0;
    }
    if (element->type == HTML_XML) {
        HTML_WriteStream(stream, "<!-- XML no supported -->");
        return -1;
    }
    if (element->type == HTML_DOCUMENT) {
        HTML_ForeachElementChildren(element, child) {
            HTML_WriteElementToStream(child, stream);
        }
        return 0;
    }

    /* Normal Way */

    // Write starting
    HTML_PutCharToStream(stream, '<');
    HTML_WriteStream(stream, element->name);

    // Write attributes
    if (HTML_Type_HasFlags(element->type, HTML_HAS_ATTR)) {
        HTML_WriteAttributesToStream(element, stream);
    }
    HTML_PutCharToStream(stream, '>');

    // Write Tag inside
    if (HTML_Type_HasFlags(element->type, HTML_HAS_TEXT)) {
        // Write Text
        if (element->text) {
            HTML_WriteStream(stream, element->text);
        }

        // Write Children
        if (HTML_Type_HasFlags(element->type, HTML_HAS_CHILD)) {
            HTML_ForeachElementChildren(element, child) {
                HTML_WriteElementToStream(child, stream);
            }
        }

        // Write Closing
        HTML_WriteStream(stream, "</");
        HTML_WriteStream(stream, element->name);
        HTML_PutCharToStream(stream, '>');
    }

    // Write interval
    if (element->interval) {
        HTML_WriteStream(stream, element->interval);
    }
    return 0;
}







char* HTML_WriteElementToString(HTML_Element* element, size_t* lpSize) {
    // DEBUG: Invild parameters
    HTML_Error_NullParam(element, NULL);
    
    HTML_String str = _HTML_CreateString(256);
    HTML_Error_NoEnoughMemory(str.buffer, "Failed to create string buffer!", NULL);

    HTML_Stream stream = {&str};
    stream.putc = (HTML_Callback_PutChar)HTML_PutCharToString;
    stream.puts = (HTML_Callback_PutString)HTML_PutCStringToString;

    HTML_WriteElementToStream(element, &stream);
    HTML_ConvertStringToCString(&str);

    if (lpSize) {
        *lpSize = str.length;
    }
    return str.buffer;
}


int HTML_WriteElementToFileObject(HTML_Element* element, FILE* file) {
    HTML_Error_NullParam(element, -1);
    HTML_Error_NullParam(file, -1);
    
    HTML_Stream stream = {file};
    stream.putc = (HTML_Callback_PutChar)fputc;
    stream.puts = (HTML_Callback_PutString)fputs;

    HTML_WriteElementToStream(element, &stream);
    return 0;
}

int HTML_WriteElementToFile(HTML_Element* element, const char* filename) {
    HTML_Error_NullParam(element, -1);
    HTML_Error_NullParam(filename, -1);
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "ERROR: %s: Failed to open file!\n", __func__);
        return -1;
    }

    HTML_Stream stream = {file};
    stream.putc = (HTML_Callback_PutChar)fputc;
    stream.puts = (HTML_Callback_PutString)fputs;
    
    HTML_WriteElementToStream(element, &stream);
    fclose(file);
    return 0;
}






#endif /* MYHTML_WRITER_H */