#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    if (argc < 3) {
        return 1;
    }

    if (strlen(argv[1]) == 0) {
        return 1;
    }

    FILE* fp = fopen(argv[2], "rb");
    if (!fp) {
        return 1;
    }

    unsigned char buffer[1024];
    size_t totalBytes = 0;
    size_t bytes;

    printf("unsigned char %s[] = {", argv[1]);
    const size_t lineLength = 16;
    while((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        for (size_t i = 0; i < bytes; i ++) {
            if (i % lineLength == 0) {
                printf("\n");
            }
            printf(" 0x%02x,", buffer[i]);
        }
        totalBytes += bytes;
    }

    printf("\n 0x00};\n"
           "unsigned int %s_len = %lu;\n", argv[1], totalBytes);

    fclose(fp);

    return 0;
}
