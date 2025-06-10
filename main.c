#include "myhtml2/myhtml.h"
#include <sys/time.h>


int main() {
	HtmlObject* object = HtmlCreateObjectTag("div", "Hello, World!", NULL);

	printf("Object Name: %s\n", object->name);
	printf("Inner Text: %s\n", object->innerText);
	HtmlDestroyObject(object);
	return 0;
}