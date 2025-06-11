#ifndef _MYHTML_WRITER_H_
#define _MYHTML_WRITER_H_

#ifndef _MYHTML_H_
#include "myhtml.h"
#endif



HtmlCode HtmlLibWriteFormattedStringToStream(const char* s, HtmlStream* stream) {
	// init convertion map
    static char converts[93][3] = {};
    if (!converts[(uint8_t)'\\'][HTML_OK]) {
        strcpy(converts[(uint8_t)'\\'], "\\\\");
        strcpy(converts[(uint8_t)'\"'], "\\\"");
        strcpy(converts[(uint8_t)'\''], "\\\'");
        strcpy(converts[(uint8_t)'\t'], "\\t");
        strcpy(converts[(uint8_t)'\r'], "\\r");
        strcpy(converts[(uint8_t)'\n'], "\\n");
        strcpy(converts[(uint8_t)'\a'], "\\a");
    }
	
	// convert
	stream->putchar(stream->data, '\"');

	for (char* p = s; (c = *p); p++) {
		if (c < 93) {
			char* specialText = converts[c];

			if (specialText[HTML_OK]) {
				stream->write(specialText, 2, 1, stream->data);
				continue;
			}
		}
		stream->putchar(stream->data, c);
		continue;
	}
	stream->putchar(stream->data, '\"');
}



// Write Attributes
HtmlCode HtmlLibWriteAttributesToStream(HtmlObject* object, HtmlStream* stream) {
    int c;
	const char* attrName, *attrValue;
    HtmlForeachObjectAttributes(object, attrName, attrValue) {
        stream->putchar(stream->data, ' ');
        stream->write(attrName, strlen(attrName), 1, stream->data);

		if (attrValue != NULL) {
			stream->putchar(stream->data, '=');
			HtmlLibWriteFormattedStringToStream(attrValue, stream);
		}
    }
    return HTML_OK;
}


HtmlCode HtmlWriteObjectToStream(HtmlObject* object, HtmlStream* stream) {
	HtmlObject* child;
	
    if (object->type == HTML_TYPE_COMMENT) {
        stream->write("<!--", 4, 1, stream->data);
		
		const char* innerText = HtmlGetObjectInnerText(object);
        stream->write(innerText, strlen(innerText), 1, stream->data);
		
		stream->write("-->", 3, 1, stream->data);
        return HTML_OK;
    }
    if (object->type == HTML_TYPE_DOCTYPE) {
        stream->write("<!DOCTYPE ", 11, 1, stream->data);
		
		const char* innerText = HtmlGetObjectInnerText(object);
        stream->write(innerText, strlen(innerText), 1, stream->data);
		
		stream->putchar(stream->data, '>');
        return HTML_OK;
    }
    if (object->type == HTML_TYPE_DOCUMENT) {
        HtmlForeachObjectChildren(object, child) {
            HtmlWriteObjectToStream(child, stream);
        }
        return HTML_OK;
    }

    /* Normal Way */
	
	const char* name = HtmlObjectGetName(object);

    // Write starting
    stream->putchar(stream->data, '<');
    stream->write(name, strlen(name), 1, stream->data);

    // Write attributes
    HtmlLibWriteAttributesToStream(object, stream);
    stream->putchar(stream->data, '>');

    // Write Tag inside
    if (HTML_Type_HasFlags(object->type, HTML_HAS_TEXT)) {
        // Write Text
        if (object->text) {
            HTML_WriteStream(stream, object->text);
        }

        // Write Children
        if (HTML_Type_HasFlags(object->type, HTML_HAS_CHILD)) {
            HTML_ForeachobjectChildren(object, child) {
                HTML_WriteobjectToStream(child, stream);
            }
        }

        // Write Closing
        HTML_WriteStream(stream, "</");
        HTML_WriteStream(stream, object->name);
        HTML_PutCharToStream(stream, '>');
    }

    // Write interval
    if (object->interval) {
        HTML_WriteStream(stream, object->interval);
    }
    return HTML_OK;
}














#endif /* _MYHTML_WRITER_H_ */