#ifndef _MYHTML_WRITER_H_
#define _MYHTML_WRITER_H_

#ifndef _MYHTML_OBJECT_H_
#include "myhtml.h"
#endif



HtmlCode HtmlLibWriteFormattedStringToStream(const char* s, HtmlStream* stream) {
	// convert
	stream->putchar('\"', stream->data);

    int c;
	for (const char* p = s; (c = *p); p++) {
        switch (c) {
            case '\\':
                stream->write((void*)"\\", 2, 1, stream->data);
                continue;
            case '\"':
                stream->write((void*)"\\\"", 2, 1, stream->data);
                continue;
            case '\'':
                stream->write((void*)"\\\'", 2, 1, stream->data);
                continue;
            case '\t':
                stream->write((void*)"\\t", 2, 1, stream->data);
                continue;
            case '\r':
                stream->write((void*)"\\r", 2, 1, stream->data);
                continue;
            case '\n':
                stream->write((void*)"\\n", 2, 1, stream->data);
                continue;
            case '\a':
                stream->write((void*)"\\a", 2, 1, stream->data);
                continue;
            default:
                stream->putchar(c, stream->data);
        }
    }

	stream->putchar('\"', stream->data);
    return HTML_OK;
}



// Write Attributes
HtmlCode HtmlLibWriteAttributesToStream(HtmlObject* object, HtmlStream* stream) {
	const char* attrName, *attrValue;

    HtmlForeachObjectAttributes(object, attrName, attrValue) {
        stream->putchar(' ', stream->data);
        stream->write((void*)attrName, strlen(attrName), 1, stream->data);

		if (attrValue != NULL) {
			stream->putchar('=', stream->data);
			HtmlLibWriteFormattedStringToStream(attrValue, stream);
		}
    }
    return HTML_OK;
}


HtmlCode HtmlLibWriteObjectToStream(HtmlObject* object, HtmlStream* stream) {
    HtmlObject* child;
    const char* innerText;
	
    if (object->type == HTML_TYPE_COMMENT) {
        stream->write((void*)"<!--", 4, 1, stream->data);
		
		innerText = HtmlGetObjectInnerText(object);
        stream->write((void*)innerText, strlen(innerText), 1, stream->data);
		
		stream->write((void*)"-->", 3, 1, stream->data);
        return HTML_OK;
    }
    if (object->type == HTML_TYPE_DOCTYPE) {
        stream->write((void*)"<!DOCTYPE ", 10, 1, stream->data);
		
		innerText = HtmlGetObjectInnerText(object);
        stream->write((void*)innerText, strlen(innerText), 1, stream->data);
		
		stream->putchar('>', stream->data);
        return HTML_OK;
    }
    if (object->type == HTML_TYPE_DOCUMENT) {
        HtmlForeachObjectChildren(object, child) {
            HtmlLibWriteObjectToStream(child, stream);
        }
        return HTML_OK;
    }

    /* Normal Way */
	
	const char* name = HtmlGetObjectName(object);

    // Write starting
    stream->putchar('<', stream->data);
    stream->write((void*)name, strlen(name), 1, stream->data);

    // Write attributes
    HtmlLibWriteAttributesToStream(object, stream);
    stream->putchar('>', stream->data);

    // Write Tag inside
    innerText = HtmlGetObjectInnerText(object);
    stream->write((void*)innerText, strlen(innerText), 1, stream->data);

    // Write Children
    HtmlForeachObjectChildren(object, child) {
        HtmlLibWriteObjectToStream(child, stream);
    }

    // Write Closing
    if (object->type != HTML_TYPE_SINGLE) {
        stream->write((void*)"</", 2, 1, stream->data);
        stream->write((void*)name, strlen(name), 1, stream->data);
        stream->putchar('>', stream->data);
    }

    // Write interval
    if (object->afterText) {
        stream->write((void*)object->afterText, strlen(object->afterText), 1, stream->data);
    }
    
    return HTML_OK;
}















HtmlCode HtmlWriteObjectToStream(HtmlObject* object, HtmlStream* stream) {
    HtmlHandleNullError(object, HTML_NULL_POINTER);
    HtmlHandleNullError(stream, HTML_NULL_POINTER);

    return HtmlLibWriteObjectToStream(object, stream);
}


HtmlCode HtmlWriteObjectToFileObject(HtmlObject* object, FILE* file) {
    HtmlHandleNullError(object, HTML_NULL_POINTER);
    HtmlHandleNullError(file, HTML_NULL_POINTER);

    HtmlStream stream = HtmlCreateStreamFileObject(file);
    return HtmlLibWriteObjectToStream(object, &stream);
    // no needed to destroy the HtmlStream
}


HtmlCode HtmlWriteObjectToFile(HtmlObject* object, const char* filename) {
    HtmlHandleNullError(object, HTML_NULL_POINTER);
    HtmlHandleEmptyStringError(filename, HTML_EMPTY_STRING);

    FILE* file = fopen(filename, "w");
    HtmlHandleError(file == NULL, HTML_FILE_NOT_WRITABLE, "error %s: failed to open '%s' in write mode", __func__, filename);

    HtmlStream stream = HtmlCreateStreamFileObject(file);
    HtmlCode ret = HtmlLibWriteObjectToStream(object, &stream);
    fclose(file);
    return ret;
}


// convert HtmlObject to html string
// you don't need to destroy the result string as it will destroy with object destroying
const char* HtmlWriteObjectToString(HtmlObject* object) {
    HtmlHandleNullError(object, "");

    // create StreamBuffer that write after object->name
    HtmlStream stream = HtmlCreateStreamBuffer(4096);

    int resultOffset = 1;
    if (object->name) {
        resultOffset += strlen(object->name);
        stream.write(object->name, resultOffset, 1, stream.data);
    }
    else {
        stream.putchar('\0', stream.data);
    }

    // write string
    HtmlCode ret = HtmlLibWriteObjectToStream(object, &stream);
    if (ret != HTML_OK) {
        HtmlDestroyStream(&stream);

        HtmlHandleError(true, "", "error writing object (%d)", ret);
    }

    // store result to object->name
    if (object->name) {
        free(object->name);
    }

    HtmlStreamString* streamString = (HtmlStreamString*)stream.data;
    streamString->buffer[streamString->length] = 0;

    object->name = streamString->buffer;

    // free HtmlStreamString without its buffer
    free(stream.data);

    // return result
    return object->name + resultOffset;
}


#endif /* _MYHTML_WRITER_H_ */