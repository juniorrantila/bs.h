# bs.h

**bs.h** is a header-only build system for C/C++.

## Basic usage

```cpp
#include "bs.h"

static auto const hello = target([]{
    return cpp_library("Hello", {
        .srcs = {
            "Hello.cpp",
        },
        .exported_headers = {
            "Hello.h",
        },
        .header_namespace = "Hello",
        .compile_flags = {},
        .linker_flags = {},
        .target_triple = system_target_triple(),
        .link_style = "static",
        .deps = {},
    });
});

static auto const example = target([]{
    return cpp_binary("example", {
        .srcs = {
            "main.cpp",
        },
        .compile_flags = {},
        .linker_flags = {},
        .target_triple = system_target_triple(),
        .deps = {
            hello,
        },
    });
});

int main()
{
    setup("build");
    FILE* ninja = fopen("build/build.ninja", "w");
    if (!ninja) {
        perror("could not open build/build.ninja");
        return 1;
    }
    emit_ninja(ninja, all_targets);
    fclose(ninja);
    return 0;
}

```
