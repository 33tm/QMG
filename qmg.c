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

typedef struct {
    uint8_t b;
    uint8_t g;
    uint8_t r;
} RGB888;

#pragma pack(pop)

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <qmg> <audio>\n", argv[0]);
        return 1;
    }

    FILE *qmg = fopen(argv[1], "rb");
    FILE *audio = fopen(argv[2], "r");

    if (!qmg) {
        fprintf(stderr, "Failed to open %s\n", argv[1]);
        return 1;
    }

    if (!audio) {
        fprintf(stderr, "Failed to open %s\n", argv[2]);
        return 1;
    }

    fclose(audio);

    fseek(qmg, 0, SEEK_END);
    size_t size = ftell(qmg);
    fseek(qmg, 0, SEEK_SET);

    char *buffer = malloc(size);
    fread(buffer, 1, size, qmg);
    fclose(qmg);

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

        uint8_t *body = malloc(current->size);
        memcpy(body, (char *)current + sizeof(QMG), current->size);

        char *filenametemp;
        asprintf(&filenametemp, "output/%d.txt", current->frame);
        FILE *frametemp = fopen(filenametemp, "w");
        for (int i = 0; i < current->size; i++) {
            if (body[i] == 0xff || body[i] == 0x7F || body[i] == 0xF7) {
                if (body[i] == 0xff) fwrite(".", 1, 1, frametemp);
                if (body[i] == 0x7f || body[i] == 0xf7) fwrite("\n", 1, 1, frametemp);
            }
            else fwrite("x", 1, 1, frametemp);
        }

        fwrite("\n\n", 2, 1, frametemp);

        free(filenametemp);

        uint8_t *footer = malloc(extra);
        RGB888 *palette = malloc(extra / 2 * sizeof(RGB888));
        memcpy(footer, (char *)current + sizeof(QMG) + current->size, extra);

        char *qmgfntemp;
        asprintf(&qmgfntemp, "output/%d.qmg", current->frame);
        FILE *qmgtemp = fopen(qmgfntemp, "wb");

        fwrite(current, sizeof(QMG), 1, qmgtemp);
        fwrite(body, current->size, 1, qmgtemp);
        fwrite(footer, extra, 1, qmgtemp);

        fclose(qmgtemp);
        free(qmgfntemp);

        free(body);

        for (size_t i = 0; i < extra; i += 2) {
            uint16_t rgb565 = *(uint16_t *)(&footer[i]);
            palette[i / 2].b = (rgb565 & 0x1F) * 527 + 23 >> 6;
            palette[i / 2].g = (rgb565 >> 5 & 0x3F) * 259 + 33 >> 6;
            palette[i / 2].r = (rgb565 >> 11 & 0x1F) * 527 + 23 >> 6;
        }

        for (int i = 0; i < extra / 2; i++) {
            RGB888 c = palette[i];
            char *temp;
            asprintf(&temp, "#%02x%02x%02x\n", c.r, c.g, c.b);
            fwrite(temp, 8, 1, frametemp);
            free(temp);
        }

        fclose(frametemp);

        free(footer);

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

        RGB888 *frame = malloc(current->width * current->height * sizeof(RGB888));

        for (int y = 0; y < current->height; y++) {
            for (int x = 0; x < current->width; x++) {
                RGB888 *pixel = &frame[y * current->width + x];
                *pixel = extra ? palette[0] : (RGB888) { 0, 0, 0 };
            }
        }

        free(frame);
        free(palette);

        char *filename;
        asprintf(&filename, "output/%d.bmp", current->frame);
        FILE *output = fopen(filename, "wb");
        free(filename);

        fwrite(&bmp, sizeof(BMP), 1, output);
        fwrite(frame, 3, current->width * current->height, output);
        fclose(output);

        current = (QMG *)((char *)current + current->size + extra + sizeof(QMG));
    }

    free(buffer);

    char *ffmpeg;
    asprintf(
        &ffmpeg,
        "ffmpeg -r 12 -i 'output/%%d.bmp' -i %s -c:a copy -pix_fmt yuv420p -loglevel error output.mp4",
        argv[2]
    );

    system(ffmpeg);

    free(ffmpeg);

    return 0;
}