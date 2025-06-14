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
	HtmlObject* objTitle = HtmlCreateObjectTagEx(objHead, "title", "First Sample of myhtml2", NULL);
	HtmlObject* objMeta = HtmlCreateObjectSingle(objHead, "meta");
	HtmlSetObjectAttribute(objMeta, "charset", "utf-8");
	HtmlObject* objComment = HtmlCreateObjectComment(objBody, "Hello, World!");
	
	// html.body
	HtmlObject* objH1 = HtmlCreateObjectTag(objHead, "h1");
	
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
	HtmlObject* document = CreateSampleDocument();

	HtmlStream stream = HtmlCreateStreamStringBuffered(128);
	HtmlWriteObjectToStream(document, &stream);

	HtmlStringStream* streamData = (HtmlStringStream*)stream.data;
	streamData->buffer[streamData->length] = '\0'; // Null-terminate the string
	printf("HTML Output:\n%s\n", ((HtmlStringStream*)stream.data)->buffer);
	
	HtmlDestroyStream(&stream);
	HtmlDestroyObject(document);
	return 0;
}




