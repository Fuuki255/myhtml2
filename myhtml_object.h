#ifndef _MYHTML_OBJECT_H_
#define _MYHTML_OBJECT_H_

#define HTML_VERSION {2, 3, 0}

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
	HTML_FAILED,

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
typedef size_t (*HtmlCallbackWrite)(void* buf, size_t size, size_t n, void* streamData);

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
	if (p) {\
		free((void*)p);\
		p = NULL;\
	}

#define HtmlSetText(var, text) \
	HtmlLibDestroyPointer(var);\
	if (text) {\
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
	
	if (iter->now) {
		iter->next = iter->now->next;
        
        *attrName = iter->now->name;
        *attrValue = iter->now->value;
	}
	return iter->now;
}

bool HtmlPrevAttribute(HtmlAttributeIterator* iter, const char** attrName, const char** attrValue) {
	iter->next = iter->now;
	iter->now = iter->prev;
	
	if (iter->now) {
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
	
	if (iter->now) {
		iter->next = iter->now->next;
	}
	return iter->now;
}

HtmlObject* HtmlPrevObject(HtmlObjectIterator* iter) {
	iter->next = iter->now;
	iter->now = iter->prev;
	
	if (iter->now) {
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

#define HtmlIsStreamReadable(stream) (stream && stream->getchar && stream->read)
#define HtmlIsStreamWritable(stream) (stream && stream->putchar && stream->write)
#define HtmlIsStreamSeekable(stream) (stream && stream->seek)


#define HtmlGetcharFromStream(stream) \
	(stream && stream->getchar ? stream->getchar(stream->data) : EOF)

#define HtmlReadContentFromStream(buf, length, size, stream) \
	(stream && stream->read ? stream->read(buf, length, size, stream->data) : 0)

#define HtmlPutcharToStream(c, stream) \
	(stream && stream->putchar ? stream->putchar(c, stream->data) : EOF)

#define HtmlWriteContentToStream(content, length, size, stream) \
	(stream && stream->write ? stream->write(content, length, size, stream->data) : 0)


#define HtmlGetStreamPosition(stream) \
	(stream && stream->tell ? stream->tell(stream->data) : 0)



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

size_t HtmlLibWriteStreamString(void* content, size_t size, size_t n, HtmlStreamString* stream) {
	size_t total = n * size;

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
	if (streamData) {
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
HtmlStream HtmlCreateStreamString(const char* str) {
	HtmlHandleEmptyStringError(str, HtmlCreateStreamEmpty());

	// create streamData //
	HtmlStreamString* streamString = (HtmlStreamString*)malloc(sizeof(HtmlStreamString));
	HtmlHandleOutOfMemoryError(streamString, HtmlCreateStreamEmpty());

	// Initialize streamData
	streamString->buffer = (char*)str;
	streamString->position = 0;
	streamString->length = strlen(str);
	streamString->capacity = streamString->length + 1;

	HtmlStream stream = HtmlLibInitStreamString(streamString);
	stream.destroy = NULL;

	return stream;
}





// get

const char* HtmlGetStreamString(HtmlStream* stream) {
	HtmlHandleNullError(stream, "");
	HtmlHandleError((void*)stream->getchar != (void*)HtmlLibGetcharFromStreamString, "", "not a StreamString!");
	
	HtmlStreamString* streamString = (HtmlStreamString*)stream->data;
	streamString->buffer[streamString->length] = 0; // Ensure null-termination
	
	return streamString->buffer;
}

size_t HtmlGetStreamLength(HtmlStream* stream) {
	HtmlHandleNullError(stream, "");
	HtmlHandleError((void*)stream->getchar != (void*)HtmlLibGetcharFromStreamString, "", "not a StreamString!");

	return ((HtmlStreamString*)stream->data)->length;
}

HtmlCode HtmlClearStreamBuffer(HtmlStream* stream) {
	HtmlHandleNullError(stream, HTML_FAILED);
	HtmlHandleError((void*)stream->destroy != (void*)HtmlLibDestroyStringStream, HTML_FAILED, "not a StreamString!");	// identify StreamString and StreamBuffer

	HtmlStreamString* streamBuffer = (HtmlStreamString*)stream->data;
	streamBuffer->length = 0;
	return HTML_OK;
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

HtmlObject* HtmlLibAddObjectChild(HtmlObject* parent, HtmlObject* child);

HtmlObject* HtmlLibCreateObject(HtmlObjectType type, const char* name, HtmlObject* parent) {
	// assign memory
	HtmlObject* object = (HtmlObject*)calloc(1, sizeof(HtmlObject));
    HtmlHandleOutOfMemoryError(object, NULL);

	// set value
	object->type = type;
	HtmlLibSetTextLowered(&object->name, name);
	
	// set relationship
	if (parent) {
		HtmlLibAddObjectChild(parent, object);
	}
	return object;
}


HtmlObject* HtmlCreateObjectDocument() {
	return HtmlLibCreateObject(HTML_TYPE_DOCUMENT, NULL, NULL);
}
#define HtmlCreateObjectDocument HtmlCreateObjectDocument


HtmlObject* HtmlCreateObjectDoctype(HtmlObject* parent, const char* innerText) {
	HtmlHandleNullError(innerText, NULL);
	
	HtmlObject* doctype = HtmlLibCreateObject(HTML_TYPE_DOCTYPE, NULL, parent);
	HtmlSetText(doctype->innerText, innerText);
	return doctype;
}


HtmlObject* HtmlCreateObjectTag(HtmlObject* parent, const char* name) {
	HtmlHandleNullError(name, NULL);
	
	HtmlObject* tag = HtmlLibCreateObject(HTML_TYPE_TAG, name, parent);
	return tag;
}

/* EX版タグ作成 <tagName>innerText</tagName>

@param parent 親タグ
@param tagName タグ名
@param innerText 内部テキスト
@param afterText 次のテキストまでの隙間
*/
HtmlObject* HtmlCreateObjectTagEx(HtmlObject* parent, const char* tagName, const char* innerText, const char* afterText) {
	HtmlHandleNullError(tagName, NULL);
	
	HtmlObject* tag = HtmlLibCreateObject(HTML_TYPE_TAG, tagName, parent);
	HtmlSetText(tag->innerText, innerText);
	HtmlSetText(tag->afterText, afterText);
	return tag;
}


HtmlObject* HtmlCreateObjectSingle(HtmlObject* parent, const char* name) {
	HtmlHandleNullError(name, NULL);
	
	HtmlObject* singleTag = HtmlLibCreateObject(HTML_TYPE_SINGLE, name, parent);
	return singleTag;
}


// Create a HTML script
// input text to be script content, or NULL to create empty script
HtmlObject* HtmlCreateObjectScript(HtmlObject* parent, const char* content) {
	HtmlObject* script = HtmlLibCreateObject(HTML_TYPE_SCRIPT, "script", parent);
	HtmlSetText(script->innerText, content);
	return script;
}


HtmlObject* HtmlCreateObjectStyle(HtmlObject* parent, const char* content) {
	HtmlObject* script = HtmlLibCreateObject(HTML_TYPE_SCRIPT, "style", parent);
	HtmlSetText(script->innerText, content);
	return script;	
}


HtmlObject* HtmlCreateObjectComment(HtmlObject* parent, const char* text) {
	HtmlObject* comment = HtmlLibCreateObject(HTML_TYPE_COMMENT, NULL, parent);
	HtmlSetText(comment->innerText, text);
	return comment;
}


// A html document that having <html> <head> <meta> <title> and <body>
HtmlObject* HtmlCreateDocumentTemplate(const char* title) {
  HtmlObject* doc = HtmlCreateDocument();
  HtmlObject* tagHtml = HtmlCreateObjectTag(doc, "html");

  // head
  HtmlObject* tagHead = HtmlCreateObjectTag(tagHtml, "head");
  HtmlObject* tagMeta = HtmlCreateObjectSingle(tagHead, "meta");
  HtmlSetObjectAttrValue(tagMeta, "charset", "utf-8");

  HtmlCreateObjectTag(tagHead, "title", title, NULL);

  // body
  HtmlObject* tagBody = HtmlCreateObjectTag(tagBody, "body");

  return doc;
}




// Destroy

void HtmlLibClearObjectRelationship(HtmlObject* object) {
	if (object->parent == NULL) return;

	if (object->next) {
		object->next->prev = object->prev;
	}
	else {
		object->parent->lastChild = object->prev;
	}
	if (object->prev) {
		object->prev->next = object->next;
	}
	else {
		object->parent->firstChild = object->next;
	}
	object->parent = NULL;
}




void HtmlClearObjectAttributes(HtmlObject* object) {
	HtmlAttributeIterator iter = HtmlBeginAttribute(object);
    object->firstAttribute = NULL;
    object->lastAttribute = NULL;
	
    while ((iter.now = iter.next)) {
		if (iter.next) {
			iter.next = iter.next->next;
		}

        free(iter.now->name);
        free(iter.now->value);
        free(iter.now);
    }
}

void HtmlLibDestroyObject(HtmlObject* object);

void HtmlClearObjectChildren(HtmlObject* object) {
	HtmlObjectIterator iter = HtmlBeginObject(object);
	object->firstChild = NULL;
	object->lastChild = NULL;

    HtmlObject* child;
    while ((child = HtmlNextObject(&iter))) {
        HtmlLibDestroyObject(child);
    }
}


const char* HtmlGetObjectTypeString(HtmlObject* object);

void HtmlLibDestroyObject(HtmlObject* object) {
	// printf("Destroying object: %s (%s)\n", object->name ? object->name : "Unnamed", HtmlGetObjectTypeString(object));

	// Clear relationships
	HtmlLibClearObjectRelationship(object);

	// Clear children and attributes
	HtmlClearObjectChildren(object);
    HtmlClearObjectAttributes(object);
	
	free(object->name);
	free(object->innerText);
	free(object->afterText);

    free(object);
}

void HtmlDestroyObject(HtmlObject* object) {
	if (object == NULL) return;

    HtmlLibClearObjectRelationship(object);
	HtmlLibDestroyObject(object);
}




// Add

HtmlObject* HtmlLibAddObjectChild(HtmlObject* parent, HtmlObject* child) {
	// Edit child relationship setting
    child->parent = parent;
    child->prev = parent->lastChild;
    child->next = NULL;

	// Edit parent relationship setting
    if (parent->lastChild) {
        parent->lastChild->next = child;
    }
    if (parent->firstChild == NULL) {
        parent->firstChild = child;
    }
    parent->lastChild = child;
	return child;
}

HtmlObject* HtmlAddObjectChild(HtmlObject* parent, HtmlObject* child) {
	// Check invalid parameter
    HtmlHandleNullError(parent, NULL);
    HtmlHandleNullError(child, NULL);
	
	// clear old relationship
	HtmlLibClearObjectRelationship(child);
	return HtmlLibAddObjectChild(parent, child);
}

HtmlObject* HtmlInsertObjectChildBefore(HtmlObject* parent, HtmlObject* target, HtmlObject* object) {
	// Check invalid parameter
	HtmlHandleNullError(parent, NULL);
	HtmlHandleNullError(object, NULL);

	if (target == NULL) {
		target = parent->firstChild; // If target is NULL, insert before the first child
	}

	// Clear old relationship
	HtmlLibClearObjectRelationship(object);

	// Set new relationship
	object->parent = parent;
	object->next = target;

	if (target) {
		if (target->prev) {
			target->prev->next = object;
		}
		else {
		   parent->firstChild = object; // If no previous, set as first child
		}

		object->prev = target->prev;
		target->prev = object;
		return object;
	}

	// parent is empty, so that init parent children
	parent->firstChild = object;
	parent->lastChild = object;
	object->prev = NULL;
	return object;
}

HtmlObject* HtmlInsertObjectChildAfter(HtmlObject* parent, HtmlObject* target, HtmlObject* object) {
	// Check invalid parameter
	HtmlHandleNullError(parent, NULL);
	HtmlHandleNullError(object, NULL);

	if (target == NULL) {
		target = parent->lastChild; // If target is NULL, insert after the last child
	}

	// Clear old relationship
	HtmlLibClearObjectRelationship(object);

	// Set new relationship
	object->parent = parent;
	object->prev = target;

	if (target) {
		if (target->next) {
			target->next->prev = object;
		}
		else {
			parent->lastChild = object; // If no next, set as last child
		}

		object->next = target->next;
		target->next = object;
		return object;
	}

	// parent is empty, so that init parent children
	parent->firstChild = object;
	parent->lastChild = object;
	object->next = NULL;
	return object;
}



// Set

HtmlCode HtmlSetObjectInnerText(HtmlObject* object, const char* text) {
    HtmlHandleNullError(object, HTML_NULL_POINTER);

    HtmlSetText(object->innerText, text);
    return HTML_OK;
}


HtmlCode HtmlSetObjectAttrValue(HtmlObject* object, const char* attrName, const char* attrValue) {
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
    if (attrValue && attrValue[0] != '\0') {
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

#define HtmlSetObjectAttribute(object, name, attrValue) HtmlSetObjectAttrValue(object, name, attrValue)



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


/* 内部すべてのタグのテキストを取得 EX

object の下にあるすべてのタグからテキストを取得し、stream に書き込む

@param object HtmlObject to get text from
@param stream HtmlStream to write the text to
@return HTML_OK on success, or an error code on failure
*/
HtmlCode HtmlGetObjectTextEx(HtmlObject* object, HtmlStream* stream) {
	// checks parameter useable
	HtmlHandleNullError(object, HTML_NULL_POINTER);
	HtmlHandleNullError(stream, HTML_NULL_POINTER);

	// Check a valid object type
	if (HtmlLibIsIntegerIn(object->type, HTML_TYPE_DOCUMENT, HTML_TYPE_TAG, -1) == false) {
		return HTML_OK; // No text to write for not text-containing types
	}

	// Check if stream is writeable
	HtmlHandleError(HtmlIsStreamWritable(stream), HTML_STREAM_NOT_WRITEABLE,
		"HtmlStream is not writeable, please setup the write callback function!");
	
	return HtmlLibGetObjectText(object, stream);
}


// -- 内部すべてのタグのテキストを取得 --
// 新メモリを作るため、通常ではストリームを作成してそこに書くだが、
// それは EX 版に移し、メモリの余剰空間を使用する HtmlGetObjectText となった
// その空間の具体的な場所は `object->name + strlen(object->name) + 1`
const char* HtmlGetObjectText(HtmlObject* object) {
	// checks parameter useable
	HtmlHandleNullError(object, "");

	// Check a valid object type
	if (HtmlLibIsIntegerIn(object->type, HTML_TYPE_DOCUMENT, HTML_TYPE_TAG, -1) == false) {
		return ""; // No text to write for not text-containing types
	}

	// create StreamBuffer that write after object->name
    HtmlStream stream = HtmlCreateStreamBuffer(128);

    int resultOffset = 1;
    if (object->name) {
        resultOffset += strlen(object->name);
        stream.write(object->name, resultOffset, 1, stream.data);
    }
    else {
        stream.putchar('\0', stream.data);
    }

    // write string
    HtmlCode ret = HtmlLibGetObjectText(object, &stream);
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
	return object->name + resultOffset; // Return the name as the text content
}



const char* HtmlGetObjectAfterText(HtmlObject* object) {
	HtmlHandleNullError(object, NULL);
	return object->afterText ? object->afterText : "";
}

const char* HtmlGetObjectAttrValue(HtmlObject* object, const char* attrName) {
	HtmlHandleNullError(object, NULL);
	HtmlHandleEmptyStringError(attrName, NULL);

	// Search for the attribute
	const char* _attrName, *_attrValue;
	HtmlForeachObjectAttributes(object, _attrName, _attrValue) {
		if (strcmp(attrName, _attrName) == 0) {
			return _attrValue;
		}
	}
	return NULL; // Attribute not found
}
#define HtmlGetObjectAttributeValue(object, attrName) HtmlGetObjectAttrValue(object, attrName)


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
	HtmlHandleNullError(object, "(null)");
	
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

void HtmlRemoveObjectAttribute(HtmlObject* object, const char* attrName) {
	HtmlHandleNullError(object, );
	HtmlHandleNullError(attrName, );

	// Search for the attribute
	HtmlAttributeIterator iter = HtmlBeginAttribute(object);
	
	while ((iter.now = iter.next)) {
		if (iter.next) {
			iter.next = iter.next->next;
		}
		if (strcmp(attrName, iter.now->name) != 0) {
			continue;
		}

		// Remove relationships
		if (iter.now->prev) {
			iter.now->prev->next = iter.now->next;
		}
		else {
			object->firstAttribute = iter.next;
		}
		if (iter.next) {
			iter.next->prev = iter.now->prev;
		}
		else {
			object->lastAttribute = iter.now->prev;
		}

		// Free the attribute memory
		free(iter.now->name);
		free(iter.now->value);
		free(iter.now);
		return;
	}
	return;
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
		HtmlLibAddObjectChild(copy, childCopy);
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


#endif // _MYHTML_OBJECT_H_ //
