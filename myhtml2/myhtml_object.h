#ifndef _MYHTML_OBJECT_H_
#define _MYHTML_OBJECT_H_

#define HTML_VERSION {2, 0, 1}

// Includes //

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>


// Enums //

typedef enum HtmlCode {
	HTML_OK,
	HTML_SUCCESS = 0,

	HTML_OUT_OF_MEMORY,
	HTML_NULL_POINTER,
	HTML_ITEM_NOT_FOUND,
	HTML_EMPTY_STRING,

	HTML_STREAM_NOT_WRITEABLE,
	HTML_STREAM_NOT_READABLE,
	HTML_STREAM_NOT_SEEKABLE,

	HTML_FILE_NOT_WRITABLE,
	HTML_FILE_NOT_READABLE
} HtmlCode;


typedef enum HtmlObjectFlag {
	HTML_ID_NONE,
	HTML_ID_SINGLE,
	HTML_ID_SCRIPT,
	HTML_ID_TAG,
	HTML_ID_DOCUMENT,
	HTML_ID_COMMENT,
	HTML_ID_DOCTYPE,

	HTML_HAS_NAME = 0x10,
	HTML_HAS_ATTR = 0x20,
	HTML_HAS_TEXT = 0x40,
	HTML_HAS_CHILD = 0x80,
} HtmlObjectFlag;


typedef enum HtmlObjectType {
	HTML_TYPE_NONE,
	HTML_TYPE_SINGLE = HTML_ID_SINGLE | HTML_HAS_NAME | HTML_HAS_ATTR,
	HTML_TYPE_SCRIPT = HTML_ID_SCRIPT | HTML_HAS_NAME | HTML_HAS_ATTR | HTML_HAS_TEXT,
	HTML_TYPE_TAG = HTML_ID_TAG | HTML_HAS_NAME | HTML_HAS_ATTR | HTML_HAS_TEXT | HTML_HAS_CHILD,
	HTML_TYPE_DOCUMENT = HTML_ID_DOCUMENT | HTML_HAS_TEXT,
	HTML_TYPE_COMMENT = HTML_ID_COMMENT | HTML_HAS_TEXT,
	HTML_TYPE_DOCTYPE = HTML_ID_DOCTYPE,
} HtmlObjectType;



// Pre-Defines //

typedef int (*HtmlCallbackGetchar)(void* streamData);
typedef size_t (*HtmlCallbackRead)(void* buf, size_t n, size_t size, void* streamData);

typedef int (*HtmlCallbackPutchar)(int c, void* streamData);
typedef size_t (*HtmlCallbackWrite)(void* buf, size_t n, size_t size, void* streamData);

typedef int (*HtmlCallbackSeek)(void* streamData, long move, int seek);
typedef size_t (*HtmlCallbackTell)(void* streamData);
typedef void (*HtmlCallbackDestroy)(void* streamData);



typedef struct HtmlVersion {
    int major, minor, patch;
} HtmlVersion;



typedef struct HtmlAttribute HtmlAttribute;
typedef struct HtmlObject HtmlObject;

typedef struct HtmlAttribute {
	char* name;
	char* value;
	
	HtmlAttribute* prev, *next;
} HtmlAttribute;


typedef struct HtmlObject {
	HtmlObjectType type;
	
	char* name;
	char* innerText;
	char* afterText;
	
	HtmlObject* firstChild, *lastChild;
	HtmlAttribute* firstAttribute, *lastAttribute;
	
	HtmlObject* parent;
	HtmlObject* prev, *next;
} HtmlObject;




// Useful Macros //

#ifndef MIN
	#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
	#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif


int HtmlLibLowerChar(int c)	{
	return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
}


#define HtmlLibDestroyPointer(p) \
	if (p != NULL) {\
		free((void*)p);\
		p = NULL;\
	}

#define HtmlSetText(var, text) \
	HtmlLibDestroyPointer(var);\
	if (text != NULL) {\
		var = (char*)malloc(strlen(text) + 1);\
		strcpy(var, text);\
	}


void HtmlLibSetTextLowered(char** var, const char* text) {
	if (*var) {
		HtmlLibDestroyPointer((*var));
	}
	if (text) {
		*var = (char*)malloc(strlen(text) + 1);

		int c;
		char* p = *var;
		while ((c = *text++)) {
			*p++ = HtmlLibLowerChar(c);
		}
		*p++ = 0;
	}
}



// Error Handle without debug message
#ifdef HTML_NO_DEBUG
#define HtmlHandleError(check, retValue, ...) \
	if (check) return retValue;


#define HtmlHandleNullError(parameter, retValue) \
	if (parameter == NULL) return retValue;

#define HtmlHandleOutOfMemoryError(pointer, retValue) \
    if (pointer == NULL) return retValue;


#define HtmlHandleEmptyStringError(str, retValue) \
	if (str == NULL || (str == NULL && str[0] == 0))\
        return retValue;

#define HtmlLogWarning(...)


// Error Handle with debug message
#else
#define HtmlHandleError(check, retValue, fmt, ...) \
	if (check) {\
		fprintf(stderr, "error %s: " fmt "\n", __func__, ##__VA_ARGS__);\
		return retValue;\
	}


#define HtmlHandleNullError(parameter, retValue) \
	if (parameter == NULL) {\
		fprintf(stderr, "error %s: Parameter '%s' couldn't be NULL!\n", __func__, #parameter);\
		return retValue;\
	}


#define HtmlHandleOutOfMemoryError(pointer, retValue) \
    if (pointer == NULL) {\
        fprintf(stderr, "error %s: Out of memory!\n", __func__);\
        return retValue;\
    }


#define HtmlHandleEmptyStringError(str, retValue) \
	if (str == NULL || (str == NULL && str[0] == 0)) {\
		fprintf(stderr, "error %s: Empty string as parameter!\n", __func__);\
        return retValue;\
    }


#define HtmlLogWarning(fmt, ...) \
	fprintf(stderr, "warning %s: " fmt "\n", __func__, ##__VA_ARGS__);


#endif



bool HtmlLibIsIntegerIn(int compareValue, ...) {
	va_list args;
	va_start(args, compareValue);
	
	int value;
	while ((value = va_arg(args, int)) != -1) {
		if (value == compareValue) {
			va_end(args);
			return true;
		}
	}
	
	va_end(args);
	return false;
}




// HtmlAttributeIterator //

typedef struct HtmlAttributeIterator {
	HtmlAttribute* prev, *now, *next;
} HtmlAttributeIterator;




#define HtmlBeginAttribute(object) (HtmlAttributeIterator){NULL, NULL, object ? object->firstAttribute : NULL}

#define HtmlEndAttribute(object) (HtmlAttributeIterator){object ? object->lastAttribute : NULL, NULL, NULL}


bool HtmlNextAttribute(HtmlAttributeIterator* iter, const char** attrName, const char** attrValue) {
	iter->prev = iter->now;
	iter->now = iter->next;
	
	if (iter->now != NULL) {
		iter->next = iter->now->next;
        
        *attrName = iter->now->name;
        *attrValue = iter->now->value;
	}
	return iter->now;
}

bool HtmlPrevAttribute(HtmlAttributeIterator* iter, const char** attrName, const char** attrValue) {
	iter->next = iter->now;
	iter->now = iter->prev;
	
	if (iter->now != NULL) {
		iter->prev = iter->now->prev;

        *attrName = iter->now->name;
        *attrValue = iter->now->value;
	}
	return iter->now;
}


#define HtmlForeachObjectAttributes(object, attrName, attrValue) \
	for (HtmlAttributeIterator object##AttrIter = HtmlBeginAttribute(object); HtmlNextAttribute(&object##AttrIter, &attrName, &attrValue); )



// HtmlObjectIterator //

typedef struct HtmlObjectIterator {
	HtmlObject* prev, *now, *next;
} HtmlObjectIterator;




#define HtmlBeginObject(object) (HtmlObjectIterator){NULL, NULL, object ? object->firstChild : NULL}

#define HtmlEndObject(object) (HtmlObjectIterator){object ? object->lastChild : NULL, NULL, NULL}


HtmlObject* HtmlNextObject(HtmlObjectIterator* iter) {
	iter->prev = iter->now;
	iter->now = iter->next;
	
	if (iter->now != NULL) {
		iter->next = iter->now->next;
	}
	return iter->now;
}

HtmlObject* HtmlPrevObject(HtmlObjectIterator* iter) {
	iter->next = iter->now;
	iter->now = iter->prev;
	
	if (iter->now != NULL) {
		iter->prev = iter->now->prev;
	}
	return iter->now;
}


#define HtmlForeachObjectChildren(object, child) \
	for (HtmlObjectIterator object##childIter = HtmlBeginObject(object); (child = HtmlNextObject(&object##childIter)); )


// HtmlStream Methods //

typedef struct HtmlStream {
	void* data;
	HtmlCallbackDestroy destroy;
	
	HtmlCallbackGetchar getchar;
	HtmlCallbackRead read;
	HtmlCallbackPutchar putchar;
	HtmlCallbackWrite write;
	HtmlCallbackSeek seek;
	HtmlCallbackTell tell;
} HtmlStream;

typedef struct HtmlStreamString {
	char* buffer;
	size_t position; // for reading
	size_t length;
	size_t capacity;
} HtmlStreamString;


// Basic methods

#define HtmlCreateStreamEmpty() (HtmlStream){0}

void HtmlDestroyStream(HtmlStream* stream) {
	if (stream && stream->destroy) {
		stream->destroy(stream->data);
		stream->data = NULL;
	}
}

#define HtmlIsStreamReadable(stream) (stream != NULL && stream->getchar != NULL && stream->read)
#define HtmlIsStreamWritable(stream) (stream != NULL && stream->putchar != NULL && stream->write)
#define HtmlIsStreamSeekable(stream) (stream != NULL && stream->seek != NULL)


#define HtmlGetcharFromStream(stream) \
	(stream != NULL && stream->getchar != NULL ? stream->getchar(stream->data) : EOF)

#define HtmlReadContentFromStream(buf, length, size, stream) \
	(stream != NULL && stream->read != NULL ? stream->read(buf, length, size, stream->data) : 0)

#define HtmlPutcharToStream(c, stream) \
	(stream != NULL && stream->putchar != NULL ? stream->putchar(c, stream->data) : EOF)

#define HtmlWriteContentToStream(content, length, size, stream) \
	(stream != NULL && stream->write != NULL ? stream->write(content, length, size, stream->data) : 0)


#define HtmlGetStreamPosition(stream) \
	(stream != NULL && stream->tell != NULL ? stream->tell(stream->data) : 0)



// string stream


HtmlCode HtmlLibExpandStreamString(HtmlStreamString* streamData, size_t expandSize) {
	if (streamData->length + expandSize >= streamData->capacity) {
		size_t newCapacity = (streamData->capacity + expandSize) * 2;
		char* newBuffer = (char*)realloc(streamData->buffer, newCapacity);

		HtmlHandleOutOfMemoryError(newBuffer, HTML_OUT_OF_MEMORY);

		streamData->buffer = newBuffer;
		streamData->capacity = newCapacity;
	}
	return HTML_OK;
}




int HtmlLibGetcharFromStreamString(HtmlStreamString* stream) {
	if (stream->position >= stream->length) {
		return EOF; // End of stream
	}

	return stream->buffer[stream->position++];
}

size_t HtmlLibReadFromStreamString(void* content, size_t length, size_t size, HtmlStreamString* streamData) {
	size_t total = length * size;
	size_t less = streamData->length - streamData->position;
	total = MIN(total, less);

	memcpy(content, streamData->buffer + streamData->position, total);
	streamData->position += total;
	return total;
}




int HtmlLibPutcharToStreamString(int c, HtmlStreamString* stream) {
	// expand size
	if (HtmlLibExpandStreamString(stream, 1) != HTML_OK) {
		return 0; // Out of memory
	}

	// put char
	stream->buffer[stream->length++] = (char)c;
	// stream->buffer[stream->length] = 0;			// Null-terminate the string but removed for performance
	return c;
}

size_t HtmlLibWriteStreamString(void* content, size_t length, size_t size, HtmlStreamString* stream) {
	size_t total = length * size;

	// expand size
	if (HtmlLibExpandStreamString(stream, total) != HTML_OK) {
		return 0; // Out of memory
	}

	// add content
	memcpy(stream->buffer + stream->length, content, total);
	stream->length += total;
	// stream->buffer[stream->length] = 0;			// Null-terminate the string but removed for performance

	return total;
}


int HtmlLibSeekStreamString(HtmlStreamString* streamData, long move, int seek) {
	if (seek == SEEK_SET) {
		streamData->position = move;
	} else if (seek == SEEK_CUR) {
		streamData->position += move;
	} else if (seek == SEEK_END) {
		streamData->position = streamData->position + move;
	}

	if (streamData->position < 0) {
		streamData->position = 0;
	} else if (streamData->position > streamData->length) {
		streamData->position = streamData->length;
	}

	return HTML_OK;
}

long HtmlLibTellStreamString(HtmlStreamString* streamData) {
	return streamData->position;
}





void HtmlLibDestroyStringStream(HtmlStreamString* streamData) {
	if (streamData != NULL) {
		free(streamData->buffer);
		free(streamData);
	}
}



#define HtmlLibInitStreamString(streamData) \
	(HtmlStream){\
		.data = streamData,\
		.destroy = (HtmlCallbackDestroy)HtmlLibDestroyStringStream,\
		.getchar = (HtmlCallbackGetchar)HtmlLibGetcharFromStreamString,\
		.read = (HtmlCallbackRead)HtmlLibReadFromStreamString,\
		.putchar = (HtmlCallbackPutchar)HtmlLibPutcharToStreamString,\
		.write = (HtmlCallbackWrite)HtmlLibWriteStreamString,\
		.seek = (HtmlCallbackSeek)HtmlLibSeekStreamString,\
		.tell = (HtmlCallbackTell)HtmlLibTellStreamString\
	}

HtmlStream HtmlCreateStreamBuffer(size_t blockSize) {
	// create streamData //
	HtmlStreamString* streamString = (HtmlStreamString*)malloc(sizeof(HtmlStreamString));
	HtmlHandleOutOfMemoryError(streamString, HtmlCreateStreamEmpty());

	// Initialize streamData
	streamString->buffer = (char*)malloc(blockSize);
	if (streamString->buffer == NULL) {
		free(streamString);
		HtmlHandleOutOfMemoryError(NULL, HtmlCreateStreamEmpty());
	}
	streamString->buffer[0] = 0;
	
	// other streamData
	streamString->position = 0;
	streamString->length = 0;
	streamString->capacity = blockSize;

	return HtmlLibInitStreamString(streamString);
}

// create a HtmlStream almost for read string
HtmlStream HtmlCreateStreamString(char* str) {
	HtmlHandleEmptyStringError(str, HtmlCreateStreamEmpty());

	// create streamData //
	HtmlStreamString* streamData = (HtmlStreamString*)malloc(sizeof(HtmlStreamString));
	HtmlHandleOutOfMemoryError(streamData, HtmlCreateStreamEmpty());

	// Initialize streamData
	streamData->length = strlen(str);
	*streamData = (HtmlStreamString){(char*)str, 0, streamData->length, streamData->length + 1};

	return HtmlLibInitStreamString(streamData);
}





// get

const char* HtmlGetStreamString(HtmlStream* stream) {
	HtmlHandleNullError(stream, "");
	HtmlHandleError(stream->destroy == NULL, "", "not a StreamString!");
	
	HtmlStreamString* streamString = (HtmlStreamString*)stream->data;
	streamString->buffer[streamString->length] = 0; // Ensure null-termination
	
	return streamString->buffer;
}




// StreamFile

HtmlStream HtmlCreateStreamFileObject(FILE* file) {
	HtmlHandleNullError(file, HtmlCreateStreamEmpty());

	HtmlStream stream = HtmlCreateStreamEmpty();
	stream.data = file;

	stream.putchar = (HtmlCallbackPutchar)fputc;
	stream.write = (HtmlCallbackWrite)fwrite;
	stream.getchar = (HtmlCallbackGetchar)fgetc;
	stream.read = (HtmlCallbackRead)fread;
	stream.seek = (HtmlCallbackSeek)fseek;
	stream.tell = (HtmlCallbackTell)ftell;
	return stream;
}

HtmlStream HtmlCreateStreamFile(const char* filename, const char* mode) {
	HtmlHandleEmptyStringError(filename, HtmlCreateStreamEmpty());

	FILE* file = fopen(filename, mode);
	HtmlStream stream = HtmlCreateStreamFileObject(file);
	stream.destroy = (HtmlCallbackDestroy)fclose;

	return stream;
}



// HtmlObject //

// Create

HtmlObject* HtmlAddObjectChild(HtmlObject* parent, HtmlObject* child);

HtmlObject* HtmlLibCreateObject(HtmlObjectType type, const char* name, HtmlObject* parent) {
	// assign memory
	HtmlObject* object = (HtmlObject*)calloc(1, sizeof(HtmlObject));
    HtmlHandleOutOfMemoryError(object, NULL);

	// set value
	object->type = type;
	HtmlLibSetTextLowered(&object->name, name);
	
	// set relationship
	if (parent != NULL) {
		HtmlAddObjectChild(parent, object);
	}
	return object;
}


HtmlObject* HtmlCreateObjectDocument() {
	return HtmlLibCreateObject(HTML_TYPE_DOCUMENT, NULL, NULL);
}


HtmlObject* HtmlCreateObjectComment(HtmlObject* parent, const char* text) {
	HtmlObject* comment = HtmlLibCreateObject(HTML_TYPE_COMMENT, NULL, parent);
	HtmlSetText(comment->innerText, text);
	return comment;
}


HtmlObject* HtmlCreateObjectTag(HtmlObject* parent, const char* name) {
	HtmlHandleNullError(name, NULL);
	
	HtmlObject* tag = HtmlLibCreateObject(HTML_TYPE_TAG, name, parent);
	return tag;
}

HtmlObject* HtmlCreateObjectTagEx(HtmlObject* parent, const char* name, const char* innerText, const char* afterText) {
	HtmlHandleNullError(name, NULL);
	
	HtmlObject* tag = HtmlLibCreateObject(HTML_TYPE_TAG, name, parent);
	HtmlSetText(tag->innerText, innerText);
	HtmlSetText(tag->afterText, afterText);
	return tag;
}


// Create a HTML script
// input text to be script content, or NULL to create empty script
HtmlObject* HtmlCreateObjectScript(HtmlObject* parent, const char* content) {
	HtmlObject* script = HtmlLibCreateObject(HTML_TYPE_SCRIPT, "script", parent);
	HtmlSetText(script->innerText, content);
	return script;
}


HtmlObject* HtmlCrerateObjectStyle(HtmlObject* parent, const char* content) {
	HtmlObject* script = HtmlLibCreateObject(HTML_TYPE_SCRIPT, "style", parent);
	HtmlSetText(script->innerText, content);
	return script;	
}


HtmlObject* HtmlCreateObjectSingle(HtmlObject* parent, const char* name) {
	HtmlHandleNullError(name, NULL);
	
	HtmlObject* singleTag = HtmlLibCreateObject(HTML_TYPE_SINGLE, name, parent);
	return singleTag;
}




// Destroy

void HtmlDestroyObject(HtmlObject* object);



void HtmlClearObjectAttributes(HtmlObject* object) {
    const char* attrName, *attrValue;
    HtmlForeachObjectAttributes(object, attrName, attrValue) {
        HtmlLibDestroyPointer(attrName);
        HtmlLibDestroyPointer(attrValue);
        
        free(objectAttrIter.now);
    }

    object->firstAttribute = NULL;
    object->lastAttribute = NULL;
}

void HtmlClearObjectChildren(HtmlObject* object) {
    HtmlObject* child;
    HtmlForeachObjectChildren(object, child) {
        HtmlDestroyObject(child);
    }

    object->firstChild = NULL;
    object->lastChild = NULL;
}


void HtmlDestroyObject(HtmlObject* object) {
	HtmlClearObjectChildren(object);
    HtmlClearObjectAttributes(object);
	
	HtmlLibDestroyPointer(object->name);
	HtmlLibDestroyPointer(object->innerText);
	HtmlLibDestroyPointer(object->afterText);


    // Clear relationship
    if (object->parent) {
		if (object->next) {
			object->next->prev = object->prev;
			object->next = NULL;
		}
		if (object->prev) {
			object->prev->next = object->next;
			object->prev = NULL;
		}
		if (object->parent->firstChild == object) {
			object->parent->firstChild = object->next;
		}
		if (object->parent->lastChild == object) {
			object->parent->lastChild = object->prev;
		}
		object->parent = NULL;
	}

    free(object);
}




// Add

HtmlObject* HtmlAddObjectChild(HtmlObject* parent, HtmlObject* child) {
	// Check invalid parameter
    HtmlHandleNullError(parent, NULL);
    HtmlHandleNullError(child, NULL);
	
	// clear old relationship
	if (child->parent) {
		child->prev->next = child->next;
		child->next->prev = child->prev;
		
		if (child->parent->firstChild == child) {
			child->parent->firstChild = child->next;
		}
		if (child->parent->lastChild == child) {
			child->parent->lastChild = child->prev;
		}
		
		// don't need to set child pointer to NULL, it will set after
	}

	// Edit child relationship setting
    child->parent = parent;
    child->prev = parent->lastChild;
    child->next = NULL;

	// Edit parent relationship setting
    if (parent->lastChild) {
        parent->lastChild->next = child;
    }
    if (!parent->firstChild) {
        parent->firstChild = child;
    }
    parent->lastChild = child;

    return child;
}



// Set

HtmlCode HtmlSetObjectInnerText(HtmlObject* object, const char* text) {
    HtmlHandleNullError(object, HTML_NULL_POINTER);

    HtmlSetText(object->innerText, text);
    return HTML_OK;
}


HtmlCode HtmlSetObjectAttribute(HtmlObject* object, const char* attrName, const char* attrValue) {
    HtmlHandleNullError(object, HTML_NULL_POINTER);
    HtmlHandleNullError(attrName, HTML_NULL_POINTER);

    // moidfy attrValue if attribute exists
    const char* _attrName, *_attrValue;
    HtmlForeachObjectAttributes(object, _attrName, _attrValue) {
        if (strcmp(attrName, _attrName) == 0) {
            // Update existing attribute
            HtmlSetText(objectAttrIter.now->value, attrValue);
            return HTML_OK;
        }
    }

    // Create new attribute if attribute not exists
    HtmlAttribute* attr = (HtmlAttribute*)malloc(sizeof(HtmlAttribute));
    HtmlHandleOutOfMemoryError(attr, HTML_OUT_OF_MEMORY);
    
    // Set attribute name
    attr->name = (char*)malloc(strlen(attrName) + 1);
    HtmlHandleOutOfMemoryError(attr->name, HTML_OUT_OF_MEMORY);
    strcpy(attr->name, attrName);

    // Set attribute value
    if (attrValue != NULL && attrValue[0] != '\0') {
        attr->value = (char*)malloc(strlen(attrValue) + 1);
        HtmlHandleOutOfMemoryError(attr->value, HTML_OUT_OF_MEMORY);

		strcpy(attr->value, attrValue);
    }
    else {
        attr->value = NULL;
    }
    
    // Setup relationship
    attr->prev = object->lastAttribute;
    attr->next = NULL;

    if (object->lastAttribute) {
        object->lastAttribute->next = attr;
    }
    object->lastAttribute = attr;

    if (object->firstAttribute == NULL) {
        object->firstAttribute = attr;
    }
    return HTML_OK;
}



// Get

const char* HtmlGetObjectName(HtmlObject* object) {
	HtmlHandleNullError(object, "(null pointer)");
	
	switch (object->type) {
		case HTML_TYPE_NONE:
			return "(none)";
		case HTML_TYPE_DOCUMENT:
			return "(document)";
		case HTML_TYPE_DOCTYPE:
			return "(doctype)";
		case HTML_TYPE_COMMENT:
			return "(comment)";
		default:
			return object->name ? object->name : "(unknown)";
	}
}

const char* HtmlGetObjectInnerText(HtmlObject* object) {
	HtmlHandleNullError(object, NULL);
	return object->innerText ? object->innerText : "";
}

HtmlCode HtmlLibGetObjectText(HtmlObject* object, HtmlStream* stream) {
	// Write innerText
	if (object->innerText) {
		size_t len = strlen(object->innerText);
		if (stream->write(object->innerText, len, 1, stream->data) != len) {
			return HTML_OUT_OF_MEMORY;
		}
	}

	// Write object children's text
	HtmlObject* child;
	HtmlForeachObjectChildren(object, child) {
		// Skip writing afterText for COMMENT, DOCTYPE, and SCRIPT types
		if (HtmlLibIsIntegerIn(child->type, HTML_TYPE_COMMENT, HTML_TYPE_DOCTYPE, HTML_TYPE_SCRIPT, -1)) {
			continue;
		}

		// Special case (br or hr)
		if (strcmp(child->name, "br") == 0) {
			// Write a newline for <br> and <hr> tags
			if (stream->putchar('\n', stream->data)) {
				return HTML_OUT_OF_MEMORY;
			}
			continue;
		}
		if (strcmp(child->name, "hr") == 0) {
			// Write a newline for <br> and <hr> tags
			if (stream->write((void*)"\n\n", 2, 1, stream->data) != 2) {
				return HTML_OUT_OF_MEMORY;
			}
			continue;
		}


		// Write child's text unless its type SINGLE
		if (child->type != HTML_TYPE_SINGLE) {
			HtmlCode code = HtmlLibGetObjectText(child, stream);
			if (code != HTML_OK) {
				return code;
			}
		}

		// Write child's afterText to stream
		if (child->afterText) {
			size_t len = strlen(child->afterText);
			if (stream->write(child->afterText, len, 1, stream->data) != len) {
				return HTML_OUT_OF_MEMORY;
			}
		}
	}
	return HTML_OK;
}

HtmlCode HtmlGetObjectText(HtmlObject* object, HtmlStream* stream) {
	// checks parameter useable
	HtmlHandleNullError(object, HTML_NULL_POINTER);
	HtmlHandleNullError(stream, HTML_NULL_POINTER);

	// Check a valid object type
	if (HtmlLibIsIntegerIn(object->type, HTML_TYPE_DOCUMENT, HTML_TYPE_TAG, -1) == false) {
		return HTML_OK; // No text to write for not text-containing types
	}

	// Check if stream is writeable
	HtmlHandleError(stream->write == NULL, HTML_STREAM_NOT_WRITEABLE,
		"HtmlStream is not writeable, please setup the write callback function!");
	
	return HtmlLibGetObjectText(object, stream);
}



const char* HtmlGetObjectAfterText(HtmlObject* object) {
	HtmlHandleNullError(object, NULL);
	return object->afterText ? object->afterText : "";
}

const char* HtmlGetObjectAttributeValue(HtmlObject* object, const char* attrName) {
	HtmlHandleNullError(object, NULL);
	HtmlHandleEmptyStringError(attrName, NULL);

	// Search for the attribute
	const char* _attrName, *_attrValue;
	HtmlForeachObjectAttributes(object, _attrName, _attrValue) {
		if (strcmp(attrName, _attrName) == 0) {
			return _attrValue;
		}
	}
	return ""; // Attribute not found
}

HtmlObject* HtmlGetObjectFirstChild(HtmlObject* object) {
	HtmlHandleNullError(object, NULL);
	return object->firstChild;
}

HtmlObject* HtmlGetObjectLastChild(HtmlObject* object) {
	HtmlHandleNullError(object, NULL);
	return object->lastChild;
}

HtmlObject* HtmlGetObjectParent(HtmlObject* object) {
	HtmlHandleNullError(object, NULL);
	return object->parent;
}


const char* HtmlGetObjectTypeString(HtmlObject* object) {
	HtmlHandleNullError(object, "(null pointer)");
	
	switch (object->type) {
		case HTML_TYPE_NONE:
			return "NONE";
		case HTML_TYPE_SINGLE:
			return "SINGLE";
		case HTML_TYPE_SCRIPT:
			return "SCRIPT";
		case HTML_TYPE_TAG:
			return "TAG";
		case HTML_TYPE_DOCUMENT:
			return "DOCUMENT";
		case HTML_TYPE_COMMENT:
			return "COMMENT";
		case HTML_TYPE_DOCTYPE:
			return "DOCTYPE";
		default:
			return "(unknown)";
	}
}



// Remove

HtmlCode HtmlRemoveObjectAttribute(HtmlObject* object, const char* attrName) {
	HtmlHandleNullError(object, HTML_NULL_POINTER);
	HtmlHandleNullError(attrName, HTML_NULL_POINTER);

	// Search for the attribute
	const char* _attrName, *_attrValue;
	HtmlForeachObjectAttributes(object, _attrName, _attrValue) {
		if (strcmp(attrName, _attrName) == 0) {
			// Remove relationships
			if (objectAttrIter.prev) {
				objectAttrIter.prev->next = objectAttrIter.now->next;
			}
			if (objectAttrIter.next) {
				objectAttrIter.next->prev = objectAttrIter.now->prev;
			}
			if (object->firstAttribute == objectAttrIter.now) {
				object->firstAttribute = objectAttrIter.next;
			}
			if (object->lastAttribute == objectAttrIter.now) {
				object->lastAttribute = objectAttrIter.prev;
			}

			// Free the attribute memory
			HtmlLibDestroyPointer(_attrName);
			HtmlLibDestroyPointer(_attrValue);
			free(objectAttrIter.now);
			return HTML_OK;
		}
	}
	return HTML_ITEM_NOT_FOUND;
}





// Copy

HtmlObject* HtmlCopyObject(HtmlObject* object) {
	HtmlHandleNullError(object, NULL);

	// Create a new object with the same type and name
	HtmlObject* copy = (HtmlObject*)calloc(1, sizeof(HtmlObject));
	HtmlHandleOutOfMemoryError(copy, NULL);

	// Copy innerText and afterText
	HtmlSetText(copy->innerText, object->innerText);
	HtmlSetText(copy->afterText, object->afterText);

	// Copy attributes
	const char* attrName, *attrValue;
	HtmlForeachObjectAttributes(object, attrName, attrValue) {
		if (HtmlSetObjectAttribute(copy, attrName, attrValue) != HTML_OK) {
			HtmlDestroyObject(copy);
			return NULL; // Out of memory error
		}
	}

	// Copy children
	HtmlObject* child;
	HtmlForeachObjectChildren(object, child) {
		HtmlObject* childCopy = HtmlCopyObject(child);
		if (childCopy == NULL) {
			HtmlDestroyObject(copy);
			return NULL; // Out of memory error
		}
		HtmlAddObjectChild(copy, childCopy);
	}

	return copy;
}





// Count

size_t HtmlCountObjectChildren(HtmlObject* object) {
	HtmlHandleNullError(object, 0);

	size_t count = 0;
	HtmlObject* child;
	HtmlForeachObjectChildren(object, child) {
		count++;
	}
	return count;
}

size_t HtmlCountObjectAttributes(HtmlObject* object) {
	HtmlHandleNullError(object, 0);

	size_t count = 0;
	const char* attrName, *attrValue;
	HtmlForeachObjectAttributes(object, attrName, attrValue) {
		count++;
	}
	return count;
}



// Print Information

HtmlCode HtmlPrintObjectInfo(HtmlObject* object) {
	printf("HtmlObject <%s> (%s)\n", HtmlGetObjectName(object), HtmlGetObjectTypeString(object));
	printf("innerText: %s\n", HtmlGetObjectInnerText(object));
	printf("afterText: %s\n", HtmlGetObjectAfterText(object));
	
	printf("attributes: ");
	const char* attrName, *attrValue;
	HtmlForeachObjectAttributes(object, attrName, attrValue) {
		printf("%s ", attrName);
	}
	putchar('\n');
	
	printf("children: ");
	HtmlObject* child;
	HtmlForeachObjectChildren(object, child) {
		printf("<%s> ", HtmlGetObjectName(child));
	}
	putchar('\n');
	
	printf("parent: <%s>\n", object->parent ? HtmlGetObjectName(object->parent) : "(null)");
	printf("prev: <%s>\n", object->prev ? HtmlGetObjectName(object->prev) : "(null)");
	printf("next: <%s>\n", object->next ? HtmlGetObjectName(object->next) : "(null)");
	putchar('\n');
	
	return HTML_OK;
}




#endif // _MYHTML_OBJECT_H_ //