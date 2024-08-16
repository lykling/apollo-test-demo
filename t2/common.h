#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>

namespace apollo {
namespace lab {
namespace common {

bool shm_open(uint64_t key, void* base_address, void** shm_address);

bool shm_open_or_create(uint64_t key, uint64_t size, void* base_address, void** shm_address, bool* is_created);

union NotifyEvent {
    struct {
        uint64_t id_;
        uint64_t timestamp_;
        uint64_t block_index_;
    } struct_;
    uint8_t bytes_[64];
};

union NotifierMeta {
    struct {
        uint64_t version_;
        std::atomic<uint64_t> ref_count_;
        std::atomic<uint64_t> next_seq_;
    } struct_;
    uint8_t bytes_[64];
};

class Notifier {
public:
    static const uint64_t NOTIFY_EVENT_QUEUE_SIZE = 10;
    size_t key_;
    uint64_t next_seq_;
    void* base_address_;
    void* shm_address_;
    NotifierMeta* meta_;
    NotifyEvent* events_;

    Notifier();
    ~Notifier();

    void emit(NotifyEvent* event);
    void poll(NotifyEvent* event);
};

}  // namespace common
}  // namespace lab
}  // namespace apollo
