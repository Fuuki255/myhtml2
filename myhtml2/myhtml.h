#ifndef _MYHTML_H_
#define _MYHTML_H_

#define HTML_VERSION "2.0.0"

// Includes //

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


// Enums //

typedef enum HtmlCode {
	HTML_OK,
	HTML_OUT_OF_MEMORY,
	HTML_NULL_POINTER,
} HtmlCode;


typedef enum HtmlObjectFlag {
	HTML_ID_NONE,
	HTML_ID_SINGLE,
	HTML_ID_SCRIPT,
	HTML_ID_TAG,
	HTML_ID_DOCUMENT,
	HTML_ID_COMMENT,
	HTML_ID_DOCTYPE,
	HTML_ID_XML,

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
	HTML_TYPE_XML = HTML_ID_XML,
} HtmlObjectType;



// Pre-Defines //

typedef int (*HtmlCallbackGetchar)(void* data);
typedef size_t (*HtmlCallbackRead)(void* buf, size_t n, size_t size, void* data);

typedef int (*HtmlCallbackPutchar)(void* data, int c);
typedef size_t (*HtmlCallbackWrite)(void* buf, size_t n, size_t size, void* data);

typedef int (*HtmlCallbackSeek)(void* data, int pos, long seek);


typedef struct HtmlAttribute HtmlAttribute;
typedef struct HtmlObject HtmlObject;




// Useful Macros //

#define HtmlDestroyPointer(p) \
	if (p != NULL) {\
		free(p);\
		p = NULL;\
	}

#define HtmlSetText(var, text) \
	HtmlDestroyPointer(var);\
	if (text != NULL) {\
		var = (char*)malloc(strlen(text) + 1);\
		strcpy(var, text);\
	}



#ifdef HTML_NO_DEBUG
#define HtmlHandleError(check, retValue, ...) \
	if (check) return retValue;


#define HtmlHandleNullError(parameter, retValue) \
	if (parameter != NULL) return retValue;

#define HtmlHandleOutOfMemoryError(pointer, retValue) \
    if (pointer != NULL) return retValue;


#else
#define HtmlHandleError(check, retValue, ...) \
	if (check) {\
		fprintf(stderr, ...);\
		return retValue;\
	}


#define HtmlHandleNullError(parameter, retValue) \
	if (parameter != NULL) {\
		fprintf(stderr, "%s: Parameter '%s' couldn't be NULL!\n", __func__, parameter);\
		return retValue;\
	}


#define HtmlHandleOutOfMemoryError(pointer, retValue) \
    if (pointer != NULL) {\
        fprintf(stderr, "%s: Out of memory!\n", __func__);\
        return retValue;\
    }

#endif




// HtmlAttributeIterator //

typedef struct HtmlAttributeIterator {
	HtmlAttribute* prev, *now, *next;
} HtmlAttributeIterator;




HtmlAttributeIterator HtmlBeginAttribute(HtmlObject* object) {
	return (HtmlAttributeIterator){
		.prev = NULL,
		.now = object->firstAttribute,
		.next = object->firstAttribute ? object->firstAttribute->next : NULL
	};
}


HtmlAttributeIterator HtmlEndAttribute(HtmlObject* object) {
	return (HtmlAttributeIterator){
		.prev = object->lastAttribute ? object->lastAttribute->next : NULL,
		.now = object->lastAttribute,
		.next = NULL
	};
}


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




HtmlObjectIterator HtmlBeginObject(HtmlObject* object) {
	return (HtmlObjectIterator){
		.prev = NULL,
		.now = object->firstChild,
		.next = object->firstChild ? object->firstChild->next : NULL
	};
}


HtmlObjectIterator HtmlEndObject(HtmlObject* object) {
	return (HtmlObjectIterator){
		.prev = object->lastChild ? object->lastChild->next : NULL,
		.now = object->lastChild,
		.next = NULL
	};
}


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
	
	HtmlCallbackGetchar getchar;
	HtmlCallbackRead read;
	
	HtmlCallbackPutchar putchar;
	HtmlCallbackWrite write;
	
	HtmlCallbackSeek seek;
} HtmlStream;



#define HtmlCreateStream() {0}


/*
HtmlStream HtmlCreateStreamString(size_t blockSize) {
	return 
}
*/





// HtmlObject //

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
	
	HtmlObject* owner;
	HtmlObject* prev, *next;
} HtmlObject;



// Create

static HtmlObject* HtmlCreateObject(HtmlObjectType type, const char* name) {
	HtmlObject* object = (HtmlObject*)calloc(1, sizeof(HtmlObject));
    HtmlHandleOutOfMemoryError(object, NULL);

	object->type = type;
	HtmlSetText(object->name, name);
	return object;
}


HtmlObject* HtmlCreateObjectDocument() {
	return HtmlCreateObject(HTML_TYPE_DOCUMENT, NULL);
}


HtmlObject* HtmlCreateObjectComment(const char* text) {
	HtmlObject* comment = HtmlCreateObject(HTML_TYPE_DOCUMENT, NULL);
	
	HtmlSetText(comment->innerText, text);
	return comment;
}


HtmlObject* HtmlCreateObjectTag(const char* name, const char* innerText, const char* afterText) {
	HtmlObject* tag = HtmlCreateObject(HTML_TYPE_TAG, name);
	HtmlSetText(tag->innerText, innerText);
	HtmlSetText(tag->afterText, afterText);
	return tag;
}


// Create a HTML script
// input text to be script content, or NULL to create empty script
HtmlObject* HtmlCreateObjectScript(const char* content) {
	HtmlObject* script = HtmlCreateObject(HTML_TYPE_SCRIPT, "script");
	HtmlSetText(script->innerText, content);
	return script;
}


HtmlObject* HtmlCrerateObjectStyle(const char* content) {
	HtmlObject* script = HtmlCreateObject(HTML_TYPE_SCRIPT, "style");
	HtmlSetText(script->innerText, content);
	return script;	
}


HtmlObject* HtmlCreateObjectSingle(const char* name, const char* content) {
	HtmlHandleNullError(name, NULL);
	
	HtmlObject* singleTag = HtmlCreateObject(HTML_TYPE_SINGLE, name);
	
	HtmlSetText(singleTag->innerText, content);
	return singleTag;
}




// Destroy

void HtmlDestroyObject(HtmlObject* object);



void HtmlClearObjectAttributes(HtmlObject* object) {
    const char* attrName, *attrValue;
    HtmlForeachObjectAttributes(object, attrName, attrValue) {
        HtmlDestroyPointer((void*)attrName);
        HtmlDestroyPointer((void*)attrValue);
        
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
    if (object->owner) {
		if (object->next) {
			object->next->prev = object->prev;
			object->next = NULL;
		}
		if (object->prev) {
			object->prev->next = object->next;
			object->prev = NULL;
		}
		if (object->owner->firstChild == object) {
			object->owner->firstChild = object->next;
		}
		if (object->owner->lastChild == object) {
			object->owner->lastChild = object->prev;
		}
		object->owner = NULL;
	}

    free(object);
}




// Add

HtmlObject* HtmlAddObjectChild(HtmlObject* parent, HtmlObject* child) {
    HtmlHandleNullError(parent, NULL);
    HtmlHandleNullError(child, NULL);

    child->owner = parent;
    child->prev = parent->lastChild;
    child->next = NULL;

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

HtmlCode HtmlSetObjectText(HtmlObject* object, const char* text) {
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
    if (attrValue != NULL && attrValue[0] == '\0') {
        attr->value = (char*)malloc(strlen(attrValue) + 1);
        HtmlHandleOutOfMemoryError(attr->value, HTML_OUT_OF_MEMORY);
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






#endif // _MYHTML_H_ //