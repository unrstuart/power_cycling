cc_binary(
    name = "cycling",
    deps = [":main"],
)

cc_library(
    name = "measurement",
    srcs = ["measurement.cc"],
    hdrs = ["measurement.h"],
    deps = [":si_var"],
)

cc_library(
    name = "si_base_unit",
    srcs = ["si_base_unit.cc"],
    hdrs = ["si_base_unit.h"],
)

cc_library(
    name = "si_unit",
    srcs = ["si_unit.cc"],
    hdrs = ["si_unit.h"],
    deps = [":si_base_unit"],
)

cc_library(
    name = "si_var",
    srcs = ["si_var.cc"],
    hdrs = ["si_var.h"],
    deps = [
        ":si_base_unit",
        ":si_unit",
    ]
)

cc_library(
    name = "time_sample",
    srcs = ["time_sample.cc"],
    hdrs = ["time_sample.h"],
    deps = [
        ":measurement",
        ":si_var",
    ],
)

cc_library(
    name = "time_series",
    srcs = ["time_series.cc"],
    hdrs = ["time_series.h"],
    deps = [":time_sample"],
)

cc_library(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        ":si_var",
        ":si_unit",
    ]
)

cc_library(
    name = "gtest",
    srcs = [
        "gtest-death-test.cc",
        "gtest-filepath.cc",
        "gtest-port.cc",
        "gtest-printers.cc",
        "gtest-test-part.cc",
        "gtest-typed-test.cc",
        "gtest.cc",
    ],
    hdrs = [
        "gtest-internal-inl.h",
        "gtest/gtest-death-test.h",
        "gtest/gtest-message.h",
        "gtest/gtest-param-test.h",
        "gtest/gtest-printers.h",
        "gtest/gtest-spi.h",
        "gtest/gtest-test-part.h",
        "gtest/gtest-typed-test.h",
        "gtest/gtest.h",
        "gtest/gtest_pred_impl.h",
        "gtest/gtest_prod.h",
        "gtest/internal/custom/gtest-port.h",
        "gtest/internal/custom/gtest-printers.h",
        "gtest/internal/custom/gtest.h",
        "gtest/internal/gtest-death-test-internal.h",
        "gtest/internal/gtest-filepath.h",
        "gtest/internal/gtest-internal.h",
        "gtest/internal/gtest-linked_ptr.h",
        "gtest/internal/gtest-param-util-generated.h",
        "gtest/internal/gtest-param-util.h",
        "gtest/internal/gtest-port-arch.h",
        "gtest/internal/gtest-port.h",
        "gtest/internal/gtest-string.h",
        "gtest/internal/gtest-tuple.h",
        "gtest/internal/gtest-type-util.h",
		],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "gtest_main",
    srcs = [
        "gtest-death-test.cc",
        "gtest-filepath.cc",
        "gtest-port.cc",
        "gtest-printers.cc",
        "gtest-test-part.cc",
        "gtest-typed-test.cc",
        "gtest.cc",
        "gtest_main.cc",
    ],
    hdrs = [
        "gtest-internal-inl.h",
        "gtest/gtest-death-test.h",
        "gtest/gtest-message.h",
        "gtest/gtest-param-test.h",
        "gtest/gtest-printers.h",
        "gtest/gtest-spi.h",
        "gtest/gtest-test-part.h",
        "gtest/gtest-typed-test.h",
        "gtest/gtest.h",
        "gtest/gtest_pred_impl.h",
        "gtest/gtest_prod.h",
        "gtest/internal/custom/gtest-port.h",
        "gtest/internal/custom/gtest-printers.h",
        "gtest/internal/custom/gtest.h",
        "gtest/internal/gtest-death-test-internal.h",
        "gtest/internal/gtest-filepath.h",
        "gtest/internal/gtest-internal.h",
        "gtest/internal/gtest-linked_ptr.h",
        "gtest/internal/gtest-param-util-generated.h",
        "gtest/internal/gtest-param-util.h",
        "gtest/internal/gtest-port-arch.h",
        "gtest/internal/gtest-port.h",
        "gtest/internal/gtest-string.h",
        "gtest/internal/gtest-tuple.h",
        "gtest/internal/gtest-type-util.h",
		],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "gmock_main",
    srcs = [
        "gmock-cardinalities.cc",
        "gmock-internal-utils.cc",
        "gmock-matchers.cc",
        "gmock-spec-builders.cc",
        "gmock.cc",
        "gmock_main.cc",
    ],
    hdrs = [
        "gmock/gmock-actions.h",
        "gmock/gmock-cardinalities.h",
        "gmock/gmock-generated-actions.h",
        "gmock/gmock-generated-function-mockers.h",
        "gmock/gmock-generated-matchers.h",
        "gmock/gmock-generated-nice-strict.h",
        "gmock/gmock-matchers.h",
        "gmock/gmock-more-actions.h",
        "gmock/gmock-more-matchers.h",
        "gmock/gmock-spec-builders.h",
        "gmock/gmock.h",
        "gmock/internal/custom/gmock-generated-actions.h",
        "gmock/internal/custom/gmock-matchers.h",
        "gmock/internal/custom/gmock-port.h",
        "gmock/internal/gmock-generated-internal-utils.h",
        "gmock/internal/gmock-internal-utils.h",
        "gmock/internal/gmock-port.h",
    ],
		deps = [
				":gtest",
		],
    linkopts = ["-pthread"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "si_base_unit_test",
    srcs = ["si_base_unit_test.cc"],
    deps = [
        ":si_base_unit",
        ":gmock_main",
    ],
)

cc_test(
    name = "si_unit_tesT",
    srcs = ["si_unit_test.cc"],
    deps = [
        ":si_unit",
        ":si_base_unit",
        ":gtest_main",
    ],
)
