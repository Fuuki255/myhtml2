#include "MyHtml/MyHtml.h"
#include <sys/time.h>

void PrintHtmlStructure(HTML_Element* element, int spaceCount) {
	char spaces[spaceCount + 1];
	memset(spaces, ' ', spaceCount);
	spaces[spaceCount] = 0;

	HTML_ForeachElementChildren(element, child) {
		if (HTML_Type_HasFlags(child->type, HTML_HAS_NAME)) {
			printf("%s%s ", spaces, child->name);
		}
		putchar('\n');

		PrintHtmlStructure(child, spaceCount + 4);
	}
}

int CountElements(HTML_Element* element) {
	int total = 0;

	HTML_ForeachElementChildren(element, child) {
		if (HTML_Type_HasFlags(child->type, HTML_HAS_CHILD)) {
			total += CountElements(child);
		}
		total++;
	}
	return total;
}


int main() {
	struct timeval st, ed;

	char buffer[1024 * 1024];
	FILE* file = fopen("youtube.html", "r");
	fread(buffer, 1, 1024 * 1023, file);
	fclose(file);

	printf("buffer size: %lu\n", strlen(buffer));

	gettimeofday(&st, NULL);
	HTML_Element* doc = HTML_ParseString(buffer);
	gettimeofday(&ed, NULL);

	printf("usetime: %lu us\n", (ed.tv_sec - st.tv_sec) * 1000000 + ed.tv_usec - st.tv_usec);

	printf("element count: %d\n", CountElements(doc));

	HTML_WriteElementToFileObject(doc, stdout);
	// PrintHtmlStructure(doc, 0);

	// fprintf(stderr, "Document:\n%s\n", result);
	// free(result);

	printf("Destroy Element!\n");
	HTML_DestroyElement(doc);
	return 0;
}