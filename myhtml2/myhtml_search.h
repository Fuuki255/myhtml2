#ifndef _MYHTML_SEARCH_H_
#define _MYHTML_SEARCH_H_

#ifndef _MYHTML_OBJECT_H_
#include "myhtml_object.h"
#endif


typedef struct HtmlSearch {
    const char* lessPatterns;

    char* pattern;
    int patternLength;
    int patternCapacity;
    int patternTargetIndex;

    HtmlObject** temp;
    int tempCount;
} HtmlSearch;



HtmlSearch HtmlCreateSearch(HtmlObject* object, const char* patterns) {
    
}

HtmlSearch HtmlNextSearch(HtmlSearch* past) {

}





HtmlObjectArray HtmlSearchObject(HtmlObject* object, const char* patterns) {

}

HtmlObject* HtmlGetObjectChild(HtmlObject* object, const char* patterns) {
    
}






#endif /* _MYHTML_SEARCH_H_ */