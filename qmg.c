#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define QM 0x4D51
#define BM 0x4D42

#pragma pack(push, 1)

typedef struct {
    uint16_t magic;
    uint32_t version;
    uint16_t width;
    uint16_t height;
    uint16_t unknown1;
    uint16_t unknown2;
    uint16_t unknown3;
    uint16_t total;
    uint16_t frame;
    uint16_t duration;
    uint16_t repeat;
    uint32_t size;
    uint32_t fullsize;
} QMG;

typedef struct {
    uint16_t type;
    uint32_t filesize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
    uint32_t headersize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits;
    uint32_t compression;
    uint32_t imagesize;
    int32_t xppm;
    int32_t yppm;
    uint32_t palette;
    uint32_t significant;
} BMP;

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

    QMG *current = (QMG *)buffer;

    while ((uintptr_t)current < (uintptr_t)(buffer + size)) {
        printf("%d/%d\n", current->frame, current->total);

        if (current->magic != QM) {
            fprintf(stderr, "Malformed QMG header!\n");
            return 1;
        }

        size_t extra = 0;
        QMG *next = (QMG *)((char *)current + current->size);

        while (next->magic != QM && (uintptr_t)next < (uintptr_t)(buffer + size))
            next = (QMG *)((char *)current + current->size + ++extra);

        current->size -= sizeof(QMG);

        char *body = malloc(current->size);
        memcpy(body, (char *)current + sizeof(QMG), current->size);

        char *footer = malloc(extra);
        char *palette = malloc((extra / 2) * 3);
        memcpy(footer, (char *)current + sizeof(QMG) + current->size, extra);

        for (size_t i = 0; i < extra; i += 2) {
            uint16_t rgb565;
            memcpy(&rgb565, footer + i, 2);
            palette[i / 2] = (((rgb565 >> 11) & 0x1F) * 255 + 15) / 31;
            palette[i / 2 + 1] = (((rgb565 >> 5) & 0x3F) * 255 + 31) / 63;
            palette[i / 2 + 2] = ((rgb565 & 0x1F) * 255 + 15) / 31;
        }

        BMP bmp;
        bmp.type = BM;
        bmp.filesize = sizeof(BMP) + current->width * current->height * 3;
        bmp.reserved1 = 0;
        bmp.reserved2 = 0;
        bmp.offset = sizeof(BMP);
        bmp.headersize = 40;
        bmp.width = current->width;
        bmp.height = -current->height;
        bmp.planes = 1;
        bmp.bits = 24;
        bmp.compression = 0;
        bmp.imagesize = current->width * current->height * 3;
        bmp.xppm = 2835;
        bmp.yppm = 2835;
        bmp.palette = 0;
        bmp.significant = 0;

        char *frame = malloc(current->width * current->height * 3);

        for (int y = 0; y < current->height; y++) {
            for (int x = 0; x < current->width; x++) {
                memcpy(&frame[(y * current->width + x) * 3], &palette[extra / 2], 3);
            }
        }

        char filename[16];
        sprintf(filename, "output/%d.bmp", current->frame);
        FILE *output = fopen(filename, "wb");
        fwrite(&bmp, sizeof(BMP), 1, output);
        fwrite(frame, 3, current->width * current->height, output);
        fclose(output);

        free(body);
        free(footer);
        free(palette);
        free(frame);

        current = (QMG *)((char *)current + current->size + extra + sizeof(QMG));
    }

    free(buffer);

    return 0;
}