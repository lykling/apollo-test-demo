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

#include "cyber/examples/cyberrt_test/proto/msg.pb.h"

#include "cyber/cyber.h"
#include "cyber/time/rate.h"
#include "cyber/time/time.h"

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
    // size_t data_len = 4 * 1024 * 1024;
    char* data = (char*)malloc(data_len);
    memset(data, 0xff, sizeof(char) * data_len);
    for (; seq < message_num;) {
        auto msg = std::make_shared<apollo::cyber::examples::cyberrt_test::proto::Frame>();
        msg->set_seq(seq);
        msg->set_text(data);
        // for (size_t i = 0; i < data_len / 2; ++i) {
        //     msg->add_data(1L);
        // }
        msg->set_write_timestamp(apollo::cyber::Time::Now().ToNanosecond());

        sender->Write(msg);
        ++seq;
        rate_ctl.Sleep();
    }
    free(data);
    return 0;
}
