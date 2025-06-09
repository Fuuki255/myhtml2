#ifndef MYHTML_METHODS_H
#define MYHTML_METHODS_H

#define MYHTML_METHODS_VERSION {1, 0, 0}


#ifndef MYHTML_DEFINES_H
#include "MyHtml_defines.h"
#endif


/*** Basic ***/

// This method creating new memory copy from exists string
char* HTML_CopyLoweredString(const char* str) {
	if (!str) {
		return NULL;
	}

	size_t length = strlen(str);
	char* dest = (char*)malloc(length + 1);
	HTML_Error_NoEnoughMemory(dest, "Failed to copy string", NULL);

	for (size_t i = 0; i < length; i++) {
		int c = str[i];
		if (c >= 65 && c <= 90) {
			c += 32;
		}
		dest[i] = c;
	}
	dest[length] = 0;
	return dest;
}

// This method creating new memory copy from exists string
char* HTML_CopyString(const char* str) {
	if (!str) {
		return NULL;
	}

	size_t len = strlen(str) + 1;
	char* dest = (char*)malloc(len);
	HTML_Error_NoEnoughMemory(dest, "Failed to copy string!", NULL);

	HTML_strcpy_s(dest, len, str);
	return dest;
}




/*** HTML_Element ***/

// Create

#define _HTML_CreateElement(element, eleType)	HTML_Element* element = (HTML_Element*)calloc(1, sizeof(HTML_Element)); HTML_Error_NoEnoughMemory(element, "Failed to create element!", NULL); element->type = eleType;

extern inline HTML_Element* HTML_CreateDocument() {
	_HTML_CreateElement(element, HTML_DOCUMENT);
	return element;
}

extern inline HTML_Element* HTML_CreateTag(const char* tagName, const char* text) {
	_HTML_CreateElement(element, HTML_TAG);
	element->name = HTML_CopyLoweredString(tagName);

	if (text) {
		element->text = HTML_CopyString(text);
	}
	return element;
}

extern inline HTML_Element* HTML_CreateSingle(const char* tagName) {
	_HTML_CreateElement(element, HTML_TAG);
	element->name = HTML_CopyLoweredString(tagName);
	return element;
}

extern inline HTML_Element* HTML_CreateComment(const char* comment) {
	_HTML_CreateElement(element, HTML_COMMENT);
	element->text = HTML_CopyString(comment);
	return element;
}




// Foreach

#define HTML_ForeachElementAttributes(element, attr)	for (HTML_Attribute* attr = element->firstAttr, * attr##Front = attr ? attr->front : NULL; attr; attr = attr##Front, attr##Front = attr ? attr->front : NULL)
#define HTML_ForeachElementChildren(element, child)		for (HTML_Element* child = element->firstChild, * child##Front = child ? child->front : NULL; child; child = child##Front, child##Front = child ? child->front : NULL)



// Destroy

extern inline void HTML_DestroyElementAttributes(HTML_Element* element) {
	HTML_ForeachElementAttributes(element, attr) {
		free(attr->name);
		if (attr->value) {
			free(attr->value);
		}
		free(attr);
	}
	element->firstAttr = element->lastAttr = NULL;
}

void HTML_DestroyElement(HTML_Element* element);

extern inline void HTML_DestroyElementChildren(HTML_Element* element) {
	HTML_ForeachElementChildren(element, child) {
		HTML_DestroyElement(child);
	}
}


void HTML_ClearElementRelationship(HTML_Element* element) {
	if (element->owner) {
		if (element->front) {
			element->front->back = element->back;
			element->front = NULL;
		}
		if (element->back) {
			element->back->front = element->front;
			element->back = NULL;
		}
		if (element->owner->firstChild == element) {
			element->owner->firstChild = element->front;
		}
		if (element->owner->lastChild == element) {
			element->owner->lastChild = element->back;
		}
		element->owner = NULL;
	}
}



void HTML_DestroyElement(HTML_Element* element) {
	// Destroy name, text and interval
	if (element->name) {
		free(element->name);
	}
	if (element->text) {
		free(element->text);
	}
	if (element->interval) {
		free(element->interval);
	}

	// Destroy attribute and children
	HTML_DestroyElementAttributes(element);
	HTML_DestroyElementChildren(element);

	// Clear relationship
	HTML_ClearElementRelationship(element);

	// Destroy self
	free(element);
}





// Get

const char* HTML_GetElementTagName(HTML_Element* element) {
	return element->name ? element->name : "";
}

const char* HTML_GetElementString(HTML_Element* element) {
	return element->text ? element->text : "";
}

const char* HTML_GetElementInterval(HTML_Element* element) {
	return element->interval ? element->interval : "";
}



const HTML_Attribute* HTML_GetElementAttribute(HTML_Element* element, const char* attrName) {
	HTML_ForeachElementAttributes(element, attr) {
		if (strcmp(attr->name, attrName) == 0) {
			return attr;
		}
	}
	return NULL;
}



// Set

inline const char* HTML_SetElementTagName(HTML_Element* element, const char* tagName) {
	if (element->name) {
		free(element->name);
	}

	if (tagName) {
		return element->name = HTML_CopyLoweredString(tagName);
	}
	return element->name = NULL;
}

extern inline const char* HTML_SetElementString(HTML_Element* element, const char* text) {
	if (element->text) {
		free(element->text);
	}

	if (text) {
		return element->text = HTML_CopyString(text);
	}
	return element->text = NULL;
}



void HTML_SetElementAttribute(HTML_Element* element, const char* attrName, const char* attrValue) {
	// find attr
	HTML_Attribute* attr = (HTML_Attribute*)HTML_GetElementAttribute(element, attrName);
	
	// attr found: Recycle value
	if (attr) {
		free(attr->value);
	}
	// attr not found: Create Attr
	else {
		// Create object
		attr = (HTML_Attribute*)malloc(sizeof(HTML_Attribute));
		HTML_Error_NoEnoughMemory(attr, "Failed to create attribute!", );

		attr->name = HTML_CopyLoweredString(attrName);

		// Setup relationship
		attr->front = NULL;
		attr->back = element->lastAttr;

		if (element->lastAttr) {
			element->lastAttr->front = attr;
		}
		if (!element->firstAttr) {
			element->firstAttr = attr;
		}
		element->lastAttr = attr;
	}

	// Set value
	attr->value = HTML_CopyString(attrValue);
}



// Add

HTML_Element* HTML_AddElementChild(HTML_Element* element, HTML_Element* child) {
	HTML_Error_NullParam(element, child);
	HTML_Error_NullParam(child, NULL);

	child->owner = element;
	child->front = NULL;
	child->back = element->lastChild;

	if (element->lastChild) {
		element->lastChild->front = child;
	}
	if (!element->firstChild) {
		element->firstChild = child;
	}
	element->lastChild = child;
	return child;
}





// Remove

void HTML_RemoveElementAttribute(HTML_Element* element, const char* attrName) {
	HTML_Attribute* attr = (HTML_Attribute*)HTML_GetElementAttribute(element, attrName);
	if (!attr) {
		return;
	}

	// Destroy data
	free(attr->name);
	free(attr->value);

	// Destroy attribute relationship
	if (attr->front) {
		attr->front->back = attr->back;
	}
	if (attr->back) {
		attr->back->front = attr->front;
	}
	if (element->firstAttr == attr) {
		element->firstAttr = attr->front;
	}
	if (element->lastAttr == attr) {
		element->lastAttr = attr->back;
	}
	free(attr);
}






#endif /* MYHTML_METHODS_H */