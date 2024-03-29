/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include "cyberrt_test/proto/msg.pb.h"

#include "cyber/cyber.h"
#include "cyber/time/rate.h"
#include "cyber/time/time.h"

struct Statis {
    uint64_t count;
    uint64_t total_time;
    std::vector<uint64_t> transmit_time_list;
};

int main(int argc, char** argv) {
    (void)argc;

    apollo::cyber::Init(argv[0]);

    std::string node_name = "n2";
    if (argc > 2) {
        node_name = argv[2];
    }
    auto node = apollo::cyber::CreateNode(node_name);

    struct Statis statis = {};

    auto sender = node->CreateWriter<apollo::cyber::examples::cyberrt_test::proto::Frame>("/apollo/msg2");
    apollo::cyber::ReaderConfig reader_config;
    reader_config.channel_name = "/apollo/msg";
    reader_config.pending_queue_size = 30;
    auto receiver = node->CreateReader<apollo::cyber::examples::cyberrt_test::proto::Frame>(
            reader_config,
            [sender, &statis](const std::shared_ptr<apollo::cyber::examples::cyberrt_test::proto::Frame>& msg) {
                msg->set_read_timestamp(apollo::cyber::Time::Now().ToNanosecond());
                statis.count++;
                statis.total_time += msg->read_timestamp() - msg->write_timestamp();
                statis.transmit_time_list.emplace_back(msg->read_timestamp() - msg->write_timestamp());
                // fprintf(stderr, "seq: %ld\n", msg->seq());
                sender->Write(msg);
            });
    apollo::cyber::WaitForShutdown();

    double sum = std::accumulate(std::begin(statis.transmit_time_list), std::end(statis.transmit_time_list), 0.0);
    double mean = sum / statis.transmit_time_list.size();
    double accum = 0.0;
    std::for_each(std::begin(statis.transmit_time_list), std::end(statis.transmit_time_list), [&](const double d) {
        accum += (d - mean) * (d - mean);
    });
    double variance = accum / (statis.transmit_time_list.size() - 1);
    double stddev = sqrt(variance);
    fprintf(stdout, "count: %lu\n", statis.count);
    fprintf(stdout, "total_time: %lf\n", sum);
    fprintf(stdout, "average_time: %lf\n", mean);
    fprintf(stdout, "variance: %lf\n", variance);
    fprintf(stdout, "standard_deviation: %lf\n", stddev);
    std::sort(statis.transmit_time_list.begin(), statis.transmit_time_list.end());
    // 50
    fprintf(stdout, "50_tantile: %lu\n", statis.transmit_time_list[statis.transmit_time_list.size() * 0.50]);
    // 80
    fprintf(stdout, "80_tantile: %lu\n", statis.transmit_time_list[statis.transmit_time_list.size() * 0.80]);
    // 90
    fprintf(stdout, "90_tantile: %lu\n", statis.transmit_time_list[statis.transmit_time_list.size() * 0.90]);
    // 95
    fprintf(stdout, "95_tantile: %lu\n", statis.transmit_time_list[statis.transmit_time_list.size() * 0.95]);
    // 99
    fprintf(stdout, "99_tantile: %lu\n", statis.transmit_time_list[statis.transmit_time_list.size() * 0.99]);
    return 0;
}
