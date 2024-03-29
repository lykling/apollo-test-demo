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
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <vector>

#include "cyberrt_test/proto/msg.pb.h"

#include "cyber/cyber.h"
#include "cyber/time/rate.h"
#include "cyber/time/time.h"
#include "cyber/common/global_data.h"

uint32_t random_uint32() {
    return rand();
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;

    apollo::cyber::Init(argv[0]);
    // argv[1] --> message number
    size_t message_num = std::stol(argv[1]);
    // argv[2] --> rate
    float rate = std::stof(argv[2]);
    // argv[3] --> data_size
    size_t data_len = std::stol(argv[3]);
    // argv[4] --> flag_using_bytes
    long flag_using_bytes = 0;
    if (argc > 4) {
        flag_using_bytes = std::stol(argv[4]);
    }

    auto config = apollo::cyber::common::GlobalData::Instance()->Config();
    fprintf(stderr, "config: %s\n", config.DebugString().c_str());

    auto node = apollo::cyber::CreateNode("n1");

    apollo::cyber::proto::RoleAttributes attrs;
    attrs.set_channel_name("/apollo/msg");
    auto qos = attrs.mutable_qos_profile();
    qos->set_history(apollo::cyber::proto::QosHistoryPolicy::HISTORY_KEEP_LAST);
    qos->set_reliability(apollo::cyber::proto::QosReliabilityPolicy::RELIABILITY_RELIABLE);
    qos->set_durability(apollo::cyber::proto::QosDurabilityPolicy::DURABILITY_TRANSIENT_LOCAL);
    auto sender = node->CreateWriter<apollo::cyber::examples::cyberrt_test::proto::Frame>(attrs);

    // sleep a while for initialization, aboout 2 seconds
    apollo::cyber::Rate rate_init(0.5);
    rate_init.Sleep();

    apollo::cyber::Rate rate_ctl(rate);
    uint64_t seq = 0;

    // points data
    std::vector<apollo::cyber::examples::cyberrt_test::proto::Point> points;
    for (size_t i = 0; i < data_len; ++i) {
        apollo::cyber::examples::cyberrt_test::proto::Point point;
        point.set_x(1039.1139);
        point.set_y(1039.1139);
        point.set_z(1039.1139);
        point.set_i(1);
        point.set_t(1657324662872342528);
        points.emplace_back(point);
    }

    // float * 3 + uint32 * 1 + uint64 * 1 = 24 bytes
    char* data = (char*)malloc(data_len * sizeof(char) * 24);
    memset(data, 0x00, sizeof(char) * data_len * 24);
    srand(time(NULL));
    for (size_t i = 0; i < data_len; ++i) {
        // data[i * 4] = rand() % 0xff;
        // data[i * 4 + 1] = rand() % 0xff;
        // data[i * 4 + 2] = rand() % 0xff;
        // data[i * 4 + 3] = rand() % 0xff;
        // data[i * 4] = 0xff;
        // data[i * 4 + 1] = 0xff;
        // data[i * 4 + 2] = 0xff;
        // data[i * 4 + 3] = 0xff;
        *(float*)(data + i * 24 + 0) = 1039.1139;
        *(float*)(data + i * 24 + 4) = 1039.1139;
        *(float*)(data + i * 24 + 8) = 1039.1139;
        *(uint32_t*)(data + i * 24 + 12) = 0x0f;
        *(uint64_t*)(data + i * 24 + 16) = 1657324662872342528;
    }

    for (; seq < message_num;) {
        auto msg = std::make_shared<apollo::cyber::examples::cyberrt_test::proto::Frame>();
        msg->set_seq(seq);
        if (flag_using_bytes == 1) {
            msg->set_text(std::string(data, data_len * 24));
        } else if (flag_using_bytes == 2) {
            for (size_t i = 0; i < data_len; ++i) {
                msg->add_points()->CopyFrom(points[i]);
            }
        } else {
            for (size_t i = 0; i < data_len * 6; ++i) {
                msg->add_data(*(uint32_t*)(data + i * 4));
            }
        }
        msg->set_write_timestamp(apollo::cyber::Time::Now().ToNanosecond());

        sender->Write(msg);
        ++seq;
        rate_ctl.Sleep();
    }
    free(data);
    return 0;
}
