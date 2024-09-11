#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include <cstdint>
#include <memory.h>
#include "common.h"

void show_bytes(void* addr, size_t len, size_t sep_len, void* last, bool break_line = true) {
    uint8_t* p = reinterpret_cast<uint8_t*>(addr);
    uint8_t* q = reinterpret_cast<uint8_t*>(last);
    for (size_t i = 0; i < len; ++i) {
        fprintf(stdout,
                "\033[%dm%02x\033[0m%c",
                (q && p[i] != q[i]) ? 43 : 0,
                p[i],
                (((i % sep_len) == sep_len - 1) || (i == len - 1)) ? (break_line ? '\n' : ' ')
                                                                   : ((i % 8) == 7 ? ' ' : '\0'));
    }
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    int32_t queue_num = 64;
    auto key = std::hash<std::string>{}("/apollo/lab/__atomic_lock__");
    void* base_address = (void*)0x710000000000;
    void* shm_address = nullptr;

    bool first_created = false;
    apollo::lab::common::shm_open_or_create(key, 4096, base_address, &shm_address, &first_created);
    std::atomic<int32_t>* meta = reinterpret_cast<std::atomic<int32_t>*>(shm_address);
    std::atomic<int32_t>* lock_num = meta + 2;
    if (first_created) {
        meta[0].store(0);
        meta[1].store(queue_num);
        for (int32_t i = 0; i < queue_num; ++i) {
            lock_num[i].store(0);
        }
    }

    uint8_t last[4 * (2 + 64)] = {0};
    memcpy(last, shm_address, 4 * (2 + 64));
    fprintf(stdout, "\033[0;0H\033[2J\033[0;0H");
    for (;;) {
        fprintf(stdout, "\033[0;0H");
        show_bytes(shm_address, 4 * 2, 32, last, true);
        fprintf(stdout, "%8d%8d\n", meta[0].load(), meta[1].load());
        show_bytes(lock_num, 4 * 64, 32, last + sizeof(std::atomic<int32_t>) * 2, true);
        for (int32_t i = 0; i < queue_num; ++i) {
            fprintf(stdout, "%8d%c", lock_num[i].load(), (i % 8 == 7) ? '\n' : (i % 2 == 1) ? ' ' : '\0');
        }
        memcpy(last, shm_address, 4 * 64);
        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }

    return 0;
}
