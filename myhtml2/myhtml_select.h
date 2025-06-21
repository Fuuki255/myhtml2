#ifndef _MYHTML_SELECT_H_
#define _MYHTML_SELECT_H_

#ifndef _MYHTML_OBJECT_H_
#include "myhtml_object.h"
#endif


typedef struct HtmlSelect {
    char* _name;
    char* _class;
    char* _id;

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
        // cleaning //

        // clear spaces
        while (c != 0 && isspace(c)) {
            c = *(++patterns);
        }

        // no more pattern
        if (c == 0) {
            return first;
        }

        // making HtmlSelect //

        // malloc HtmlSelect
        HtmlSelect* select = (HtmlSelect*)calloc(sizeof(HtmlSelect));
        HtmlHandleOutOfMemoryError(select, first);

        // setup relationship
        if (last != NULL) {
            last->next = select;
        }
        if (first == NULL) {
            first = select;
        }

        // read pattern //
        char** write = &select->_name;
        int writeLength, writeCapacity = 0;

        c = *patterns;
        while (c != 0) {
            // clearing spaces
            if (isspace(c)) {
                c = *(++patterns);
                break;
            }

            // switch reading type
            if (c == '.') {
                if (selcet->_class != NULL) {
                    fprintf(stderr, "warning %s: pattern overwriting class select!\n", __func__);
                    free(select->_class);
                }
                write = &select->_class;
                continue;
            }
            if (c == '#') {
                if (selcet->_id != NULL) {
                    fprintf(stderr, "warning %s: pattern overwriting id select!\n", __func__);
                    free(select->_id);
                }
                write = &select->_id;
                continue;
            }

            // targeted index and close
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
                char* newBuffer = (char*)realloc(*write, newCapacity);

                //! error: failed to allocate memory
                if (newBuffer == NULL) {
                    free(select->pattern);
                    free(select);
                    last->next = NULL;
                    HtmlHandleOutOfMemoryError(true, first);
                }

                *write = newBuffer;
                capacity = newCapacity;
            }

            // write char
            (*write)[length++] = c;
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