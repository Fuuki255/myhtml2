#include <stdio.h>

int main(int argc, char** argv) {
    FILE* file = fopen("README.md", "r");

    printf("fputc return %d\n", fseek(file, 20000, SEEK_SET));

    fclose(file);
    return 0;
}