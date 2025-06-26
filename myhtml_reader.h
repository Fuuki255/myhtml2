#ifndef _MYHTML_READER_H_
#define _MYHTML_READER_H_

#ifndef _MYHTML_OBJECT_H_
#include "myhtml_object.h"
#endif


#include <ctype.h>






bool HtmlLibIsStringIn(const char* str, ...) {
    va_list ap;
    va_start(ap, str);

    const char* cmp;
    while ((cmp = va_arg(ap, const char*))) {
        if (strcmp(str, cmp) == 0) {
            va_end(ap);
            return true;
        }
    }
    va_end(ap);
    return false;
}



HtmlObjectType HtmlLibDetactObjectType(const char* tagName) {
    if (HtmlLibIsStringIn(tagName, "meta", "link", "img", "input", "br", "hr", NULL)) {
        return HTML_TYPE_SINGLE;
    }
    if (HtmlLibIsStringIn(tagName, "script", "style", NULL)) {
        return HTML_TYPE_SCRIPT;
    }
    return HTML_TYPE_TAG;
}

int HtmlLibHexToInt(char c) {
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



char* HtmlLibParseFormatedString(HtmlStream* stream, int symbol) {
	HtmlStream output = HtmlCreateStreamBuffer(24);
	int c;
	int c1, c2;

	while ((c = stream->getchar(stream->data)) != symbol) {
        if (c == EOF) {
            fprintf(stderr, "%s: HTML not expected end! (position: %lu)\n", __func__, stream->tell(stream->data));
            HtmlStreamString* outputData = (HtmlStreamString*)output.data;
            outputData->buffer[outputData->length] = 0; // Null-terminate the string

            return outputData->buffer;
        }
		if (c != '\\') {
			output.putchar(c, output.data);
			continue;
		}
        
		switch (stream->getchar(stream->data)) {
			case 'a':
				output.putchar('\a', output.data);
				break;
			case 'r':
				output.putchar('\r', output.data);
				break;
			case 'n':
				output.putchar('\n', output.data);
				break;
			case 't':
				output.putchar('\t', output.data);
				break;
			case 'x':
				c1 = HtmlLibHexToInt(stream->getchar(stream->data));
				c2 = HtmlLibHexToInt(stream->getchar(stream->data));

				output.putchar((c1 << 4) + c2, output.data);
				break;
			default:
				output.putchar(c, output.data);
		}
	}
	return (char*)HtmlGetStreamString(&output);
}



int HtmlLibParseAttributes(HtmlObject* object, HtmlStream* stream, int c) {
	HtmlAttribute* attr;
	HtmlStream name, value;

	while (isspace(c)) {
		// Clear Spaces
		while (isspace(c)) {
			c = stream->getchar(stream->data);
		}
		if (c == '/') {
			c = stream->getchar(stream->data);

			if (c == '>') {
				return -1;
			}
		}

		// Create attribute
		attr = (HtmlAttribute*)malloc(sizeof(HtmlAttribute));
		HtmlHandleOutOfMemoryError(attr, EOF);
		
		attr->next = NULL;
		attr->prev = object->lastAttribute;

		if (object->lastAttribute) {
			object->lastAttribute->next = attr;
		}
		if (!object->firstAttribute) {
			object->firstAttribute = attr;
		}
		object->lastAttribute = attr;

		// Read name
		name = HtmlCreateStreamBuffer(16);

		while (isalnum(c) || c == '-' || c == '_') {
			name.putchar(HtmlLibLowerChar(c), name.data);
			c = stream->getchar(stream->data);
		}
		attr->name = (char*)HtmlGetStreamString(&name);

		// Set value (bool)
		if (c != '=') {
			attr->value = NULL;
			continue;
		}

		// Next level
		c = stream->getchar(stream->data);

		// Set value (formated string)
		if (c == '\'' || c == '\"') {
			attr->value = HtmlLibParseFormatedString(stream, c);
			c = stream->getchar(stream->data);
			continue;
		}

		// Set value (string)
		value = HtmlCreateStreamBuffer(24);

		while (isalnum(c) || c == '_' || c == '-') {
			value.putchar(HtmlLibLowerChar(c), value.data);
			c = stream->getchar(stream->data);
		}

		attr->value = (char*)HtmlGetStreamString(&value);
	}
	return c;
}






HtmlObject* HtmlLibReadObjectFromStream(HtmlStream* stream) {
	HtmlObject* doc = HtmlCreateObjectDocument();
	
	HtmlObject* current = doc;
	HtmlStreamString buffer1;
	int c;
	size_t start;
	
	while (true) {
		// Read char
		start = stream->tell(stream->data);
		c = stream->getchar(stream->data);
	SkipLoopGetChar:
		// Clear Spaces
		while (isspace(c)) {
			c = stream->getchar(stream->data);
		}
		
		// Exit if end
		if (c == -1) {
			return doc;
		}
		
		// Insert Text
		if (c != '<') {
			stream->seek(stream->data, start, SEEK_SET);
			goto InsertText;
		}
		
		// Create Buffer to save read
		buffer1 = (HtmlStreamString){(char*)malloc(24), 0, 0, 24};
		
		// Next level detact
		c = stream->getchar(stream->data);
		
		if (c == '!') {
			// HTML_COMMENT
			stream->read(buffer1.buffer, 1, 2, stream->data);
			
			if (strcmp(buffer1.buffer, "--") == 0) {
				// Read comment text
				buffer1.length = 0;
				c = stream->getchar(stream->data);

				while (true) {
					if (c == -1) {
						free(buffer1.buffer);
						HtmlHandleError(true, doc, "html not expected end! (position %lu)", stream->tell(stream->data));
					}

					HtmlLibPutcharToStreamString(HtmlLibLowerChar(c), &buffer1);
					if (buffer1.length >= 3 && memcmp(buffer1.buffer + buffer1.length - 3, "-->", 3) == 0) {
						break;
					}
					c = stream->getchar(stream->data);
				}
				buffer1.buffer[buffer1.length - 3] = 0;

				// Create Object
				HtmlObject* tagComment = HtmlAddObjectChild(current, (HtmlObject*)calloc(1, sizeof(HtmlObject)));
				tagComment->type = HTML_TYPE_COMMENT;
				tagComment->innerText = buffer1.buffer;
				continue;
			}
			
			/* HTML_DOCTYPE */
			stream->read(buffer1.buffer + 2, 1, 6, stream->data);
			buffer1.buffer[8] = 0;
			
			if (strcasecmp(buffer1.buffer, "doctype ") == 0) {
				// Read text
				buffer1.length = 0;
				c = stream->getchar(stream->data);

				while (c != '>' && c != -1) {
					HtmlLibPutcharToStreamString(HtmlLibLowerChar(c), &buffer1);
					c = stream->getchar(stream->data);
				}

				// Create object
				HtmlObject* tagDoctype = HtmlAddObjectChild(current, (HtmlObject*)calloc(1, sizeof(HtmlObject)));
				tagDoctype->type = HTML_TYPE_DOCTYPE;
                
                buffer1.buffer[buffer1.length] = 0; // Null-terminate the string
				tagDoctype->innerText = buffer1.buffer;
				continue;
			}
			
			// ERROR: Unknow format "<!XXXXXXXX"
			free(buffer1.buffer);
			goto InsertText;
		}
		/* End Tag */
		if (c == '/') {
			// read close tag name
			c = stream->getchar(stream->data);

			while (isalnum(c)) {
				HtmlLibPutcharToStreamString(HtmlLibLowerChar(c), &buffer1);
				c = stream->getchar(stream->data);
			}

			// ERROR: Not expected stream end
			if (c == -1) {
				fprintf(stderr, "error %s: HTML not expected end! (position: %lu)\n", __func__, stream->tell(stream->data));
				free(buffer1.buffer);
				return doc;
			}

			buffer1.buffer[buffer1.length] = 0; // Null-terminate the string
			
			// Find return element
			HtmlObject* backTag = current;
			while (backTag != doc && strcmp(backTag->name, buffer1.buffer) != 0) {
				backTag = backTag->parent;
			}

			// warning: Not exists element name
			if (backTag == doc) {
				HtmlLogWarning("'%s' tag exit without closing! (position: %lu)", HtmlGetObjectName(current), stream->tell(stream->data));
				continue;
			}

			// Return
			current = backTag->parent;
			continue;
		}
		
		/* HTML_TAG | HTML_SCRIPT | HTML_SINGLE */

		// Read tag name
		while (isalnum(c)) {
			HtmlLibPutcharToStreamString(HtmlLibLowerChar(c), &buffer1);
			c = stream->getchar(stream->data);
		}

		// Create tag
		current = HtmlAddObjectChild(current, (HtmlObject*)calloc(1, sizeof(HtmlObject)));

        buffer1.buffer[buffer1.length] = 0; // Null-terminate the string
		current->name = buffer1.buffer;
		current->type = HtmlLibDetactObjectType(current->name);


		// Read Attributes
		c = HtmlLibParseAttributes(current, stream, c);

		// Return if new element is single
		if (current->type == HTML_TYPE_SINGLE) {
			current = current->parent;
		}

		// Special read text method for script / style
		if (current->type == HTML_TYPE_SCRIPT) {
			// Make string "</tagName>" for end check, it will never take more than 10 bytes
			size_t endLength = strlen(current->name) + 3;

			char endText[12];
			sprintf(endText, "</%s>", current->name);

			buffer1 = (HtmlStreamString){(char*)malloc(256), 0, 0, 256};			// Bigger start buffer

			// Read forever
			while ((c = stream->getchar(stream->data)) != -1) {
				HtmlLibPutcharToStreamString(c, &buffer1);

				if (buffer1.length >= endLength && memcmp(buffer1.buffer + buffer1.length - endLength, endText, endLength) == 0) {
					break;
				}
			}
			buffer1.length -= endLength;

			// Store text
            buffer1.buffer[buffer1.length] = 0; // Null-terminate the string
			current->innerText = buffer1.buffer;
			
			// Leave tag
			current = current->parent;
		}
		
		continue;
		
		
	InsertText:
		// Get insert place
		char** place;
		if (current->lastChild) {
			place = &current->lastChild->afterText;
		}
		else {
			place = &current->innerText;
		}
		
		// Setup buffer1 by place result
		if (*place) {
			buffer1.buffer = *place;
			buffer1.length = strlen(buffer1.buffer);
			buffer1.capacity = buffer1.length + 1;
		}
		else {
			buffer1 = (HtmlStreamString){(char*)malloc(24), 0, 0, 24};
		}

		// Set Standard Read
		long standardRead = stream->tell(stream->data) - start;
		stream->seek(stream->data, start, SEEK_SET);

		// Read untils structure detacted
		while (true) {
			c = stream->getchar(stream->data);

			if ((standardRead--) <= 0 && (c == -1 || c == '<')) {
				break;
			}
			HtmlLibPutcharToStreamString(c, &buffer1);
		}
		
		// Store buffer1
        buffer1.buffer[buffer1.length] = 0; // Null-terminate the string
		*place = buffer1.buffer;

		// Next loop but no GetChar
		start = stream->tell(stream->data) - 1;
		goto SkipLoopGetChar;
	}

	return doc;
}




HtmlObject* HtmlReadObjectFromStream(HtmlStream* stream) {
    HtmlHandleError(HtmlIsStreamReadable(stream) == false, NULL, "error %s: stream is not readable.", __func__);

    HtmlObject* doc = HtmlLibReadObjectFromStream(stream);
    if (doc == NULL) {
        fprintf(stderr, "Error: Failed to parse HTML stream.\n");
    }
    return doc;
}

HtmlObject* HtmlReadObjectFromString(const char* str) {
	HtmlHandleEmptyStringError(str, NULL);

    HtmlStream stream = HtmlCreateStreamString((char*)str);
    HtmlObject* doc = HtmlLibReadObjectFromStream(&stream);
    if (doc == NULL) {
        fprintf(stderr, "Error: Failed to parse HTML stream.\n");
    }
    return doc;
}

HtmlObject* HtmlReadObjectFromFileObject(FILE* file) {
	HtmlHandleNullError(file, NULL);

	HtmlStream stream = HtmlCreateStreamFileObject(file);
	HtmlObject* doc = HtmlLibReadObjectFromStream(&stream);
	if (doc == NULL) {
		fprintf(stderr, "Error: Failed to parse HTML stream.\n");
	}
	return doc;
}

HtmlObject* HtmlReadObjectFromFile(const char* filename) {
	HtmlHandleEmptyStringError(filename, NULL);

	FILE* file = fopen(filename, "r");
	HtmlHandleError(file == NULL, NULL, "failed to open '%s' in read mode", filename);

	HtmlStream stream = HtmlCreateStreamFileObject(file);
	HtmlObject* doc = HtmlLibReadObjectFromStream(&stream);
	fclose(file);

    HtmlHandleError(doc == NULL, NULL, "failed to parse html file");
	return doc;
}








// libcurl extension

#ifdef CURLINC_CURL_H

HtmlObject* HtmlReadObjectFromCURL(CURL* curl, const char* url) {
	HtmlHandleNullError(curl, NULL);
	HtmlHandleEmptyStringError(url, NULL);

	// Set URL
	HtmlStream stream = HtmlCreateStreamBuffer(1024);
	HtmlHandleError(stream.data == NULL, NULL, "failed to create stream buffer!");

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, stream.data);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, HtmlLibWriteStreamString);

	// perform the request
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		HtmlDestroyStream(&stream);
		HtmlHandleError(true, NULL, "failed to request '%s' (%s)", url, curl_easy_strerror(res));
	}

	// Read HTML object from stream
	HtmlObject* doc = HtmlLibReadObjectFromStream(&stream);
	HtmlDestroyStream(&stream);

	// message if failed
	HtmlHandleError(doc == NULL, NULL, "failed to parse HTML stream from CURL");
	return doc;
}


#endif










#endif /* _MYHTML_READER_H_ */