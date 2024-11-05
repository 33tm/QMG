#include "qmg.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <qmg> <audio?>" << endl;
        return 1;
    }

    ifstream qmg(argv[1], ios::binary);

    if (!qmg.is_open()) {
        cerr << "Failed to open " << argv[1] << endl;
        return 1;
    }

    if (argc == 3 && !filesystem::exists(argv[2])) {
        cerr << argv[2] << " does not exist!" << endl;
        return 1;
    }

#ifdef _WIN32
    int ffmpeg = !system("where ffmpeg > nul 2>&1");
#else
    int ffmpeg = !system("which ffmpeg > /dev/null 2>&1");
#endif

    if (!ffmpeg) {
        cerr << "ffmpeg not found!" << endl;
        return 1;
    }

    uintmax_t size = filesystem::file_size(argv[1]);
    vector<char> buffer(size);
    qmg.read(buffer.data(), static_cast<streamsize>(size));
    qmg.close();

    if (filesystem::exists("output"))
        filesystem::remove_all("output");
    filesystem::create_directory("output");

    char *current = buffer.data();

    while (true) {
        QMG *header = reinterpret_cast<QMG *>(current);

        if (header->magic != QM) {
            cerr << "Malformed QMG!" << endl;
            return 1;
        }

        cout << header->frame << "/" << header->total << endl;

        char *next = current + header->size;
        while (reinterpret_cast<QMG *>(next)->magic != QM && next < buffer.data() + size)
            next++;

        vector<char> body(current + sizeof(QMG), current + header->size);
        vector<char> footer(current + header->size, next);

        vector<RGB888> palette(footer.size() / 2);
        for (size_t i = 0; i < footer.size(); i += 2) {
            uint16_t rgb565 = (static_cast<uint8_t>(footer[i + 1]) << 8) | static_cast<uint8_t>(footer[i]);
            palette[i / 2].r = ((rgb565 >> 11 & 0x1F) * 527 + 23) >> 6;
            palette[i / 2].g = ((rgb565 >> 5 & 0x3F) * 259 + 33) >> 6;
            palette[i / 2].b = ((rgb565 & 0x1F) * 527 + 23) >> 6;
        }

        ofstream colors("output/" + to_string(header->frame) + ".html");
        for (size_t i = 0; i < palette.size(); i++) {
            colors << "<div style=\"background-color: #" << hex << uppercase << setfill('0') << setw(2)
                   << static_cast<uint16_t>(palette[i].r) << setfill('0') << setw(2)
                   << static_cast<uint16_t>(palette[i].g) << setfill('0') << setw(2)
                   << static_cast<uint16_t>(palette[i].b) << "\">" << dec << i << "</div>";
        }

        auto *frame = new RGB888[header->width * header->height];
        if (palette.empty()) {
            fill(frame, frame + header->width * header->height, RGB888{0, 0, 0});
        } else {
            for (size_t i = 0; i < header->width; i++) {
                for (size_t j = 0; j < header->height; j++) {
                    frame[j * header->width + i] = palette[i % palette.size()];
                }
            }
        }
        bmp(header->width, header->height, frame, "output/" + to_string(header->frame) + ".bmp");
        delete[] frame;

        if (header->frame == header->total)
            break;

        current = next;
    }

    if (argc == 3)
        system(("ffmpeg -r 12 -i output/%d.bmp -i " + string(argv[2]) +
                " -c:a copy -pix_fmt yuv420p -loglevel error output.mp4").c_str());
    else
        system("ffmpeg -r 12 -i output/%d.bmp -pix_fmt yuv420p -loglevel error output.mp4");

    return 0;
}