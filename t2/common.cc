#include "common.h"

#include <errno.h>
#include <memory.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

namespace apollo {
namespace lab {
namespace common {

bool shm_open(uint64_t key, void* base_address, void** shm_address) {
    int shmid = shmget(static_cast<key_t>(key), 0, 0);
    if (shmid == -1) {
        // open failed
        return false;
    }
    *shm_address = shmat(shmid, base_address, 0);
    if (*shm_address == reinterpret_cast<void*>(-1)) {
        // shmat failed
        return false;
    }
    return true;
}

bool shm_open_or_create(uint64_t key, uint64_t size, void* base_address, void** shm_address, bool* is_created) {
    int shmid = shmget(static_cast<key_t>(key), size, 0644 | IPC_CREAT | IPC_EXCL);
    if (shmid == -1) {
        if (errno == EINVAL) {
            // TODO: recreate with larger size
        } else if (errno == EEXIST) {
            if (is_created) {
                *is_created = false;
            }
            return shm_open(key, base_address, shm_address);
        } else {
            return false;
        }
    }
    *shm_address = shmat(shmid, base_address, 0);
    if (*shm_address == reinterpret_cast<void*>(-1)) {
        // shmat failed
        return false;
    }
    if (is_created) {
        *is_created = true;
    }
    return true;
}

Notifier::Notifier() {
    key_ = std::hash<std::string>{}("/apollo/lab/notifier");
    base_address_ = reinterpret_cast<void*>(0x720000000000);
    shm_address_ = nullptr;
    bool first_created = false;
    shm_open_or_create(key_, 2048, base_address_, &shm_address_, &first_created);

    meta_ = reinterpret_cast<NotifierMeta*>(shm_address_);
    if (first_created) {
        meta_->struct_.version_ = 1;
        meta_->struct_.next_seq_ = 0;
    }
    next_seq_ = meta_->struct_.next_seq_.load();

    events_ = reinterpret_cast<NotifyEvent*>(meta_ + 1);
}

Notifier::~Notifier() {
    meta_->struct_.ref_count_.fetch_sub(1);
    if (meta_->struct_.ref_count_.load() == 0) {
        shmdt(shm_address_);
    }
}

void Notifier::emit(NotifyEvent* event) {
    uint64_t seq = meta_->struct_.next_seq_.fetch_add(1);
    uint64_t index = seq % NOTIFY_EVENT_QUEUE_SIZE;

    events_[index].struct_.id_ = seq;
    events_[index].struct_.timestamp_ = std::chrono::system_clock::now().time_since_epoch().count();
    events_[index].struct_.block_index_ = event->struct_.block_index_;
}

void Notifier::poll(NotifyEvent* event) {
    for (;;) {
        uint64_t seq = meta_->struct_.next_seq_.load();
        if (seq != next_seq_) {
            uint64_t index = next_seq_ % NOTIFY_EVENT_QUEUE_SIZE;
            if (events_[index].struct_.id_ >= next_seq_) {
                next_seq_ = events_[index].struct_.id_;
                memcpy(event, &events_[index], sizeof(NotifyEvent));
                ++next_seq_;
                return;
            }
        }

        std::this_thread::sleep_for(std::chrono::microseconds(50));
    }
}

}  // namespace common
}  // namespace lab
}  // namespace apollo
