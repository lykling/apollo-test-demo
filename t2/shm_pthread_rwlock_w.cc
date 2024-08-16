#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include <thread>

#include "common.h"

#include "cyber/base/pthread_rw_lock.h"
#include "cyber/base/rw_lock_guard.h"

union Segment {
    struct {
        union {
            struct {
                uint64_t version_;
                uint64_t block_size_;
                uint64_t block_number_;
                uint64_t block_buffer_size_;
                uint64_t current_block_index_;
            } struct_;
            uint8_t data_[64];
        } meta_;
        uint8_t content_[960];
    } struct_;
    uint8_t data_[1024];
};

union Block {
    struct {
        uint64_t size_;
        std::atomic<uint64_t> writing_ref_count_;
        std::atomic<uint64_t> reading_ref_count_;
        apollo::cyber::base::PthreadRWLock rw_mutex_;
    } struct_;
    uint8_t data_[64];
};

struct Frame {
    uint64_t index_;
    uint64_t timestamp_;
    uint8_t data_[100];
};

uint64_t get_next_avaliable_block(Block* blocks, uint64_t start_index, uint64_t block_number) {
    for (uint64_t index = start_index;; index = (index + 1) % block_number) {
        if (blocks[start_index].struct_.writing_ref_count_ == 0
            && blocks[start_index].struct_.reading_ref_count_ == 0) {
            return index;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void acquire_block_wlock(Block* block) {
    apollo::cyber::base::WriteLockGuard lock(block->struct_.rw_mutex_);
    block->struct_.writing_ref_count_ = 1;
}

void release_block_wlock(Block* block) {
    apollo::cyber::base::WriteLockGuard lock(block->struct_.rw_mutex_);
    block->struct_.writing_ref_count_ = 0;
}

void acquire_block_rlock(Block* block) {
    apollo::cyber::base::ReadLockGuard lock(block->struct_.rw_mutex_);
    block->struct_.reading_ref_count_++;
}

void release_block_rlock(Block* block) {
    apollo::cyber::base::ReadLockGuard lock(block->struct_.rw_mutex_);
    block->struct_.reading_ref_count_--;
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    auto key = std::hash<std::string>{}("/apollo/lab/test");
    void* base_address = (void*)0x710000000000;
    void* shm_address = nullptr;
    uint64_t version = 1;
    uint64_t block_size = 128;
    uint64_t block_number = 8;
    uint64_t block_buffer_size = 128;
    uint64_t current_block_index = 0;

    bool first_created = false;
    apollo::lab::common::shm_open_or_create(key, 4096, base_address, &shm_address, &first_created);
    Segment* segment = reinterpret_cast<Segment*>(shm_address);
    if (first_created) {
        segment->struct_.meta_.struct_.version_ = version;
        segment->struct_.meta_.struct_.block_size_ = block_size;
        segment->struct_.meta_.struct_.block_number_ = block_number;
        segment->struct_.meta_.struct_.block_buffer_size_ = block_buffer_size;
        segment->struct_.meta_.struct_.current_block_index_ = current_block_index;
    } else {
        version = segment->struct_.meta_.struct_.version_;
        block_size = segment->struct_.meta_.struct_.block_size_;
        block_number = segment->struct_.meta_.struct_.block_number_;
        current_block_index = segment->struct_.meta_.struct_.current_block_index_;
    }

    Block* blocks = reinterpret_cast<Block*>(segment->struct_.content_);
    if (first_created) {
        for (uint64_t i = 0; i < block_number; ++i) {
            blocks[i].struct_.size_ = block_size;
            blocks[i].struct_.writing_ref_count_ = 0;
            blocks[i].struct_.reading_ref_count_ = 0;
        }
    }

    uint8_t* buffers = reinterpret_cast<uint8_t*>(blocks) + block_number * sizeof(Block);

    apollo::lab::common::Notifier notifier;
    for (uint64_t index = 0;;) {
        uint64_t block_index = get_next_avaliable_block(blocks, current_block_index, block_number);
        Block* block = blocks + block_index;

        acquire_block_wlock(block);

        // update index
        current_block_index = block_index;
        segment->struct_.meta_.struct_.current_block_index_ = current_block_index;

        // write message
        Frame* frame = reinterpret_cast<Frame*>(buffers + block_index * block_buffer_size);
        frame->index_ = index++;
        frame->timestamp_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count();
        std::sprintf((char*)frame->data_, "Hello, World! %lu", frame->index_);

        release_block_wlock(block);

        // notify
        apollo::lab::common::NotifyEvent event;
        event.struct_.block_index_ = block_index;
        notifier.emit(&event);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return EXIT_SUCCESS;
}
