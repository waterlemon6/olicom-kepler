#include <stdio.h>
#include <zconf.h>
#include <stdlib.h>
#include <string.h>
#include "UpdateImageCompress.h"

int main()
{
    struct imageHeader ImageHeader = {0};
    struct sectionHeader *Section;
    unsigned char *input[10];
    FILE *output = fopen("update.img", "wb");

    printf("Hello, World!\n");
    ImageHeader.magic = UPDATE_MAGIC_NUMBER;
    ImageHeader.length = sizeof(struct imageHeader);
    ImageHeader.sectionCount = 0;

    input[0] = ReadOneSection(&ImageHeader, "Distance", UPDATE_MARK_DISTANCE_EXE);
    if(input[0] == NULL)
        return -2;
    printf("Read Section 0\n");

    input[1] = ReadOneSection(&ImageHeader, "setup_in_advance.sh", UPDATE_MARK_SHELL);
    if(input[1] == NULL)
        return -2;
    printf("Read Section 1\n");

    input[2] = ReadOneSection(&ImageHeader, "led.ko", UPDATE_MARK_LED_MODULE);
    if(input[2] == NULL)
        return -2;
    printf("Read Section 2\n");

    input[3] = ReadOneSection(&ImageHeader, "printer_device.ko", UPDATE_MARK_PR_DEVICE_MODULE);
    if(input[3] == NULL)
        return -2;
    printf("Read Section 2\n");

    fwrite(&ImageHeader, sizeof(struct imageHeader), 1, output);
    for (int i = 0; i < ImageHeader.sectionCount; i++) {
        Section = (struct sectionHeader *)input[i];
        fwrite(input[i], Section->length, 1, output);
        free(input[i]);
        printf("Write Section\n");
    }
    fflush(output);
    sync();
    fclose(output);
    printf("Goodbye, Cruel World.\n");

    // CRC Start.
    unsigned int length = 0;
    unsigned char *buffer = NULL;
    struct imageHeader *Image = NULL;
    FILE *stream1 = fopen("update.img", "rb");
    FILE *stream2 = fopen("update_plus.img", "wb");
    if (stream1 == NULL) {
        printf("Open File Failed.\n");
        return -1;
    }
    fseek(stream1, 0, SEEK_END);
    length = (unsigned int) ftell(stream1);
    fseek(stream1, 0, SEEK_SET);
    buffer = malloc(length);
    fread(buffer, length, 1, stream1);

    Image = (struct imageHeader *)buffer;
    Image->check = GetCRC32CheckSum(buffer + sizeof(struct imageHeader), length - sizeof(struct imageHeader));
    fwrite(buffer, length, 1 ,stream2);

    fflush(stream2);
    sync();
    fclose(stream2);
    printf("CRC Process.\n");
    // CRC End.

    return 0;
}