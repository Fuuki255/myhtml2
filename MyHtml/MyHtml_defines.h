#ifndef MYHTML_DEFINES_H
#define MYHTML_DEFINES_H

#define MYHTML_DEFINES_VERSION {1, 0, 0}


/*** Basic Libraries ***/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


/*** Pre-Processer ***/

#define HTML_Error_NoEnoughMemory(var, msg, retValue)	if (!var) { fprintf(stderr, "ERROR: %s: " msg " (No enough memory)\n", __func__); return retValue; }
#define HTML_Error_NullParam(var, retValue)				if (!var) { fprintf(stderr, "ERROR: %s: Param '" #var "' Couldn't be NULL!\n", __func__); return retValue; }

#ifdef _MSC_VER

#define HTML_strcpy_s(dest, size, src)	strcpy_s(dest, size, src)

#else

#define HTML_strcpy_s(dest, size, src)	strcpy(dest, src)

#endif

#define HTML_Type_HasFlags(type, flags)		((type & flags) == flags)




/*** Enums ***/

typedef enum HTML_TypeFlags {
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
} HTML_TypeFlags;

typedef enum HTML_Type {
	HTML_NONE,
	HTML_SINGLE = HTML_ID_SINGLE | HTML_HAS_NAME | HTML_HAS_ATTR,
	HTML_SCRIPT = HTML_ID_SCRIPT | HTML_HAS_NAME | HTML_HAS_ATTR | HTML_HAS_TEXT,
	HTML_TAG = HTML_ID_TAG | HTML_HAS_NAME | HTML_HAS_ATTR | HTML_HAS_TEXT | HTML_HAS_CHILD,
	HTML_DOCUMENT = HTML_ID_DOCUMENT | HTML_HAS_TEXT,
	HTML_COMMENT = HTML_ID_COMMENT | HTML_HAS_TEXT,
	HTML_DOCTYPE = HTML_ID_DOCTYPE,
	HTML_XML = HTML_ID_XML,
} HTML_Type;



/*** Type Defines ***/

typedef struct HTML_Attribute HTML_Attribute;
typedef struct HTML_Element HTML_Element;



/*** Structures ***/

typedef struct HTML_Version {
	unsigned short major;
	unsigned short minor;
	unsigned short patch;
} HTML_Version;


// HTML 属性
typedef struct HTML_Attribute {
	char* name;			// 属性名前
	char* value;		// 属性データ

    HTML_Attribute* front;
    HTML_Attribute* back;
} HTML_Attribute;


// HTML 要素
typedef struct HTML_Element {
	char* name;			// 要素名前
	char* text;			// 要素に保存された文字列データ
	char* interval;		// 次の要素との間のテキスト

    HTML_Element* firstChild, *lastChild;       	// 子ども要素
    HTML_Attribute* firstAttr, *lastAttr;   	// 属性

    HTML_Type type;     // 要素タイプ

    // オブジェクト関係
    HTML_Element* owner;
    HTML_Element* front;
    HTML_Element* back;
} HTML_Element;






#endif /* MYHTML_DEFINES_H */