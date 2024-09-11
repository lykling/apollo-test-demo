#include <atomic>
#include <memory>
#include <string>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "common.h"

bool add_write_lock(std::atomic<int32_t>* lock_num) {
    int32_t expected = 0;
    return lock_num->compare_exchange_weak(expected, -1, std::memory_order_acq_rel, std::memory_order_relaxed);
}

bool remove_write_lock(std::atomic<int32_t>* lock_num) {
    lock_num->fetch_add(1);
    return true;
}

int32_t get_next_index(void* addr) {
    auto seq = reinterpret_cast<std::atomic<int32_t>*>(addr);
    auto num = reinterpret_cast<std::atomic<int32_t>*>((uint8_t*)addr + sizeof(std::atomic<int32_t>))->load();
    for (;;) {
        int32_t next_idx = seq->fetch_add(1) % num;
        if (add_write_lock(reinterpret_cast<std::atomic<int32_t>*>(addr) + 2 + next_idx)) {
            return next_idx;
        }
    }
    return 0;
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

    apollo::lab::common::Rate rate(5000);
    apollo::lab::common::Notifier notifier;
    for (int32_t seq = 0;; ++seq) {
        int32_t next_idx = get_next_index(meta);

        uint8_t buffer[65536];
        for (int32_t i = 0; i < 65536; ++i) {
            buffer[i] = i % 256;
        }

        remove_write_lock(lock_num + next_idx);

        apollo::lab::common::NotifyEvent event;
        event.struct_.block_index_ = next_idx;
        notifier.emit(&event);

        rate.sleep();
    }
    return 0;
}
