#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define QM 0x4D51

#pragma pack(push, 1)

typedef struct Header {
    uint16_t magic;
    uint32_t version;
    uint16_t width;
    uint16_t height;
    uint16_t unknown1;
    uint16_t unknown2;
    uint16_t unknown3;
    uint16_t total;
    uint16_t current;
    uint16_t duration;
    uint16_t repeat;
    uint32_t size;
    uint32_t fullsize;
} Header;

#pragma pack(pop)

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
    Header *header = (Header *)buffer;

    char *framebuffer = malloc(header->width * header->height);

    while (header->current != header->total) {
        header = (Header *)(buffer + offset);

        if (header->magic != QM) {
            fprintf(stderr, "Malformed QMG Header!\n");
            return 1;
        }

        header->size -= sizeof(Header);

        char *base = buffer + offset + sizeof(Header);

        size_t extra = 0;
        Header *next = (Header *)(base + header->size + extra);

        while (next->magic != QM && offset + sizeof(Header) + header->size + extra < size)
            next = (Header *)(base + header->size + ++extra);

        char *body = malloc(header->size);
        memcpy(body, base, header->size);

        char *palette = malloc(extra);
        memcpy(palette, base + header->size, extra);

        free(body);
        free(palette);

        offset += header->size + extra + sizeof(Header);
    }

    free(framebuffer);
    free(buffer);

    return 0;
}