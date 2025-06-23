#include <curl/curl.h>
#include "myhtml2/myhtml.h"


HtmlObject* CreateSampleDocument() {
	// create document
	HtmlObject* document = HtmlCreateObjectDocument();
	
	// document
	HtmlObject* objHtml = HtmlCreateObjectTag(document, "html");
	
	// html
	HtmlObject* objHead = HtmlCreateObjectTag(objHtml, "head");
	HtmlObject* objBody = HtmlCreateObjectTag(objHtml, "body");
	
	// html.head
	HtmlCreateObjectTagEx(objHead, "title", "First Sample of myhtml2", NULL);
	HtmlObject* objMeta = HtmlCreateObjectSingle(objHead, "meta");
	HtmlSetObjectAttribute(objMeta, "charset", "utf-8");

	HtmlCreateObjectComment(objBody, "Hello, World!");
	
	// html.body
	HtmlCreateObjectTag(objHead, "h1");
	
	return document;
}


void PrintObjectStructure(HtmlObject* object) {
	HtmlPrintObjectInfo(object);
	
	HtmlObject* child;
	HtmlForeachObjectChildren(object, child) {
		PrintObjectStructure(child);
	}
}





int main(int argc, char** argv) {
	CURL* curl = curl_easy_init();
	if (curl == NULL) {
		fprintf(stderr, "Error: Failed to initialize CURL.\n");
		return -1;
	}

	HtmlObject* doc = HtmlReadObjectFromCURL(curl, "https://www.example.com");
	printf("Document:\n%s\n\n", HtmlWriteObjectToString(doc));

	HtmlObject* tagBody = HtmlFindObject(doc, "body");

	HtmlStream stream = HtmlCreateStreamBuffer(1024);
	HtmlGetObjectText(doc, &stream);
	printf("Body:\n%s\n", HtmlGetStreamString(&stream));

	HtmlDestroyStream(&stream);
	HtmlDestroyObject(doc);
	return 0;
}




