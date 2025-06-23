#ifndef _MYHTML_SELECT_H_
#define _MYHTML_SELECT_H_

#ifndef _MYHTML_OBJECT_H_
#include "myhtml_object.h"
#endif


#include <ctype.h>



typedef struct HtmlSelectPattern {
    // word filtering
    char* _name;
    char* _class;
    char* _id;

    // number [] filtering
    bool targeted;
    bool reversal;
    int index;

    // more patterns
    HtmlSelectPattern* next;
} HtmlSelectPattern;


typedef struct HtmlSelectTask {
    HtmlSelectPattern* pattern;

    HtmlObjectIterator iterator;
    HtmlObject* (*IncreaseMethod)(HtmlObjectIterator*);

    HtmlSelectTask* prev;
} HtmlSelectTask;


typedef struct HtmlSelect {
    HtmlSelectPattern* patterns;
    HtmlSelectTask* lastTask;
} HtmlSelect;


typedef struct HtmlArray {
    HtmlObject** values;
    int length;
} HtmlArray;




int HtmlLibNoChangeChar(int c) {
    return c;
}


// HtmlSelectPattern methods //


HtmlSelectPattern* HtmlLibCreateSelectPatterns(const char* patterns) {
    // malloc first HtmlSelectPattern
    HtmlSelectPattern* first = NULL;
    HtmlSelectPattern* last = NULL;

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
        HtmlSelectPattern* selectPattern = (HtmlSelectPattern*)calloc(1, sizeof(HtmlSelectPattern));
        HtmlHandleOutOfMemoryError(selectPattern, first);

        if (first == NULL) {
            first = selectPattern;
        }
        // the `last` variable will set late

        // read pattern //
        char** write = NULL;
        int writeLength = 0;
        int writeCapacity = 16;
        int (*ConvertChar)(int) = HtmlLibLowerChar;

        for (; c != 0 && !isspace(c); c = *(++patterns)) {
            // special read
            if (c == '[') {
                // setup target
                sscanf(++patterns, "%d", &selectPattern->index);
                selectPattern->targeted = true;

                if (selectPattern->index < 0) {
                    selectPattern->reversal = true;
                    selectPattern->index = -selectPattern->index - 1; // convert to positive index
                }

                // full lessing words
                for (; (c = *patterns) != 0 && (isspace(c) || isdigit(c) || c == ']' || c == '-'); patterns++);

                break;
            }

            // switch write out
            if (c == '.') {
                if (write) {
                    (*write)[writeLength] = 0;
                }
                write = &selectPattern->_class;
                ConvertChar = HtmlLibNoChangeChar; // do not lower id
                goto SetupWrite;
            }
            if (c == '#') {
                if (write) {
                    (*write)[writeLength] = 0;
                }
                write = &selectPattern->_id;
                ConvertChar = HtmlLibNoChangeChar; // do not lower id
                goto SetupWrite;
            }
            if (write == NULL) {
                write = &selectPattern->_name;
                patterns--;
                goto SetupWrite;
            }

            // write char to `write`
            if (writeLength + 1 >= writeCapacity) {
                writeCapacity = writeCapacity + 16;
                (*write) = (char*)realloc((*write), writeCapacity);
                
                if ((*write) == NULL) {
                    HtmlLibDestroyPointer(selectPattern->_name);
                    HtmlLibDestroyPointer(selectPattern->_class);
                    HtmlLibDestroyPointer(selectPattern->_id);

                    free(selectPattern);
                    return first;
                }
            }

            (*write)[writeLength++] = ConvertChar(c);
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
            last->next = selectPattern;
        }
        last = selectPattern;
    }

    return first;
}


void HtmlLibDestroySelectPatterns(HtmlSelectPattern* selectPattern) {
    if (selectPattern == NULL) {
        return;
    }

    for (HtmlSelectPattern* next = selectPattern->next; selectPattern; selectPattern = next, next = next ? next->next : NULL) {
        free(selectPattern->_name);
        free(selectPattern->_class);
        free(selectPattern->_id);

        free(selectPattern);
    }
}


bool HtmlLibIsObjectSuitPattern(HtmlObject* object, HtmlSelectPattern* selectPattern) {
    /*printf("%s: %s %s   %s %s   %s %s\n", __func__,
           object->name, selectPattern->_name,
           HtmlGetObjectAttributeValue(object, "class"), selectPattern->_class,
              HtmlGetObjectAttributeValue(object, "id"), selectPattern->_id);*/
    if (selectPattern->_name && strcmp(selectPattern->_name, object->name) != 0) {
        return false;
    }
    if (selectPattern->_class && strcmp(selectPattern->_class, HtmlGetObjectAttributeValue(object, "class")) != 0) {
        return false;
    }
    if (selectPattern->_id && strcmp(selectPattern->_id, HtmlGetObjectAttributeValue(object, "id")) != 0) {
        return false;
    }

    return true;
}




// HtmlSelectTask methods //

HtmlSelectTask* HtmlLibCreateSelectTask(HtmlSelectTask* prev, HtmlObject* object, HtmlSelectPattern* selectPattern) {
    HtmlSelectTask* task = (HtmlSelectTask*)malloc(sizeof(HtmlSelectTask));
    HtmlHandleOutOfMemoryError(task, NULL);

    task->pattern = selectPattern;

    if (selectPattern->reversal == false) {
        task->iterator = HtmlBeginObject(object);
        task->IncreaseMethod = HtmlNextObject;
    }
    else {
        task->iterator = HtmlEndObject(object);
        task->IncreaseMethod = HtmlPrevObject;
    }

    task->prev = prev;
    return task;
}


#define HtmlLibDestroySelectTask(task) (free(task))







// HtmlSelect methods //

/*
create a select to select objects by patterns
patterns format be: "tag1.class1#id1[index1] tag2.class2#id2[index2] ..."

- `tag` : tag name
- `class` : class name by operator `.`
- `id` : id name by operator `#`
- `index` : index of result that suit to that pattern, if negative, it reverses search direction

you can use multiple patterns, separated by spaces. every pattern needs tag, class or id, you can use more than one, and optional index.

*/
HtmlSelect HtmlCreateSelect(HtmlObject* object, const char* patterns) {
    HtmlSelect select = {0};

    HtmlHandleNullError(object, select);
    HtmlHandleEmptyStringError(patterns, select);

    // create patterns
    select.patterns = HtmlLibCreateSelectPatterns(patterns);
    HtmlHandleOutOfMemoryError(select.patterns, select);

    // set last task
    select.lastTask = HtmlLibCreateSelectTask(NULL, object, select.patterns);
    if (select.lastTask == NULL) {
        HtmlLibDestroySelectPatterns(select.patterns);
        return (HtmlSelect){0};
    }

    return select;
}

void HtmlDestroySelect(HtmlSelect* select) {
    if (select == NULL) {
        return;
    }

    HtmlLibDestroySelectPatterns(select->patterns);

    HtmlSelectTask* prev;
    while (select->lastTask) {
        prev = select->lastTask->prev;
        HtmlLibDestroySelectTask(select->lastTask);
        select->lastTask = prev;
    }
}



HtmlObject* HtmlNextSelect(HtmlSelect* select) {
    if (select->lastTask == NULL) {
        return NULL;
    }

    // run tasks //
    HtmlSelectTask* task;

    while ((task = select->lastTask)) {
        HtmlObject* child = NULL;

        while (true) {
            child = task->IncreaseMethod(&task->iterator);

            // if object are no child lessing, remove the task
            if (child == NULL) {
                select->lastTask = task->prev;
                HtmlLibDestroySelectTask(task);
                break;
            }

            // if DOCUMENT object, check children //
            if (child->type == HTML_TYPE_DOCUMENT) {
                goto CheckObjectsUnderChild;
            }

            // filtering objects who types not TAG, SINGLE or SCRIPT //
            if (child->type != HTML_TYPE_TAG && child->type != HTML_TYPE_SINGLE && child->type != HTML_TYPE_SCRIPT) {
                continue;
            }

            // check patterns //
            if (HtmlLibIsObjectSuitPattern(child, task->pattern)) {
                // object suit patterns, but not target index
                if (task->pattern->index != 0) {
                    task->pattern->index--;
                    continue;
                }

                // not final pattern, make task to forward select
                if (task->pattern->next) {
                    if (child->type != HTML_TYPE_TAG) {
                        return NULL;
                    }

                    select->lastTask = HtmlLibCreateSelectTask(task, child, task->pattern->next);

                    if (task->pattern->targeted) {
                        select->lastTask->prev = task->prev;
                        HtmlLibDestroySelectTask(task);
                    }
                    break;
                }

                // return object also remove current task
                if (task->pattern->targeted) {
                    select->lastTask = task->prev;
                    HtmlLibDestroySelectTask(task);
                }
                return child;
            }
            
            // if object not suit pattern, SINGLE and SCRIPT types go to next loop, TAG types check its children
            if (child->type != HTML_TYPE_TAG) {
                continue;
            }

            // goto method to check document
        CheckObjectsUnderChild:
            select->lastTask = HtmlLibCreateSelectTask(task, child, task->pattern);
            break;
        }
    }

    // no more result
    return NULL;
}


// HtmlArray methods //

void HtmlDestroyArray(HtmlArray* array) {
    if (array == NULL) {
        return;
    }
    if (array->values == NULL) {
        return; // nothing to destroy
    }
    
    free(array->values);
    array->values = NULL;
    array->length = 0;
}






HtmlArray HtmlFindAllObjects(HtmlObject* object, const char* patterns, int maxCount) {
    HtmlArray array = {0};

    HtmlHandleNullError(object, array);
    HtmlHandleEmptyStringError(patterns, array);

    if (maxCount == 0) {
        return array;
    }

    // create array //
    array.values = (HtmlObject**)malloc(sizeof(HtmlObject*) * maxCount < 0 ? 8 : maxCount);
    if (array.values == NULL) {
        return array;
    }
    array.length = 0;

    // select objects //
    HtmlSelect select = HtmlCreateSelect(object, patterns);
    HtmlObject* result;

    while ((result = HtmlNextSelect(&select))) {
        if (maxCount < 0 && array.length%8 == 7) {
            // increase array size
            HtmlObject** newValues = (HtmlObject**)realloc(array.values, sizeof(HtmlObject*) * (array.length + 8));
            if (newValues == NULL) {
                HtmlDestroySelect(&select);
                HtmlHandleOutOfMemoryError(newValues, array);
            }
            array.values = newValues;
        }
        
        array.values[array.length++] = result;

        if (maxCount > 0 && array.length >= maxCount) {
            break; // stop if reached max count
        }
    }

    HtmlDestroySelect(&select);
    return array;
}


HtmlObject* HtmlFindObject(HtmlObject* object, const char* patterns) {
    HtmlHandleNullError(object, NULL);
    HtmlHandleEmptyStringError(patterns, NULL);

    HtmlSelect select = HtmlCreateSelect(object, patterns);
    HtmlObject* result = HtmlNextSelect(&select);
    HtmlDestroySelect(&select);
    return result;
}



#endif /* _MYHTML_SELECT_H_ */