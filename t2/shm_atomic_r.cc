#include <atomic>
#include <memory>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "common.h"

bool add_read_lock(std::atomic<int32_t>* lock_num) {
    int32_t num = lock_num->load();
    if (num < 0) {
        return false;
    }
    int32_t try_times = 0;
    while (!lock_num->compare_exchange_weak(num, num + 1, std::memory_order_acq_rel, std::memory_order_relaxed)) {
        if (++try_times >= 5) {
            return false;
        }

        num = lock_num->load();
        if (num < 0) {
            return false;
        }
    }
    return true;
}

bool remove_read_lock(std::atomic<int32_t>* lock_num) {
    lock_num->fetch_sub(1);
    return true;
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

    apollo::lab::common::Notifier notifier;
    for (int32_t seq = 0;; ++seq) {
        apollo::lab::common::NotifyEvent event;
        notifier.poll(&event);

        uint64_t block_index = event.struct_.block_index_;
        add_read_lock(lock_num + block_index);

        // do something
        uint8_t buffer[65536];
        for (int32_t i = 0; i < 65536; ++i) {
            buffer[i] = i % 256;
        }

        remove_read_lock(lock_num + (seq % queue_num));
    }

    return 0;
}
