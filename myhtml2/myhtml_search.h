#ifndef _MYHTML_SELECT_H_
#define _MYHTML_SELECT_H_

#ifndef _MYHTML_OBJECT_H_
#include "myhtml_object.h"
#endif


typedef struct HtmlSelect {
    char* pattern;
    int targetIndex;

    HtmlSelect* next;
} HtmlSelect;


typedef struct HtmlArray {
    HtmlObject** objects;
    int count;
    int limit;
} HtmlArray;



// Go forward the select
HtmlSelect* HtmlCreateSelect(const char* patterns) {
    // malloc first HtmlSelect
    HtmlSelect* first = NULL;
    HtmlSelect* last = NULL;

    // read patterns
    int c;

    while ((c = *patterns)) {
        // clear spaces
        while (c != 0 && isspace(c)) {
            c = *(++patterns);
        }

        // no more pattern
        if (c == 0) {
            return first;
        }

        // malloc HtmlSelect
        HtmlSelect* select = (HtmlSelect*)malloc(sizeof(HtmlSelect));
        HtmlHandleOutOfMemoryError(select, first);

        select->pattern = (char*)malloc(18);
        if (select->pattern == NULL) {
            free(select);
            return first;
        }
        int length = 0;
        int capacity = 18;

        // setup relationship
        if (last != NULL) {
            last->next = select;
        }
        if (first == NULL) {
            first = select;
        }

        // read pattern
        while ((c = *patterns)) {
            // special chars
            if (isspace(c)) {
                patterns++;
                break;
            }
            if (c == '.' || c == '#') {
                break;
            }
            if (c == '[') {
                sscanf(++patterns, "%d", &select->targetIndex);

                while ((c = *patterns) == ']' || isdigit(c) || isspace(c)) {
                    patterns++;
                }
                break;
            }

            // read char //

            // expand string
            if (length + 1 >= capacity) {
                int newCapacity = capacity + 18;
                char* newBuffer = (char*)realloc(select->pattern, newCapacity);

                if (newBuffer == NULL) {
                    free(select->pattern);
                    free(select);
                    last->next = NULL;
                    fprintf(stderr, "error %s: Out of memory\n", __func__);
                    return first;
                }

                select->pattern = newBuffer;
                capacity = newCapacity;
            }

            // write char
            select->pattern[length++] = c;
        }

        // add '\0' back
        select->pattern[length] = 0;
        last = select;
    }

    return first;
}





HtmlSelect HtmlSelectObject(HtmlObject* object, const char* patterns, int limit) {

}

HtmlObject* HtmlGetObjectChild(HtmlObject* object, const char* patterns) {
    HtmlHandleNullError(object, NULL);
    HtmlHandleEmptyStringError(patterns, NULL);

    
}






#endif /* _MYHTML_SELECT_H_ */