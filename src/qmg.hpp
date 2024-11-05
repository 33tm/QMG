#ifndef QMG_H
#define QMG_H

#include <cstdint>
#include <string>

#define QM 0x4D51
#define BM 0x4D42

#pragma pack(push, 1)

struct QMG {
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
};

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

void bmp(uint16_t width, uint16_t height, RGB888 *data, const std::string& filename);

#endif