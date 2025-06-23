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
	HtmlObject* doc = HtmlReadObjectFromFile("Samples/example.html");
	printf("Document:\n%s\n", HtmlWriteObjectToString(doc));

	HtmlObject* result = HtmlFindObject(doc, ".main");
	
	printf("result: %s\n", HtmlWriteObjectToString(result));

	HtmlDestroyObject(doc);
	return 0;
}




