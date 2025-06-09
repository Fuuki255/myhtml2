#ifndef MYHTML_PARSER_H
#define MYHTML_PARSER_H

#define HTML_PARSER_VERSION {1, 0, 0}


#ifndef MYHTML_DEFINES_H
#include "MyHtml_defines.h"
#endif
#ifndef MYHTML_STREAM_H
#include "MyHtml_stream.h"
#endif

#include <ctype.h>



/* Basic */

#define HTML_LowerChar(c)	(c >= 'A' && c <= 'Z' ? (c + 32) : c)


#ifndef COMPARE_STRINGS_H
#define COMPARE_STRINGS_H

#include <string.h>
#include <stdarg.h>

// Compare first string with strings
//
// @param str The string to compare with many strings
// @param ... Strings to compare with str
// @return If str equal to any string, return 1. if no string equal to str, return 0
int CompareStrings(const char* str, ...) {
	va_list ap;
	va_start(ap, str);

	const char* cmp;
	while ((cmp = va_arg(ap, const char*))) {
		if (strcmp(str, cmp) == 0) {
			va_end(ap);
			return 1;
		}
	}
	va_end(ap);
	return 0;
}

#endif /* COMPARE_STRINGS_H */





HTML_Type HTML_DetactElementType(const char* tagName) {
	if (CompareStrings(tagName, "meta", "link", "img", "input", "br", NULL)) {
		return HTML_SINGLE;
	}
	if (CompareStrings(tagName, "script", "style", NULL)) {
		return HTML_SCRIPT;
	}
	return HTML_TAG;
}

int _HTML_Hex(int c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	}
	if (c >= 'a' && c <= 'z') {
		return c - 'a' + 10;
	}
	if (c >= 'A' && c <= 'Z') {
		return c - 'A' + 10;
	}
	return 0;
}

char* _HTML_ParseFormatedString(HTML_Stream* stream, int symbol) {
	HTML_String buf = _HTML_CreateString(24);
	int c;
	int c1, c2;

	while ((c = HTML_GetCharFromStream(stream)) != symbol) {
		if (c == -1) {
			fprintf(stderr, "ERROR: %s: HTML not expected end! (position: %lu)\n", __func__, HTML_GetStreamPosition(stream));
			break;
		}
		if (c != '\\') {
			HTML_PutCharToString(c, &buf);
			continue;
		}

		switch (HTML_GetCharFromStream(stream)) {
			case 'a':
				HTML_PutCharToString('\a', &buf);
				break;
			case 'r':
				HTML_PutCharToString('\r', &buf);
				break;
			case 'n':
				HTML_PutCharToString('\n', &buf);
				break;
			case 't':
				HTML_PutCharToString('\t', &buf);
				break;
			case 'x':
				c1 = _HTML_Hex(HTML_GetCharFromStream(stream));
				c2 = _HTML_Hex(HTML_GetCharFromStream(stream));

				HTML_PutCharToString((c1 << 4) + c2, &buf);
				break;
			default:
				HTML_PutCharToString(c, &buf);
		}
	}
	return (char*)HTML_ConvertStringToCString(&buf);
}

int _HTML_ParseAttributes(HTML_Element* element, HTML_Stream* stream, int c) {
	HTML_Attribute* attr;
	HTML_String name, value;

	while (isspace(c)) {
		// Clear Spaces
		while (isspace(c)) {
			c = HTML_GetCharFromStream(stream);
		}

		// Create attribute
		attr = (HTML_Attribute*)malloc(sizeof(HTML_Attribute));
		
		attr->front = NULL;
		attr->back = element->lastAttr;

		if (element->lastAttr) {
			element->lastAttr->front = attr;
		}
		if (!element->firstAttr) {
			element->firstAttr = attr;
		}
		element->lastAttr = attr;

		// Read name
		name = _HTML_CreateString(12);

		while (isalnum(c) || c == '-' || c == '_') {
			HTML_PutCharToString(HTML_LowerChar(c), &name);
			c = HTML_GetCharFromStream(stream);
		}

		attr->name = (char*)HTML_ConvertStringToCString(&name);

		// Set value (bool)
		if (c != '=') {
			attr->value = NULL;
			continue;
		}

		// Next level
		c = HTML_GetCharFromStream(stream);

		// Set value (formated string)
		if (c == '\'' || c == '\"') {
			attr->value = _HTML_ParseFormatedString(stream, c);
			c = HTML_GetCharFromStream(stream);
			continue;
		}

		// Set value (string)
		value = _HTML_CreateString(20);

		while (isalnum(c) || c == '_' || c == '-') {
			HTML_PutCharToString(HTML_LowerChar(c), &value);
			c = HTML_GetCharFromStream(stream);
		}

		attr->value = (char*)HTML_ConvertStringToCString(&value);
	}
	return c;
}









HTML_Element* HTML_ParseStream(HTML_Stream* stream) {
	HTML_Element* doc = (HTML_Element*)calloc(1, sizeof(HTML_Element));
	doc->type = HTML_DOCUMENT;
	
	HTML_Element* current = doc;
	
	HTML_String buf1;
	int c;
	size_t start;
	
	while (1) {
		// Read char
		start = HTML_GetStreamPosition(stream);
		c = HTML_GetCharFromStream(stream);
	SkipLoopGetChar:
		// Clear Spaces
		while (isspace(c)) {
			c = HTML_GetCharFromStream(stream);
		}
		
		// Exit if end
		if (c == -1) {
			return doc;
		}
		
		// Insert Text
		if (c != '<') {
			HTML_SeekStream(stream, start, SEEK_SET);
			goto InsertText;
		}
		
		// Create Buffer to save read
		buf1 = _HTML_CreateString(24);
		
		// Next level detact
		c = HTML_GetCharFromStream(stream);
		
		if (c == '!') {
			// HTML_COMMENT
			HTML_ReadStream(buf1.buffer, 1, 2, stream);
			
			if (strcmp(buf1.buffer, "--") == 0) {
				// Read comment text
				buf1.length = 0;
				c = HTML_GetCharFromStream(stream);

				while (1) {
					if (c == -1) {
						fprintf(stderr, "ERROR: %s: HTML not expected end! (position: %lu)\n", __func__, HTML_GetStreamPosition(stream));
						HTML_DestroyString(&buf1);
						return doc;
					}

					HTML_PutCharToString(HTML_LowerChar(c), &buf1);
					if (buf1.length >= 3 && memcmp(buf1.buffer + buf1.length - 3, "-->", 3) == 0) {
						break;
					}
					c = HTML_GetCharFromStream(stream);
				}
				buf1.buffer[buf1.length - 3] = 0;

				// Create Object
				HTML_Element* tagComment = HTML_AddElementChild(current, (HTML_Element*)calloc(1, sizeof(HTML_Element)));
				tagComment->type = HTML_COMMENT;
				tagComment->text = buf1.buffer;
				continue;
			}
			
			/* HTML_DOCTYPE */
			HTML_ReadStream(buf1.buffer + 2, 1, 6, stream);
			buf1.buffer[8] = 0;
			
			if (strcasecmp(buf1.buffer, "doctype ") == 0) {
				// Read text
				buf1.length = 0;
				c = HTML_GetCharFromStream(stream);

				while (c != '>' && c != -1) {
					HTML_PutCharToString(HTML_LowerChar(c), &buf1);
					c = HTML_GetCharFromStream(stream);
				}

				// Create object
				HTML_Element* tagDoctype = HTML_AddElementChild(current, (HTML_Element*)calloc(1, sizeof(HTML_Element)));
				tagDoctype->type = HTML_DOCTYPE;
				tagDoctype->text = (char*)HTML_ConvertStringToCString(&buf1);
				continue;
			}
			
			// ERROR: Unknow format "<!XXXXXXXX"
			HTML_DestroyString(&buf1);
			goto InsertText;
		}
		/* End Tag */
		if (c == '/') {
			// read close tag name
			c = HTML_GetCharFromStream(stream);

			while (isalnum(c)) {
				HTML_PutCharToString(HTML_LowerChar(c), &buf1);
				c = HTML_GetCharFromStream(stream);
			}

			// ERROR: Not expected stream end
			if (c == -1) {
				fprintf(stderr, "ERROR: %s: HTML not expected end! (position: %lu)\n", __func__, HTML_GetStreamPosition(stream));
				HTML_DestroyString(&buf1);
				return doc;
			}

			HTML_ConvertStringToCString(&buf1);
			
			// Find return element
			HTML_Element* record = current;
			while (record != doc && strcmp(record->name, buf1.buffer) != 0) {
				record = record->owner;
			}

			// ERROR: Not exists element name
			if (record == doc) {
				printf("WARN: %s: HTML closing not exists element! (position: %lu)\n", __func__, HTML_GetStreamPosition(stream));
				continue;
			}

			// Return
			current = record->owner;
			continue;
		}
		
		/* HTML_TAG | HTML_SCRIPT | HTML_SINGLE */

		// Read tag name
		while (isalnum(c)) {
			HTML_PutCharToString(HTML_LowerChar(c), &buf1);
			c = HTML_GetCharFromStream(stream);
		}

		// Create tag
		current = HTML_AddElementChild(current, (HTML_Element*)calloc(1, sizeof(HTML_Element)));

		current->name = (char*)HTML_ConvertStringToCString(&buf1);
		current->type = HTML_DetactElementType(current->name);


		// Read Attributes
		c = _HTML_ParseAttributes(current, stream, c);

		// Return if new element is single
		if (current->type == HTML_SINGLE) {
			current = current->owner;
		}

		// Special read text method for script / style
		if (current->type == HTML_SCRIPT) {
			// Make string "</tagName>" for end check, it will never take more than 10 bytes
			size_t endLength = strlen(current->name) + 3;

			char endText[12];
			sprintf(endText, "</%s>", current->name);

			buf1 = _HTML_CreateString(256);			// Bigger start buffer

			// Read forever
			while ((c = HTML_GetCharFromStream(stream)) != -1) {
				HTML_PutCharToString(c, &buf1);

				if (buf1.length >= endLength && memcmp(buf1.buffer + buf1.length - endLength, endText, endLength) == 0) {
					break;
				}
			}
			buf1.length -= endLength;

			// Store text
			current->text = (char*)HTML_ConvertStringToCString(&buf1);
			
			// Leave tag
			current = current->owner;
		}
		
		continue;
		
		
	InsertText:
		// Get insert place
		char** place;
		if (current->lastChild) {
			place = &current->lastChild->interval;
		}
		else {
			place = &current->text;
		}
		
		// Setup buf1 by place result
		if (*place) {
			buf1.buffer = *place;
			buf1.length = strlen(buf1.buffer);
			buf1.memsize = buf1.length + 1;
		}
		else {
			buf1 = _HTML_CreateString(24);
		}

		// Set Standard Read
		long standardRead = HTML_GetStreamPosition(stream) - start;
		HTML_SeekStream(stream, start, SEEK_SET);

		// Read untils structure detacted
		while (1) {
			c = HTML_GetCharFromStream(stream);

			if ((standardRead--) <= 0 && (c == -1 || c == '<')) {
				break;
			}
			HTML_PutCharToString(c, &buf1);
		}
		
		// Store buf1
		*place = (char*)HTML_ConvertStringToCString(&buf1);

		// Next loop but no GetChar
		start = HTML_GetStreamPosition(stream) - 1;
		goto SkipLoopGetChar;
	}
	
	
	
	return doc;
}





typedef struct HTML_StrReader {
	char* buffer;
	size_t now;
	size_t limit;
} HTML_StrReader;


int HTML_GetCharFromStrReader(HTML_StrReader* reader) {
	if (reader->now < reader->limit) {
		return reader->buffer[reader->now++];
	}
	return -1;
}

size_t HTML_ReadStrReader(char* buf, size_t size, size_t n, HTML_StrReader* reader) {
	long less = reader->limit - reader->now;
	
	if (less < 0) {
		return 0;
	}
	if (n > less) {
		n = less;
	}
	
	memcpy(buf, reader->buffer + reader->now, n);
	buf[n] = 0;
	reader->now += n;
	return n;
}

void HTML_SeekStrReader(HTML_StrReader* reader, long where, int seek) {
	reader->now = where;
	
}

size_t HTML_GetStrReaderPosition(HTML_StrReader* reader) {
	return reader->now;
}


// int GetCharFromFile(FILE* file) {
// 	printf("GetCharFromFile: read!\n");
// 	return fgetc(file);
// }




HTML_Element* HTML_ParseString(const char* htmlString) {
    HTML_Error_NullParam(htmlString, NULL);

	HTML_StrReader reader;
	reader.buffer = (char*)htmlString;
	reader.now = 0;
	reader.limit = strlen(htmlString);
	
	HTML_Stream stream = {&reader};
	stream.getc = (HTML_Callback_GetChar)HTML_GetCharFromStrReader;
	stream.read = (HTML_Callback_Read)HTML_ReadStrReader;
	
	stream.seek = (HTML_Callback_Seek)HTML_SeekStrReader;
	stream.getPosition = (HTML_Callback_GetPosition)HTML_GetStrReaderPosition;
	
	
	return HTML_ParseStream(&stream);
}


HTML_Element* HTML_ParseFileObject(FILE* file) {
    HTML_Error_NullParam(file, NULL);

	HTML_Stream stream = {file};
	stream.getc = (HTML_Callback_GetChar)fgetc;
	stream.read = (HTML_Callback_Read)fread;
	stream.seek = (HTML_Callback_Seek)fseek;
	stream.getPosition = (HTML_Callback_GetPosition)ftell;

	return HTML_ParseStream(&stream);
}

HTML_Element* HTML_ParseFile(const char* filename) {
    HTML_Error_NullParam(filename, NULL);

	FILE* file = fopen(filename, "r");
	if (!file) {
		fprintf(stderr, "ERROR: %s: Failed to open file \'%s\'\n", __func__, filename);
		return NULL;
	}

	HTML_Element* element = HTML_ParseFileObject(file);
	fclose(file);
	return element;
}









#endif /* MYHTML_PARSER_H */