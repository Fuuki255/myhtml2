#ifndef _MYHTML_H_
#define _MYHTML_H_

#define HTML_VERSION "2.0.0"

// Includes //

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>


// Enums //

typedef enum HtmlCode {
	HTML_OK,
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

typedef int (*HtmlCallbackPutchar)(void* streamData, int c);
typedef size_t (*HtmlCallbackWrite)(void* buf, size_t n, size_t size, void* streamData);

typedef int (*HtmlCallbackSeek)(void* streamData, long move, int seek);
typedef void (*HtmlCallbackDestroy)(void* streamData);


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

#define HtmlDestroyPointer(p) \
	if (p != NULL) {\
		free((void*)p);\
		p = NULL;\
	}

#define HtmlSetText(var, text) \
	HtmlDestroyPointer(var);\
	if (text != NULL) {\
		var = (char*)malloc(strlen(text) + 1);\
		strcpy(var, text);\
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


// Error Handle with debug message
#else
#define HtmlHandleError(check, retValue, ...) \
	if (check) {\
		fprintf(stderr, __VA_ARGS__);\
		return retValue;\
	}


#define HtmlHandleNullError(parameter, retValue) \
	if (parameter == NULL) {\
		fprintf(stderr, "%s: Parameter '%s' couldn't be NULL!\n", __func__, #parameter);\
		return retValue;\
	}


#define HtmlHandleOutOfMemoryError(pointer, retValue) \
    if (pointer == NULL) {\
        fprintf(stderr, "%s: Out of memory!\n", __func__);\
        return retValue;\
    }


#define HtmlHandleEmptyStringError(str, retValue) \
	if (str == NULL || (str == NULL && str[0] == 0)) {\
		fprintf(stderr, "%s: Empty string as parameter!\n", __func__);\
        return retValue;\
    }


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
        
		printf("HtmlNextAttribute: %s = %s\n", iter->now->name, iter->now->value); // Debug output
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
} HtmlStream;

typedef struct HtmlStringStream {
	char* buffer;
	size_t length;
	size_t capacity;
} HtmlStringStream;


// Basic methods

#define HtmlCreateStreamEmpty() {0}

void HtmlDestroyStream(HtmlStream* stream) {
	if (stream == NULL) {
		return;
	}

	// Call destroy callback if set
	if (stream->destroy) {
		stream->destroy(stream->data);
	}
}




// string stream

int HtmlLibPutcharToStringStream(HtmlStringStream* stream, int c) {
	if (stream->length + 1 >= stream->capacity) {
		size_t newCapacity = stream->capacity * 2 + 1;
		char* newBuffer = (char*)realloc(stream->buffer, newCapacity);
		if (!newBuffer) {
			return EOF; // Out of memory
		}

		stream->buffer = newBuffer;
		stream->capacity = newCapacity;
	}

	stream->buffer[stream->length++] = (char)c;
	stream->buffer[stream->length] = 0;
	return c;
}

size_t HtmlLibWriteStringStream(void* content, size_t length, size_t size, HtmlStringStream* stream) {
	size_t totalSize = length * size;

	if (stream->length + totalSize >= stream->capacity) {
		size_t newCapacity = stream->capacity * 2 + totalSize;

		char* newBuffer = (char*)realloc(stream->buffer, newCapacity);
		HtmlHandleOutOfMemoryError(newBuffer, 0);

		stream->buffer = newBuffer;
		stream->capacity = newCapacity;
	}

	memcpy(stream->buffer + stream->length, content, totalSize);
	stream->length += totalSize;
	stream->buffer[stream->length] = 0; // Null-terminate the string

	return totalSize;
}



void HtmlLibDestroyStringStream(HtmlStringStream* stream) {
	if (stream != NULL) {
		free(stream->buffer);
		free(stream);
	}
}

HtmlStream HtmlCreateStreamString(size_t blockSize) {
	// create stream data
	HtmlStringStream* streamData = (HtmlStringStream*)malloc(sizeof(HtmlStringStream));
	if (streamData == NULL) {
		free(buffer);
		return (HtmlStream){0};
	}

	streamData->buffer = (char*)malloc(blockSize);
	HtmlHandleOutOfMemoryError(streamData->buffer, (HtmlStream){0});
	streamData->buffer[0] = 0;

	streamData->length = 0;
	streamData->capacity = blockSize;

	// create stream
	HtmlStream stream = HtmlCreateStreamEmpty();
	stream.data = streamData;
	stream.destroy = (HtmlCallbackDestroy)HtmlLibDestroyStringStream;

	stream.putchar = (HtmlCallbackPutchar)HtmlLibPutcharToStringStream;
	stream.write = (HtmlCallbackWrite)HtmlLibWriteStringStream;

	return stream;
}




// file stream

HtmlStream HtmlCreateStreamFileObject(FILE* file) {
	HtmlHandleNullError(file, HtmlCreateStreamEmpty());

	HtmlStream stream = HtmlCreateStream();
	stream->data = file;

	stream->putchar = fputc;
	stream->write = fwrite;
	stream->getchar = fgetc;
	stream->read = fread;
	stream->seek = fseek;
	return stream;
}

HtmlStream HtmlCreateStreamFile(const char* filename) {
	HtmlHandleEmptyStringError(filename, HtmlCreateStreamEmpty());

	FILE* file = fopen(filename, "r+");
	HtmlStream stream = HtmlCreateStreamFileObject(file);
	stream->destroy = fclose;

	return stream;
}





// get

const char* HtmlGetStreamString(HtmlStream* stream) {
	HtmlHandleNullError(stream, NULL);
	HtmlHandleError(stream->destroy, NULL, "not a string stream!");
	
	HtmlStringStream* streamData = (HtmlStringStream*)stream->data;
	streamData->buffer[streamData->length] = 0; // Ensure null-termination
	
	return streamData->buffer;
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
	HtmlSetText(object->name, name);
	
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
        HtmlDestroyPointer(attrName);
        HtmlDestroyPointer(attrValue);
        
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
	
	HtmlDestroyPointer(object->name);
	HtmlDestroyPointer(object->innerText);
	HtmlDestroyPointer(object->afterText);


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

HtmlCode HtmlGetText(HtmlObject* object, HtmlStream* stream) {
	// Validation Checks //

	// Check parameters
	HtmlHandleNullError(object, HTML_NULL_POINTER);
	HtmlHandleNullError(stream, HTML_NULL_POINTER);

	// Check a valid object type
	if (HtmlLibIsIntegerIn(object->type, HTML_TYPE_DOCUMENT, HTML_TYPE_TAG, -1) == false) {
		return HTML_OK; // No text to write for not text-containing types
	}

	// Check if stream is writeable
	HtmlHandleError(stream->write == NULL, HTML_STREAM_NOT_WRITEABLE,
		"%s: HtmlStream is not writeable, please setup the write callback function!", __func__);
	

	// Write text to stream //

	// Write innerText
	if (object->innerText) {
		size_t len = strlen(object->innerText);
		if (stream->write(object->innerText, len, 1, stream->data) != len) {
			return HTML_OUT_OF_MEMORY;
		}
	}

	// Write children's text
	HtmlObject* child;
	HtmlForeachObjectChildren(object, child) {
		// Skip writing afterText for COMMENT, DOCTYPE, and SCRIPT types
		if (HtmlLibIsIntegerIn(child->type, HTML_TYPE_COMMENT, HTML_TYPE_DOCTYPE, HTML_TYPE_SCRIPT, -1)) {
			continue;
		}

		// Special case (br or hr)
		if (strcmp(child->name, "br") == 0 || strcmp(child->name, "hr") == 0) {
			// Write a newline for <br> and <hr> tags
			if (stream->write("\n", 1, 1, stream->data) != 1) {
				return HTML_OUT_OF_MEMORY;
			}
			continue;
		}


		// Write child's text unless its type SINGLE
		if (child->type != HTML_TYPE_SINGLE) {
			HtmlCode code = HtmlGetText(child, stream);
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

const char* HtmlGetObjectAfterText(HtmlObject* object) {
	HtmlHandleNullError(object, NULL);
	return object->afterText ? object->afterText : "";
}

const char* HtmlGetObjectAttributeValue(HtmlObject* object, const char* attrName) {
	HtmlHandleNullError(object, NULL);
	HtmlHandleNullError(attrName, NULL);

	// Search for the attribute
	const char* _attrName, *_attrValue;
	HtmlForeachObjectAttributes(object, _attrName, _attrValue) {
		if (strcmp(attrName, _attrName) == 0) {
			return _attrValue;
		}
	}
	return NULL; // Attribute not found
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
			HtmlDestroyPointer(_attrName);
			HtmlDestroyPointer(_attrValue);
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




#endif // _MYHTML_H_ //