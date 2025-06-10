# My HTML Parser (myhtml)

| Item       | Value     |
| :--------- | :-------- |
| Langauge   | C Program |
| Version    | 1.0.0     |
| Programmer | Fuuki-255 |

## Types

**HtmlCode**

```c
typedef enum HtmlCode {
    HtmlOK,
    HtmlOUT_OF_MEMORY,
} HtmlCode;
```

**HtmlObjectFlag**

```c
typedef enum HtmlObjectFlag {
	HtmlID_NONE,
	HtmlID_SINGLE,
	HtmlID_SCRIPT,
	HtmlID_TAG,
	HtmlID_DOCUMENT,
	HtmlID_COMMENT,
	HtmlID_DOCTYPE,
	HtmlID_XML,

	HtmlHAS_NAME = 0x10,
	HtmlHAS_ATTR = 0x20,
	HtmlHAS_TEXT = 0x40,
	HtmlHAS_CHILD = 0x80,
} HtmlObjectFlag;
```

**HtmlObjectType**

```c
typedef enum HtmlObjectType {
	HtmlNONE,
	HtmlTYPE_SINGLE = HtmlID_SINGLE | HtmlHAS_NAME | HtmlHAS_ATTR,
	HtmlTYPE_SCRIPT = HtmlID_SCRIPT | HtmlHAS_NAME | HtmlHAS_ATTR | HtmlHAS_TEXT,
	HtmlTYPE_TAG = HtmlID_TAG | HtmlHAS_NAME | HtmlHAS_ATTR | HtmlHAS_TEXT | HtmlHAS_CHILD,
	HtmlTYPE_DOCUMENT = HtmlID_DOCUMENT | HtmlHAS_TEXT,
	HtmlTYPE_COMMENT = HtmlID_COMMENT | HtmlHAS_TEXT,
	HtmlTYPE_DOCTYPE = HtmlID_DOCTYPE,
} HtmlObjectType;
```

**HtmlObject**

```c
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
```

**HtmlAttribute**

```c
// Object attribute inside, not providing edit to user
typedef struct HtmlAttribute {
    char* name;
    char* value;
    HtmlAttribute* front, *back;
} HtmlAttribute;
```

**HtmlStream**

```c
typedef struct HtmlStream {
    // ...
} HtmlStream;
```

## Methods (methods.h)

**HtmlCreate`<type>`()**

- HtmlObject* HtmlCreateDocument()
- HtmlObject* HtmlCreateTag(const char* name, const char* text)
- HtmlObject* HtmlCreateSingle(const char* name)
- HtmlObject* HtmlCreateScript()
- HtmlObject* HtmlCreateStyle()

**HtmlDestroy`<type>`()**

- void HtmlDestroyObject(HtmlObject* object)
- void HtmlClearChildren(HtmlObject* object)

**HtmlAddObject`<item>`()**

- HtmlObject* HtmlAddObjectChild(HtmlObject* parent, HtmlObject* child)

**HtmlSetObject`<item>`()**

- HtmlCode HtmlSetObjectText(HtmlObject* object, const char* text)
- HtmlCode HtmlSetObjectAttribute(HtmlObject* object, const char* attrName, const char* attrValue)

**HtmlGetObject`<item>`()**

- const char* HtmlGetObjectName(HtmlObject* object)

// Return text by only this object, better not edit its return

- const char* HtmlGetObjectString(HtmlObject* object)

// Write object's text within its children

- HtmlCode HtmlGetObjectText(HtmlObject* object, HtmlStream* stringStream)
- const char* HtmlGetObjectInterval(HtmlObject* object)
- const char* HtmlGetObjectAttributeValue(HtmlObject* object, const char* attrName)
- HtmlObject* HtmlGetObjectFirstChild(HtmlObject* object)
- HtmlObject* HtmlGetObjectLastChild(HtmlObject* object)
- HtmlObject* HtmlGetObjectParent(HtmlObject* object)

**HtmlRemoveObject`<item>`()**

- HtmlCode HtmlRemoveObjectAttribute(HtmlObject* object, const char* attrName)


**Attributes Foreach Methods**

- HtmlAttributeIterator HtmlBeginAttribute(HtmlObject* object)
- HtmlAttributeIterator HtmlEndAttribute(HtmlObject* object)

- HtmlAttribute* HtmlPrevAttribute(HtmlAttributeIterator* iterator)
- HtmlAttribute* HtmlNextAttribute(HtmlAttributeIterator* iterator)

- HtmlForeachObjectAttribute(HtmlObject* object, const char* attrName, const char* attrValue) {}


**Children Foreach Methods**

- HtmlObjectIterator HtmlBeginObject(HtmlObject* object)
- HtmlObjectIterator HtmlEndObject(HtmlObject* object)

- HtmlObject* HtmlPrevObject(HtmlObjectIterator* iterator)
- HtmlObject* HtmlNextObject(HtmlObjectIterator* iterator)

- HtmlForeachObjectChild(HtmlObject* object, HtmlObject* value) {}

## Methods (reader.h)

**HtmlRead`<inputTypes>`()**

- HtmlObject* HtmlReadStream(HtmlStream* stream)
- HtmlObject* HtmlReadString(const char* html)
- HtmlObject* HtmlReadFileObject(FILE* file)
- HtmlObject* HtmlReadFile(const char* filename)

## Methods (writer.h)

**HtmlWrite`<outputTypes>`()**

- HtmlCode HtmlWriteStream(HtmlObject* object, HtmlStream* stream)
- HtmlCode HtmlWriteStringStream(HtmlObject* object, HtmlStream* stringStream)
- HtmlCode HtmlWriteFileObject(HtmlObject* object, FILE* file)
- HtmlCode HtmlWriteFile(HtmlObject* object, const char* file)

## Methods (search.h)

**HtmlObjectArray* HtmlSearchObject(HtmlObject* object, const char* words, int limit)**

**HtmlObject* HtmlFindObject(HtmlObject* object, const char* words)**



