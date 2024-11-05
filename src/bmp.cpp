#include "qmg.hpp"

#include <fstream>

using namespace std;

void bmp(uint16_t width, uint16_t height, RGB888 *data, const string& filename) {
    BMP bmp;
    bmp.type = BM;
    bmp.filesize = sizeof(BMP) + width * height * 3;
    bmp.reserved1 = 0;
    bmp.reserved2 = 0;
    bmp.offset = sizeof(BMP);
    bmp.headersize = 40;
    bmp.width = width;
    bmp.height = -height;
    bmp.planes = 1;
    bmp.bits = 24;
    bmp.compression = 0;
    bmp.imagesize = width * height * 3;
    bmp.xppm = 2835;
    bmp.yppm = 2835;
    bmp.palette = 0;
    bmp.significant = 0;

    ofstream output(filename, ios::binary);
    output.write(reinterpret_cast<char *>(&bmp), sizeof(BMP));
    output.write(reinterpret_cast<char *>(data), 3 * width * height);
    output.close();
}