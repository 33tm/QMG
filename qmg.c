#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#define QM 0x4D51

typedef struct Header {
    uint16_t magic;
    unsigned char v0[4];
    uint16_t width, height;
    unsigned char v1[6];
    uint16_t total, current;
    unsigned char v2[4];
    uint32_t size;
    unsigned char v3[4];
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

    char *buffer = malloc(size * sizeof(char));
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

        char *body = malloc((length - sizeof(Header)) * sizeof(char));
        char *frame = malloc(header->width * header->height * sizeof(char));
        memcpy(body, buffer + offset + sizeof(Header), length - sizeof(Header));

        free(body);
        free(frame);

        if (header->current == header->total) break;

        offset += length;
    }

    free(buffer);

    return 0;
}