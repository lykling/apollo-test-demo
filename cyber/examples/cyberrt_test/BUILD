load("//tools:cpplint.bzl", "cpplint")
load("//tools:apollo_package.bzl", "apollo_cc_binary", "apollo_package")

package(default_visibility = ["//visibility:public"])

apollo_cc_binary(
    name = "n1",
    srcs = ["n1.cc"],
    deps = [
        "//cyber",
        "//cyber/examples/cyberrt_test/proto:msg_proto",
    ],
)

apollo_cc_binary(
    name = "n2",
    srcs = ["n2.cc"],
    deps = [
        "//cyber",
        "//cyber/examples/cyberrt_test/proto:msg_proto",
    ],
)

apollo_cc_binary(
    name = "n3",
    srcs = ["n3.cc"],
    deps = [
        "//cyber",
        "//cyber/examples/cyberrt_test/proto:msg_proto",
    ],
)

apollo_package()

cpplint()
