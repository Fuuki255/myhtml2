# My Html Parser (myhtml2)

## Information

| Item       | Value     |
| :--------- | :-------- |
| Langauge   | C Program |
| Version    | 2.0.0     |
| Programmer | Fuuki-255 |

## How to use

To use myhtml2, you should use `HtmlReadObjectFrom<type>()`, these function can read html to  `HtmlObject`, it will be html objects in DOCUMENT object.

Than, there are some methods to find tags we need. `HtmlGetObjectChild(htmlObject, patterns)` is a method to get first object that suits to the patterns (it work like Python `BeautifulSoup`). To read multi-object, You can use `HtmlSearchObject(htmlObject, pattern)` to get all object suit the patterns to `HtmlObjectArray`.

After we have the tags we needed, we can use some `get` functions to get we needs. For example:

- `HtmlGetObjectAttributeValue(htmlObject, attributeName)` return attribute value
- `HtmlGetObjectInnerText(HtmlObject)` return text it have (no including its children, you should use `HtmlGetObjectText`)
- HtmlGetObjectText(HtmlObject, stringStream)`

You can edit the html by `HtmlSetObject<type>()` functions, 

To write html to file or string, you can use `HtmlWriteObjectTo<type>()`.

Final, destroy `HtmlObject*`, `HtmlStream*` or `HtmlArray*` by `HtmlDestroy<type>()` functions.

## Sample

This is C program sample that it would show how to use myhtml2 to handle html document (HtmlReadCurl will only compile when LibCurl existed)

```c
#include <curl/curl.h>        // libcurl should include before myhtml2, otherwise the HtmlReadCurl will not compile
#include "myhtml2/myhtml.h"

int main(int argc, char** argv) {
  /* init curl and read html from url */

  // init curl
  CURL* curl = curl_easy_init();

  // a libcurl exten function that read html from website
  // function will only set URL, WRITEDATA, WRITEFUNCTION options.
  HtmlObject* html = HtmlReadCurl(curl, "http://www.example.com/");


  /* print html title */

  // getting first <title> from document, don't need the full path.
  // You can replace HtmlGetObjectInnerText(node) by node->innerText, but using get function will more safety.
  HtmlObject* node = htmlGetObjectChild(html, "title");
  printf("html title: %s\n", HtmlGetObjectInnerText(node));


  /* print document content */

  // get html.body from html
  node = HtmlGetObjectChild(html, "body");

  // create string stream and get body content
  HtmlStream stream = HtmlCreateStreamString(64 /* buffer size */);
  HtmlGetObjectText(node, &stream);

  // htmlGetStreamString() only for the stream type as string
  printf("content:\n%s\n", htmlGetStreamString(&stream));


  /* cleanup */

  // destroy stream and html
  HtmlDestroyStream(stream);
  HtmlDestroyObject(html);

  // cleanup curl
  curl_easy_cleanup(curl);

  return 0;
}

```

## Handle Error

`myhtml2` always print error message when the error raise, e.g. "HtmlReadString: Out of memory!", than returning NULL or not HTML_OK.

You can disable printing message by HTML_NO_DEBUG, than the message will not print but the error still handling.

## Structures

**HtmlCode**

```c
typedef enum HtmlCode {
    HTML_OK,
    HTML_OUT_OF_MEMORY,
} HtmlCode;
```

**HtmlObjectFlag**

```c
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
```

**HtmlObjectType**

| Type     | Has Name | Has Attribute | Has InnerText | Has Children |
| :------- | :------- | :------------ | :------------ | :----------- |
| NONE     | false    | false         | false         | false        |
| SINGLE   | true     | true          | false         | false        |
| SCRIPT   | true     | true          | true          | false        |
| TAG      | true     | true          | true          | true         |
| DOCUMENT | false    | false         | false         | true         |
| COMMENT  | false    | false         | true          | false        |
| DOCTYPE  | false    | false         | true          | false        |

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

`HtmlAttribute` is not provided to edit but there are completed handle in HtmlObject methods.

**HtmlStream**

```c
typedef struct HtmlStream {
  void* data; // customizable pointer to read or write

  // operations same to c std
  HtmlCallbackGetchar getchar;
  HtmlCallbackRead read;
  HtmlCallbackPutchar putchar;
  HtmlCallbackWrite write;
  HtmlCallbackSeek seek;

  HtmlCallbackDestroy destroy; // customizable function to destroy `void* data` when destroying stream
} HtmlStream;
```

## Methods (methods.h)

**HtmlCreate `<type>`()**

- HtmlObject* HtmlCreateDocument()
- HtmlObject* HtmlCreateTag(const char* name, const char* text)
- HtmlObject* HtmlCreateSingle(const char* name)
- HtmlObject* HtmlCreateScript()
- HtmlObject* HtmlCreateStyle()

**HtmlDestroy `<type>`()**

- void HtmlDestroyObject(HtmlObject* object)
- void HtmlClearChildren(HtmlObject* object)

**HtmlAddObject `<item>`()**

- HtmlObject* HtmlAddObjectChild(HtmlObject* parent, HtmlObject* child)

**HtmlSetObject `<item>`()**

- HtmlCode HtmlSetObjectText(HtmlObject* object, const char* text)
- HtmlCode HtmlSetObjectAttribute(HtmlObject* object, const char* attrName, const char* attrValue)

**HtmlGetObject `<item>`()**

- const char* HtmlGetObjectName(HtmlObject* object)

// Return text by only this object, better not edit its return

- const char* HtmlGetObjectInnerText(HtmlObject* object)

// Write object's text within its children

- HtmlCode HtmlGetObjectText(HtmlObject* object, HtmlStream* stringStream)
- const char* HtmlGetObjectAfterText(HtmlObject* object)
- const char* HtmlGetObjectAttributeValue(HtmlObject* object, const char* attrName)
- HtmlObject* HtmlGetObjectFirstChild(HtmlObject* object)
- HtmlObject* HtmlGetObjectLastChild(HtmlObject* object)
- HtmlObject* HtmlGetObjectParent(HtmlObject* object)

**HtmlRemoveObject `<item>`()**

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

**HtmlRead `<inputTypes>`()**

- HtmlObject* HtmlReadStream(HtmlStream* stream)
- HtmlObject* HtmlReadString(const char* html)
- HtmlObject* HtmlReadFileObject(FILE* file)
- HtmlObject* HtmlReadFile(const char* filename)

## Methods (writer.h)

**HtmlWrite `<outputTypes>`()**

- HtmlCode HtmlWriteStream(HtmlObject* object, HtmlStream* stream)
- HtmlCode HtmlWriteStringStream(HtmlObject* object, HtmlStream* stringStream)
- HtmlCode HtmlWriteFileObject(HtmlObject* object, FILE* file)
- HtmlCode HtmlWriteFile(HtmlObject* object, const char* file)

## Methods (search.h)

**HtmlObjectArray* HtmlSearchObject(HtmlObject* object, const char* words, int limit)**

**HtmlObject* HtmlFindObject(HtmlObject* object, const char* words)**
