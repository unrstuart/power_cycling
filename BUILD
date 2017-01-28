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
    name = "main",
    srcs = ["main.cc"],
    deps = [
        ":si_var",
        ":si_unit",
    ]
)

cc_binary(
    name = "cycling",
    deps = [":main"],
)
