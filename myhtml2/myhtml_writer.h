#ifndef _MYHTML_WRITER_H_
#define _MYHTML_WRITER_H_

#ifndef _MYHTML_H_
#include "myhtml.h"
#endif



HtmlCode HtmlLibWriteFormattedStringToStream(const char* s, HtmlStream* stream) {
	// convert
	stream->putchar(stream->data, '\"');

    int c;
	for (const char* p = s; (c = *p); p++) {
        switch (c) {
            case '\\':
                stream->write("\\", 2, 1, stream->data);
                continue;
            case '\"':
                stream->write("\\\"", 2, 1, stream->data);
                continue;
            case '\'':
                stream->write("\\\'", 2, 1, stream->data);
                continue;
            case '\t':
                stream->write("\\t", 2, 1, stream->data);
                continue;
            case '\r':
                stream->write("\\r", 2, 1, stream->data);
                continue;
            case '\n':
                stream->write("\\n", 2, 1, stream->data);
                continue;
            case '\a':
                stream->write("\\a", 2, 1, stream->data);
                continue;
            default:
                stream->putchar(stream->data, c);
        }
    }

	stream->putchar(stream->data, '\"');
    return HTML_OK;
}



// Write Attributes
HtmlCode HtmlLibWriteAttributesToStream(HtmlObject* object, HtmlStream* stream) {
    int c;
	const char* attrName, *attrValue;
    HtmlForeachObjectAttributes(object, attrName, attrValue) {
        stream->putchar(stream->data, ' ');
        stream->write((void*)attrName, strlen(attrName), 1, stream->data);

		if (attrValue != NULL) {
			stream->putchar(stream->data, '=');
			HtmlLibWriteFormattedStringToStream(attrValue, stream);
		}
    }
    return HTML_OK;
}


HtmlCode HtmlWriteObjectToStream(HtmlObject* object, HtmlStream* stream) {
	HtmlObject* child;
    const char* innerText;
	
    if (object->type == HTML_TYPE_COMMENT) {
        stream->write("<!--", 4, 1, stream->data);
		
		innerText = HtmlGetObjectInnerText(object);
        stream->write((void*)innerText, strlen(innerText), 1, stream->data);
		
		stream->write("-->", 3, 1, stream->data);
        return HTML_OK;
    }
    if (object->type == HTML_TYPE_DOCTYPE) {
        stream->write("<!DOCTYPE ", 11, 1, stream->data);
		
		innerText = HtmlGetObjectInnerText(object);
        stream->write((void*)innerText, strlen(innerText), 1, stream->data);
		
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
	
	const char* name = HtmlGetObjectName(object);

    // Write starting
    stream->putchar(stream->data, '<');
    stream->write((void*)name, strlen(name), 1, stream->data);

    // Write attributes
    HtmlLibWriteAttributesToStream(object, stream);
    stream->putchar(stream->data, '>');

    // Write Tag inside
    innerText = HtmlGetObjectInnerText(object);
    stream->write((void*)innerText, strlen(innerText), 1, stream->data);

    // Write Children
    HtmlForeachObjectChildren(object, child) {
    HtmlWriteObjectToStream(child, stream);
    
    // Write Closing
    if (child->type != HTML_TYPE_SINGLE)
        stream->write("</", 2, 1, stream->data);
        stream->write((void*)name, strlen(name), 1, stream->data);
        stream->putchar(stream->data, '>');
    }

    // Write interval
    if (object->afterText) {
        stream->write((void*)object->afterText, strlen(object->afterText), 1, stream->data);
    }
    return HTML_OK;
}














#endif /* _MYHTML_WRITER_H_ */