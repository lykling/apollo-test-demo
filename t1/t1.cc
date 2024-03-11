#include <cstdio>
#include <cstdlib>
#include <string>
#include "t1.pb.h"

int main(int argc, char** argv) {
    size_t size = 4 * 1024 * 1024;
    char* data = (char*)malloc(size);
    memset(data, 0xff, sizeof(char) * size);
    srand(time(NULL));
    for (size_t i = 0; i < size / 4; ++i) {
        data[i * 4] = rand() % 0xff;
        data[i * 4 + 1] = rand() % 0xff;
        data[i * 4 + 2] = rand() % 0xff;
        data[i * 4 + 3] = rand() % 0xff;
    }

    t1::Frame frame1;
    frame1.set_data(std::string(data, size));
    fprintf(stdout, "frame1 size: %ld\n", frame1.ByteSizeLong());

    t1::Frame frame2;
    for (size_t i = 0; i < size / 4; ++i) {
        // frame2.add_data2(*(uint32_t*)(data + i * 4));
        // fprintf(stdout, "[%03u] frame2.add_data2(%u)\n", i * 4, *((uint32_t*)(data + (i * 4))));
        // for (size_t j = 0; j < 4; ++j) {
        //     fprintf(stdout, "[%03u] data[%u]: %02x\n", i * 4 + j, i * 4 + j, data[i * 4 + j]);
        // }
        frame2.add_data2(*(uint32_t*)(data + i * 4));
    }
    fprintf(stdout, "frame2 size: %ld\n", frame2.ByteSizeLong());

    free(data);

    // char aa[4] = {0x00, 0x00, 0x00, 0x01};
    // fprintf(stdout, "aa: %d\n", *(uint32_t*)aa);
    // char bb[4] = {0x01, 0x00, 0x00, 0x00};
    // fprintf(stdout, "bb: %d\n", *(uint32_t*)bb);

    return 0;
}
