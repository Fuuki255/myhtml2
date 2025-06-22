#ifndef _MYHTML_SELECT_H_
#define _MYHTML_SELECT_H_

#ifndef _MYHTML_OBJECT_H_
#include "myhtml_object.h"
#endif


#include <ctype.h>



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

    // read patterns loop
    int c;

    while ((c = *patterns) != 0) {
        // removes spaces //
        while (isspace(c)) {
            c = *(++patterns);

            if (c == 0) {
                return first;
            }
        }

        // create pattern //
        HtmlSelect* select = (HtmlSelect*)calloc(1, sizeof(HtmlSelect));
        HtmlHandleOutOfMemoryError(select, first);

        if (first == NULL) {
            first = select;
        }
        // the `last` variable will set late

        // read pattern //
        char** write = NULL;
        int writeLength = 0;
        int writeCapacity = 16;

        for (; c != 0 && !isspace(c); c = *(++patterns)) {
            // special read
            if (c == '[') {
                sscanf(++patterns, "%d", &select->targetIndex);
                
                for (; (c = *patterns) != 0 && (isspace(c) || isdigit(c) || c == ']' || c == '-'); patterns++);

                break;
            }

            // switch write out
            if (c == '.') {
                if (write) {
                    (*write)[writeLength] = 0;
                }
                write = &select->_class;
                goto SetupWrite;
            }
            if (c == '#') {
                if (write) {
                    (*write)[writeLength] = 0;
                }
                write = &select->_id;
                goto SetupWrite;
            }
            if (write == NULL) {
                write = &select->_name;
                goto SetupWrite;
            }

            // write char to `write`
            if (writeLength + 1 >= writeCapacity) {
                writeCapacity = writeCapacity + 16;
                (*write) = (char*)realloc((*write), writeCapacity);
                
                if ((*write) == NULL) {
                    HtmlLibDestroyPointer(select->_name);
                    HtmlLibDestroyPointer(select->_class);
                    HtmlLibDestroyPointer(select->_id);

                    free(select);
                    return first;
                }
            }

            (*write)[writeLength++] = HtmlLibLowerChar(c);
            continue;

            // goto method for setup `write` addional variables, you should set write before
        SetupWrite:
            if ((*write)) {
                HtmlLogWarning("overwriting text!");
            }
            else {
                *write = (char*)malloc(16);
            }

            writeLength = 0;
            writeCapacity = 16;
        }

        // completes variables
        (*write)[writeLength] = 0;

        if (last) {
            last->next = select;
        }
        last = select;
    }

    return first;
}


void HtmlDestroySelect(HtmlSelect* select) {
    if (select == NULL) {
        return;
    }

    for (HtmlSelect* next = select->next; select; select = next, next = next ? next->next : NULL) {
        free(select->_name);
        free(select->_class);
        free(select->_id);

        free(select);
    }
}


bool HtmlIsObjectPatterns(HtmlObject* object, HtmlSelect* select) {
    if (select->_name && strcmp(select->_name, object->name) != 0) {
        return false;
    }
    if (select->_class && strcmp(select->_class, HtmlGetObjectAttributeValue(object, "class")) != 0) {
        return false;
    }
    if (select->_class && strcmp(select->_class, HtmlGetObjectAttributeValue(object, "id")) != 0) {
        return false;
    }

    return true;
}






HtmlObject* HtmlLibFindObject(HtmlObject* object, HtmlSelect* select) {
    // setup iterator variables //
    HtmlObjectIterator iterator;
    HtmlObject* (*IncreaseMethod)(HtmlObjectIterator*) = NULL;

    int index;
    int indexIncrease;

    if (select->targetIndex >= 0) {
        iterator = HtmlBeginObject(object);
        IncreaseMethod = HtmlNextObject;
        index = 0;
        indexIncrease = 1;
    }
    else {
        iterator = HtmlEndObject(object);
        IncreaseMethod = HtmlPrevObject;
        index = -1;
        indexIncrease = -1;
    }

    // finding //
    HtmlObject* result;
    for (HtmlObject* child; (child = IncreaseMethod(&iterator)); ) {

        // having next select, tag and document only //
        if (select->next) {
            if (child->type == HTML_TYPE_TAG) {
                if (HtmlIsObjectPatterns(child, select)) {
                    if (index != select->targetIndex) {
                        index += indexIncrease;
                        continue;
                    }

                    result = HtmlLibFindObject(child, select->next);
                    if (result) {
                        return result;
                    }
                }

                result = HtmlLibFindObject(child, select);
                if (result) {
                    return result;
                }
            }
            if (child->type == HTML_TYPE_DOCUMENT) {
                result = HtmlLibFindObject(child, select);
                if (result) {
                    return result;
                }
            }
            continue;
        }

        // no next select, allow more types checking //

        if (child->type == HTML_TYPE_TAG || child->type == HTML_TYPE_SINGLE || child->type == HTML_TYPE_SCRIPT) {
            if (HtmlIsObjectPatterns(child, select)) {

                if (index != select->targetIndex) {
                    index += indexIncrease;
                    continue;
                }

                return child;
            }
        }

        if (child->type == HTML_TYPE_TAG || child->type == HTML_TYPE_DOCUMENT) {
            result = HtmlLibFindObject(child, select);
            if (result) {
                return result;
            }
        }
    }

    return NULL;
}








HtmlArray HtmlSelectObject(HtmlObject* object, const char* patterns, int limit) {
    return (HtmlArray){};
}

HtmlObject* HtmlFindObject(HtmlObject* object, const char* patterns) {
    HtmlHandleNullError(object, NULL);
    HtmlHandleEmptyStringError(patterns, NULL);

    HtmlSelect* select = HtmlCreateSelect(patterns);
    HtmlObject* result = HtmlLibFindObject(object, select);
    HtmlDestroySelect(select);
    return result;
}






#endif /* _MYHTML_SELECT_H_ */