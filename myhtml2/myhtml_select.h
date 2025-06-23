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
    int targetIndex;

    // more patterns
    HtmlSelectPattern* next;
} HtmlSelectPattern;


typedef struct HtmlSelectTask {
    HtmlSelectPattern pattern;

    HtmlObjectIterator iterator;
    HtmlObject* (*IncreaseMethod)(HtmlObjectIterator*);

    int index;
    int indexIncrease;

    HtmlSelectTask* prev;
} HtmlSelectTask;


typedef struct HtmlSelect {
    HtmlSelectPattern* patterns;
    HtmlSelectTask* lastTask;
} HtmlSelect;




// Go forward the select
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

        for (; c != 0 && !isspace(c); c = *(++patterns)) {
            // special read
            if (c == '[') {
                // setup target
                sscanf(++patterns, "%d", &selectPattern->targetIndex);
                selectPattern->targeted = true;

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
                goto SetupWrite;
            }
            if (c == '#') {
                if (write) {
                    (*write)[writeLength] = 0;
                }
                write = &selectPattern->_id;
                goto SetupWrite;
            }
            if (write == NULL) {
                write = &selectPattern->_name;
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
    if (selectPattern->_name && strcmp(selectPattern->_name, object->name) != 0) {
        return false;
    }
    if (selectPattern->_class && strcmp(selectPattern->_class, HtmlGetObjectAttributeValue(object, "class")) != 0) {
        return false;
    }
    if (selectPattern->_class && strcmp(selectPattern->_class, HtmlGetObjectAttributeValue(object, "id")) != 0) {
        return false;
    }

    return true;
}




HtmlSelectTask* HtmlLibCreateSelectTask(HtmlSelectTask* prev, HtmlObject* object, HtmlSelectPattern* selectPattern) {
    HtmlSelectTask* task = (HtmlSelectTask*)malloc(sizeof(HtmlSelectTask));
    HtmlHandleOutOfMemoryError(task, NULL);

    task->pattern = selectPattern;

    if (selectPattern->targetIndex >= 0) {
        task->iterator = HtmlBeginObject(object);
        task->IncreaseMethod = HtmlNextObject;
        task->index = 0;
        task->indexIncrease = 1;
    }
    else {
        task->iterator = HtmlEndObject(object);
        task->IncreaseMethod = HtmlPrevObject;
        task->index = -1;
        task->indexIncrease = -1;
    }

    task->prev = prev;
    return task;
}


#define HtmlLibDestroySelectTask(task) (free(task))










HtmlSelect HtmlCreateSelect(HtmlObject* object, const char* patterns) {
    HtmlHandleNullError(object, NULL);
    HtmlHandleEmptyStringError(patterns, NULL);

    HtmlSelect select = {0};

    // create patterns
    select.patterns = HtmlLibCreateSelectPatterns(patterns);
    HtmlHandleOutOfMemoryError(select.patterns, select);

    // set last task
    select.lastTask = HtmlLibCreateSelectTask(NULL, object, select.patterns);
    if (select.lastTask == NULL) {
        HtmlDestroyPatterns(select.patterns);
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
        prev = select->last->prev;
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
            child = task->IncreaseMethod(&iterator)

            // if object are no child lessing, remove the task
            if (child == NULL) {
                HtmlLibDestroySelectTask(task);
                select->lastTask = task->prev;
                break;
            }

            // when the pattern not final, select child who has children //

            if (task->pattern->next) {
                if (child->type == HTML_TYPE_TAG) {
                    goto CheckTag;
                }
                if (child->type == HTML_TYPE_DOCUMENT) {
                    result = HtmlLibFindObject(child, task->pattern);
                    if (result) {
                        return result;
                    }
                }
                continue;
            }

            // final pattern, checking child types TAG SINGLE or SCRIPT //
            if (child->type == HTML_TYPE_TAG || child->type == HTML_TYPE_SINGLE || child->type == HTML_TYPE_SCRIPT) {
                if (HtmlLibIsObjectSuitPattern(child, task->pattern)) {
                    // object suit patterns, but not target index
                    if (task->index != task->pattern->targetIndex) {
                        goto IncreaseIndex;
                    }

                    // return object also remove current task
                    if (task->pattern->targeted) {
                        select->lastTask = task->prev;
                        HtmlLibDestroySelectTask(task);
                    }
                    return child;
                }

                // more one checking for TAG, other types handled...
            }

            // no suit for pattern, make task for same pattern finding in child
            if (child->type == HTML_TYPE_TAG || child->type == HTML_TYPE_DOCUMENT) {
                goto CheckObjectsUnderChild;
            }

            continue;



            // goto method to check TAG object
        CheckTag:
            if (HtmlLibIsObjectSuitPattern(child, task->pattern)) {
                // child suit to pattern but not target index
                if (task->index != task->pattern->targetIndex) {
                    goto IncreaseIndex;
                }

                // make task to forward select
                select->lastTask = HtmlLibCreateSelectTask(task, child, task->pattern->next);

                if (task->pattern->targeted) {
                    select->lastTask = task->prev;
                    HtmlLibDestroySelectTask(task);
                }
                break;
            }

            // if TAG not suit to pattern, move to CheckObjectsUnderChild ...

            // goto method to check document
        CheckObjectsUnderChild:
            select->lastTask = HtmlLibCreateSelectTask(task, child, task->pattern);
            break;

            // goto method for increase index
        IncreaseIndex:
            task->index += task->indexIncrease;
            continue;
        }
    }

    // no more result
    return NULL;
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