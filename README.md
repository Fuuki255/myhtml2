# My HTML Parser (myhtml)

| Item          | Value         |
|:----------    |:--------------|
| Langauge      | C Program     |
| Version       | 1.0.0         |
| Programmer    | Fuuki-255     |


## Types

**HTML_Code**

```c
typedef enum HTML_Code {
    HTML_OK,
    HTML_OUT_OF_MEMORY,
} HTML_Code;
```


**HTML_ObjectFlag**

```c
typedef enum HTML_ObjectFlag {
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
} HTML_ObjectFlag;
```


**HTML_ObjectType**

```c
typedef enum HTML_ObjectType {
	HTML_NONE,
	HTML_OBJTYPE_SINGLE = HTML_ID_SINGLE | HTML_HAS_NAME | HTML_HAS_ATTR,
	HTML_OBJTYPE_SCRIPT = HTML_ID_SCRIPT | HTML_HAS_NAME | HTML_HAS_ATTR | HTML_HAS_TEXT,
	HTML_OBJTYPE_TAG = HTML_ID_TAG | HTML_HAS_NAME | HTML_HAS_ATTR | HTML_HAS_TEXT | HTML_HAS_CHILD,
	HTML_OBJTYPE_DOCUMENT = HTML_ID_DOCUMENT | HTML_HAS_TEXT,
	HTML_OBJTYPE_COMMENT = HTML_ID_COMMENT | HTML_HAS_TEXT,
	HTML_OBJTYPE_DOCTYPE = HTML_ID_DOCTYPE,
	HTML_OBJTYPE_XML = HTML_ID_XML,
} HTML_ObjectType;
```

**HTML_Object**

```c
typedef struct HTML_Object {
    HTML_ObjectType type;

    char* name;
    char* text;
    char* interval;

    HTML_Object* firstChild, *lastChild;
    HTML_Attribute* firstAttribute, *lastAttribute;

    HTML_Object* owner;
    HTML_Object* front, *back;
} HTML_Object;
```

**HTML_Attribute**

```c
// Object attribute inside, not providing edit to user
typedef struct HTML_Attribute {
    char* name;
    char* value;
    HTML_Attribute* front, *back;
} HTML_Attribute;
```


**HTML_Stream**

```c
typedef struct HTML_Stream {
    // ...
} HTML_Stream;
```



## Methods (methods.h)

**HTML_Create<type>()**

- HTML_Object* HTML_CreateDocument()

- HTML_Object* HTML_CreateTag(const char* name, const char* text)

- HTML_Object* HTML_CreateSingle(const char* name)

- HTML_Object* HTML_CreateScript()

- HTML_Object* HTML_CreateStyle()


**HTML_Destroy<type>()**

- void HTML_DestroyObject(HTML_Object* object)


**HTML_AddObject<item>()**

- HTML_Object* HTML_AddObjectChild(HTML_Object* parent, HTML_Object* child)


**HTML_SetObject<item>()**

- HTML_Code HTML_SetObjectText(HTML_Object* object, const char* text)

- HTML_Code HTML_SetObjectAttribute(HTML_Object* object, const char* attrName, const char* attrValue)


**HTML_GetObject<item>()**

- const char* HTML_GetObjectName(HTML_Object* object)

// Return text by only this object, better not edit its return
- const char* HTML_GetObjectString(HTML_Object* object)

// Write object's text within its children
- HTML_Code HTML_GetObjectText(HTML_Object* object, HTML_Stream* stringStream)

- const char* HTML_GetObjectInterval(HTML_Object* object)

- const char* HTML_GetObjectAttributeValue(HTML_Object* object, const char* attrName)

- HTML_Object* HTML_GetObjectFirstChild(HTML_Object* object)

- HTML_Object* HTML_GetObjectLastChild(HTML_Object* object)

- HTML_Object* HTML_GetObjectParent(HTML_Object* object)


**HTML_RemoveObject<item>()**

- HTML_Code HTML_RemoveObjectAttribute(HTML_Object* object, const char* attrName)


**Foreach Methods**

- HTML_Object* HTML_NextObject(HTML_Object* object)

- HTML_Object* HTML_PrevObject(HTML_object* object)

- HTML_ForeachObjectChild(HTML_Object* object, HTML_Object* value) {}

- HTML_ForeachObjectAttribute(HTML_Object* object, const char* attrName, const char* attrValue) {}



## Methods (reader.h)

**HTML_Read<inputTypes>()**

- HTML_Object* HTML_ReadStream(HTML_Stream* stream)

- HTML_Object* HTML_ReadString(const char* html)

- HTML_Object* HTML_ReadFileObject(FILE* file)

- HTML_Object* HTML_ReadFile(const char* filename)



## Methods (writer.h)

**HTML_Write<outputTypes>()**

- HTML_Code HTML_WriteStream(HTML_Object* object, HTML_Stream* stream)

- HTML_Code HTML_WriteStringStream(HTML_Object* object, HTML_Stream* stringStream)

- HTML_Code HTML_WriteFileObject(HTML_Object* object, FILE* file)

- HTML_Code HTML_WriteFile(HTML_Object* object, const char* file)



## Methods (search.h)

**HTML_ObjectArray* HTML_SearchObject(HTML_Object* object, const char* words, int limit)**

**HTML_Object* HTML_FindObject(HTML_Object* object, const char* words)**

