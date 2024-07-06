#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define QM 0x4D51

typedef struct Header {
    uint16_t magic;
    uint8_t major, minor, patch;
    uint8_t repeat;
    uint16_t width, height;
    uint16_t unknown; // same phone same value
    uint16_t unknown2;
    unsigned char v1[2];
    uint16_t total, current;
    uint32_t duration;
    uint32_t size;
    uint32_t size2;
} Header;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <qmg>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");

    if (!file) {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(size);
    fread(buffer, 1, size, file);
    fclose(file);

    size_t offset = 0;

    while (1) {
        Header *header = (Header *)(buffer + offset);

        if (header->magic != QM) {
            fprintf(stderr, "Malformed QMG Header!\n");
            return 1;
        }

        size_t length = header->size;
        Header *next = (Header *)(buffer + offset + length);

        if (header->current != header->total) {
            while (next->magic != QM) {
                length++;
                next = (Header *)(buffer + offset + length);
            }
        }

        length -= sizeof(Header);

        char *body = malloc(length);
        memcpy(body, buffer + offset + sizeof(Header), length);

        free(body);

        if (header->current == header->total) break;

        offset += length + sizeof(Header);
    }

    free(buffer);

    return 0;
}