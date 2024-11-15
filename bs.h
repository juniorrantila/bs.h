/*
 * BSD 2-Clause License
 * 
 * Copyright (c) 2024, Junior Rantila
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include <glob.h>
#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>

typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

static_assert(sizeof(i8) == 1);
static_assert(sizeof(i16) == 2);
static_assert(sizeof(i32) == 4);
static_assert(sizeof(i64) == 8);

static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);

typedef __SIZE_TYPE__ usize;
#define unsigned signed
typedef __SIZE_TYPE__ isize;
#undef unsigned

typedef usize uptr;
typedef isize iptr;

typedef float f32;
typedef double f64;

typedef char const* c_string;

struct StringView {
    StringView() = default;

    static StringView from_c_string(c_string s)
    {
        return StringView(s, strlen(s));
    }

    bool operator==(StringView other) const
    {
        if (m_size != other.m_size) {
            return false;
        }
        if (m_size == 0) {
            return true;
        }
        if (m_data == other.m_data) {
            return true;
        }
        return *m_data == *other.m_data && memcmp(m_data + 1, other.m_data + 1, m_size - 1) == 0;
    }

private:
    StringView(char const* data, usize size)
        : m_data(data)
        , m_size(size)
    {
    }

    char const* m_data { nullptr };
    usize m_size { 0 };
};

template <typename Key, typename Value, u32 capacity>
struct SmallMap {
    SmallMap() = default;

    void append(Key key, Value value)
    {
        usize id = m_size++;
        assert(id <= capacity);
        m_keys[id] = key;
        m_values[id] = value;
    }

    bool has(Key key) const
    {
        for (u32 i = 0; i < m_size; i++) {
            if (m_keys[i] == key)
                return true;
        }
        return false;
    }

private:
    Key m_keys[capacity] {};
    Value m_values[capacity] {};
    usize m_size { 0 };
};

#define MAX_ENTRIES 128

typedef enum {
    TargetKind_Binary,
    TargetKind_Library,
    TargetKind_Targets,
} TargetKind;

struct BinaryArgs;
struct LibraryArgs;
struct Targets;
typedef struct {
    c_string name;
    c_string file;
    c_string base_dir;
    union {
        struct BinaryArgs* binary;
        struct LibraryArgs* library;
        struct Targets* targets;
    };
    TargetKind kind;
} Target;

typedef struct {
    c_string entries[MAX_ENTRIES];
} Strings;

typedef struct Targets {
    Target entries[MAX_ENTRIES];
} Targets;

typedef struct TargetTripple {
    c_string arch;
    c_string abi;
    c_string os;
} TargetTriple;

typedef struct BinaryArgs {
    Strings srcs;
    Strings compile_flags;
    Strings linker_flags;
    TargetTriple target_triple;
    Targets deps;
} BinaryArgs;

typedef struct LibraryArgs {
    Strings srcs;
    Strings exported_headers;
    c_string header_namespace;
    Strings compile_flags;
    Strings linker_flags;
    TargetTriple target_triple;
    c_string link_style;
    Targets deps;
} LibraryArgs;

static inline Strings default_cpp_args(void);
static inline Strings default_c_args(void);
static inline Strings default_objc_args(void);
static inline Strings default_objcpp_args(void);

template <typename T, usize Count>
static inline usize len(T const (& items)[Count]);

template <typename T, usize Count>
static inline usize capacity(T const (& items)[Count]);

template <typename T, usize Count, usize Count2>
static inline void cat(T (&items)[Count], T const (& other)[Count2]);

template <typename F>
static inline auto target(F callback);

static inline Target cpp_binary(c_string name, BinaryArgs args, c_string file = __builtin_FILE());
static inline Target cpp_library(c_string name, LibraryArgs args, c_string file = __builtin_FILE());
static inline Target c_library(c_string name, LibraryArgs args, c_string file = __builtin_FILE());
static inline Target objc_library(c_string name, LibraryArgs args, c_string file = __builtin_FILE());
static inline Target objcpp_library(c_string name, LibraryArgs args, c_string file = __builtin_FILE());
static inline Target cpp_bundle_library(c_string name, LibraryArgs args, c_string file = __builtin_FILE());
static inline Strings glob(c_string name, c_string file = __builtin_FILE());

static inline void emit_ninja(FILE* output, Target target);

static inline void recurse_targets(Target target, void* user, void(*callback)(void* user, Target target));

static inline Targets flatten_targets(Target target);

static inline c_string system_os(void);
static inline c_string system_arch(void);
static inline c_string system_abi(void);

static inline TargetTriple system_target_triple(void);
static inline TargetTriple wasm_target_triple(void);
static inline TargetTriple dynamic_target_tripple(void);
static inline c_string target_triple_string(TargetTriple triple);

static inline void setup(c_string);

template <typename T, usize Count>
usize len(T const (& items)[Count])
{
    usize i = 0;
    for (; i < Count; i++) {
        char zeroes[sizeof(T)];
        memset(zeroes, 0, sizeof(zeroes));
        T item {};
        if (memcmp(&items[i], &item, sizeof(T)) == 0)
            break;
    }
    return i;
}

template <typename T, usize Count>
usize capacity(T const (& items)[Count])
{
    (void)items;
    return Count;
}

template <typename T, usize Count, usize Count2>
static inline void cat(T (&items)[Count], T const (& other)[Count2])
{
    static_assert(Count >= Count2);
    auto c = len(items);
    auto c2 = len(other);
    assert(Count - c > c2);
    memcpy(items + c, &other, sizeof(T) * c2);
}

static inline Targets all_targets_deps;
static inline usize all_targets_count = 0;
static inline Target all_targets = {
    .name = "all",
    .file = "",
    .base_dir = "",
    .targets = &all_targets_deps,
    .kind = TargetKind_Targets,
};
static inline Target cpp_binary(c_string name, BinaryArgs args, c_string file)
{
    c_string base_dir = strdup(dirname(strdup(file)));
    auto* res = (decltype(args)*)malloc(sizeof(args));
    *res = args;
    res->compile_flags = default_cpp_args();
    res->target_triple = system_target_triple();
    cat(res->compile_flags.entries, args.compile_flags.entries);
    Target target = {
        .name = name,
        .file = file,
        .base_dir = base_dir,
        .binary = res,
        .kind = TargetKind_Binary,
    };
    all_targets_deps.entries[all_targets_count++] = target;
    return target;
}

static inline Target cpp_library(c_string name, LibraryArgs args, c_string file)
{
    c_string base_dir = strdup(dirname(strdup(file)));
    auto* res = (decltype(args)*)malloc(sizeof(args));
    *res = args;
    res->compile_flags = default_cpp_args();
    cat(res->compile_flags.entries, args.compile_flags.entries);
    Target target = {
        .name = name,
        .file = file,
        .base_dir = base_dir,
        .library = res,
        .kind = TargetKind_Library,
    };
    all_targets_deps.entries[all_targets_count++] = target;
    return target;
}

static inline Target c_library(c_string name, LibraryArgs args, c_string file)
{
    c_string base_dir = strdup(dirname(strdup(file)));
    auto* res = (decltype(args)*)malloc(sizeof(args));
    *res = args;
    res->compile_flags = default_c_args();
    cat(res->compile_flags.entries, args.compile_flags.entries);
    Target target = {
        .name = name,
        .file = file,
        .base_dir = base_dir,
        .library = res,
        .kind = TargetKind_Library,
    };
    all_targets_deps.entries[all_targets_count++] = target;
    return target;
}

static inline Target objc_library(c_string name, LibraryArgs args, c_string file)
{
    c_string base_dir = strdup(dirname(strdup(file)));
    auto* res = (decltype(args)*)malloc(sizeof(args));
    *res = args;
    res->compile_flags = default_objc_args();
    cat(res->compile_flags.entries, args.compile_flags.entries);
    Target target = {
        .name = name,
        .file = file,
        .base_dir = base_dir,
        .library = res,
        .kind = TargetKind_Library,
    };
    all_targets_deps.entries[all_targets_count++] = target;
    return target;
}

static inline Target objcpp_library(c_string name, LibraryArgs args, c_string file)
{
    c_string base_dir = strdup(dirname(strdup(file)));
    auto* res = (decltype(args)*)malloc(sizeof(args));
    *res = args;
    res->compile_flags = default_objcpp_args();
    cat(res->compile_flags.entries, args.compile_flags.entries);
    Target target = {
        .name = name,
        .file = file,
        .base_dir = base_dir,
        .library = res,
        .kind = TargetKind_Library,
    };
    all_targets_deps.entries[all_targets_count++] = target;
    return target;
}

typedef struct Variable {
    c_string name;
    c_string default_value;
} Variable;

typedef struct TargetRule {
    c_string name;
    c_string command;
    c_string description;

    Variable variables[8];
} TargetRule;

static inline usize all_rules_count = 0;
static inline TargetRule all_rules[64];
static TargetRule ninja_rule(TargetRule rule)
{
    all_rules[all_rules_count++] = rule;
    return rule;
}

static inline TargetRule cxx_rule = ninja_rule({
    .name = "cxx",
    .command = "clang++ -target $target $args -MD -MQ $out -MF $depfile -o $out -c $in",
    .description = "Compiling $language object $out",
    .variables = {
        (Variable){
            .name = "out",
            .default_value = nullptr,
        },
        (Variable){
            .name = "in",
            .default_value = nullptr,
        },
        (Variable){
            .name = "target",
            .default_value = nullptr,
        },
        (Variable){
            .name = "deps",
            .default_value = "gcc",
        },
        (Variable){
            .name = "args",
            .default_value = nullptr,
        },
        (Variable){
            .name = "depfile",
            .default_value = nullptr,
        },
        (Variable){
            .name = "language",
            .default_value = nullptr,
        },
    },
});

typedef struct LanguageExtension {
    c_string name;
    c_string extension;
} LanguageExtension;

static inline c_string language_from_filename(c_string name)
{
    LanguageExtension languages[] = {
        { .name = "C++", .extension = ".cpp" },
        { .name = "C", .extension = ".c" },
        { .name = "Objective-C++", .extension = ".mm" },
        { .name = "Objective-C", .extension = ".m" },
    };
    auto name_len = strlen(name);
    for (usize i = 0; i < capacity(languages); i++) {
        auto language = languages[i];
        auto extension_len = strlen(language.extension);
        if (name_len < extension_len) {
            continue;
        }
        c_string ext = name + name_len - extension_len;
        if (strcmp(ext, language.extension) == 0) {
            return language.name;
        }
    }
    return nullptr;
}

static inline TargetRule merge_object_rule = ninja_rule({
    .name = "merge-object",
    .command = "$ld -r -o $out $in",
    .description = "Linking static target $out",
    .variables = {
        (Variable){
            .name = "out",
            .default_value = nullptr,
        },
        (Variable){
            .name = "in",
            .default_value = nullptr,
        },
    },
});

static inline TargetRule binary_link_rule = ninja_rule({
    .name = "link-binary",
    .command = "clang++ -target $target -o $out $in $link_args",
    .description = "Linking binary target $out",
    .variables = {
        (Variable){
            .name = "out",
            .default_value = nullptr,
        },
        (Variable){
            .name = "in",
            .default_value = nullptr,
        },
        (Variable){
            .name = "target",
            .default_value = nullptr,
        },
        (Variable){
            .name = "link_args",
            .default_value = nullptr,
        },
    },
});

static inline TargetRule header_link_rule = ninja_rule({
    .name = "namespace-header",
    .command = "ln -sf $in $out",
    .description = "Namespacing header $out",
    .variables = {
        (Variable){
            .name = "out",
            .default_value = nullptr,
        },
        (Variable){
            .name = "in",
            .default_value = nullptr,
        },
    },
});

static inline TargetRule compdb_rule = ninja_rule({
    .name = "compdb",
    .command = "ninja -t compdb > compile_commands.json",
    .description = "Emitting compdb",
    .variables = {},
});

static inline void setup(c_string build_dir)
{
    mkdir(build_dir, 0777);
}

static inline void emit_ninja_rule(FILE* output, TargetRule const* rule)
{
    fprintf(output, "rule %s\n", rule->name);
    fprintf(output, "    command = %s\n", rule->command);
    fprintf(output, "    description = %s\n", rule->description);
    usize variable_count = len(rule->variables);
    for (usize i = 0; i < variable_count; i++) {
        auto variable = rule->variables[i];
        if (variable.default_value) {
            fprintf(output, "    %s = %s\n", variable.name, variable.default_value);
        }
    }
    fprintf(output, "\n");
}

static inline void emit_ninja_build_binary(FILE* output, Target const* target)
{
    auto binary = target->binary;
    auto triple = target_triple_string(binary->target_triple);
    auto base_dir = target->base_dir;
    usize srcs_len = len(binary->srcs.entries);
    auto name = target->name;

    auto deps = flatten_targets({
        .name = "__deps__",
        .file = "",
        .base_dir = "",
        .targets = &binary->deps,
        .kind = TargetKind_Targets,
    });
    auto deps_len = len(deps.entries);

    fprintf(output, "build %s/%s: link-binary", triple, name);
    for (usize i = 0; i < srcs_len; i++) {
        auto src = binary->srcs.entries[i];
        fprintf(output, " %s/%s/%s.o", triple, base_dir, src);
    }
    for (usize i = 1; i < deps_len; i++) {
        auto const* dep = &deps.entries[i];
        fprintf(output, " %s/%s.o", triple, dep->name);
    }
    fprintf(output, "\n");
    fprintf(output, "    target = %s\n", triple);
    // fprintf(output, "    link_args = %s\n", triple);
    fprintf(output, "\n");

    auto const* args = &binary->compile_flags;
    auto args_len = len(args->entries);
    for (usize i = 0; i < srcs_len; i++) {
        auto src = binary->srcs.entries[i];
        fprintf(output, "build %s/%s/%s.o: cxx ../%s/%s", triple, base_dir, src, base_dir, src);
        if (deps_len > 1) {
            fprintf(output, " |");
        }
        for (usize dep_index = 1; dep_index < deps_len; dep_index++) {
            auto const* dep = &deps.entries[dep_index];
            if (dep->kind == TargetKind_Library) {
                fprintf(output, " ns/%s/_", dep->library->header_namespace);
            }
        }
        fprintf(output, "\n");

        fprintf(output, "    language = %s\n", language_from_filename(src));
        fprintf(output, "    target = %s\n", triple);
        fprintf(output, "    depfile = %s.%s.d\n", src, triple);
        fprintf(output, "    args =");
        for (usize arg = 0; arg < args_len; arg++) {
            fprintf(output, " %s", args->entries[arg]);
        }
        for (usize dep_index = 1; dep_index < deps_len; dep_index++) {
            auto const* dep = &deps.entries[dep_index];
            if (dep->kind == TargetKind_Library) {
                auto const* library = dep->library;
                c_string ns = library->header_namespace;
                fprintf(output, " -Ins/%s/h", ns);
            }
        }
        fprintf(output, "\n");
        fprintf(output, "\n");
    }
}

static inline void emit_ninja_build_library(FILE* output, Target const* target)
{
    auto library = target->library;
    auto triple = target_triple_string(library->target_triple);
    auto base_dir = target->base_dir;
    usize srcs_len = len(library->srcs.entries);
    usize headers_len = len(library->exported_headers.entries);
    auto name = target->name;

    for (usize i = 0; i < headers_len; i++) {
        auto header = library->exported_headers.entries[i];
        char* out = nullptr;
        asprintf(&out, "%s/%s", base_dir, header);
        c_string header_path = realpath(out, 0);
        fprintf(output, "build ns/%s/h/%s/%s: namespace-header %s\n", name, name, header, header_path);
    }
    fprintf(output, "\n");
    fprintf(output, "build ns/%s/_: phony", library->header_namespace);
    for (usize i = 0; i < headers_len; i++) {
        auto header = library->exported_headers.entries[i];
        fprintf(output, " ns/%s/h/%s/%s", name, name, header);
    }
    fprintf(output, "\n\n");
    
    fprintf(output, "build %s/%s.o: merge-object ", triple, name);
    for (usize i = 0; i < srcs_len; i++) {
        auto src = library->srcs.entries[i];
        fprintf(output, "%s/%s/%s.o", triple, base_dir, src);
        if (i != srcs_len - 1) {
            fprintf(output, " ");
        }
    }
    fprintf(output, "\n");
    fprintf(output, "    ld = ld\n");
    fprintf(output, "\n");

    auto deps = flatten_targets({
        .name = "__deps__",
        .file = "",
        .base_dir = "",
        .targets = &library->deps,
        .kind = TargetKind_Targets,
    });
    auto deps_len = len(deps.entries);

    auto const* args = &library->compile_flags;
    auto args_len = len(args->entries);
    for (usize i = 0; i < srcs_len; i++) {
        auto src = library->srcs.entries[i];
        fprintf(output, "build %s/%s/%s.o: cxx ../%s/%s", triple, base_dir, src, base_dir, src);
        if (deps_len > 1) {
            fprintf(output, " |");
        }
        for (usize dep_index = 1; dep_index < deps_len; dep_index++) {
            auto const* dep = &deps.entries[dep_index];
            if (dep->kind == TargetKind_Library) {
                fprintf(output, " ns/%s/_", dep->library->header_namespace);
            }
        }
        fprintf(output, "\n");

        fprintf(output, "    language = %s\n", language_from_filename(src));
        fprintf(output, "    target = %s\n", triple);
        fprintf(output, "    depfile = %s.%s.d\n", src, triple);
        fprintf(output, "    args =");
        for (usize arg = 0; arg < args_len; arg++) {
            fprintf(output, " %s", args->entries[arg]);
        }
        for (usize dep_index = 1; dep_index < deps_len; dep_index++) {
            auto const* dep = &deps.entries[dep_index];
            if (dep->kind == TargetKind_Library) {
                auto const* library = dep->library;
                c_string ns = library->header_namespace;
                fprintf(output, " -Ins/%s/h", ns);
            }
        }
        fprintf(output, "\n");
        fprintf(output, "\n");
    }
}

static inline void emit_ninja(FILE* output, Target target)
{
    fprintf(output, "ninja_required_version = 1.8.2\n\n");

    for (usize i = 0; i < all_rules_count; i++) {
        emit_ninja_rule(output, &all_rules[i]);
    }

    fprintf(output, "build compile_commands.json: compdb\n\n");

    Targets targets = flatten_targets(target);
    usize targets_len = len(targets.entries);
    for (usize i = 0; i < targets_len; i++) {
        auto const* target = &targets.entries[i];
        switch (target->kind) {
        case TargetKind_Binary:
            emit_ninja_build_binary(output, target);
            break;
        case TargetKind_Library:
            emit_ninja_build_library(output, target);
            break;
        case TargetKind_Targets:
            break;
        }
    }
}

static inline Strings glob(c_string name, c_string file)
{
    c_string dir = realpath(dirname(strdup(file)), 0);
    char* glob_str;
    assert(asprintf(&glob_str, "%s/%s", dir, name) >= 0);
    Strings result = {};
    glob_t g = {};
    int res = ::glob(glob_str, 0, 0, &g);
    if (res != 0) {
        fprintf(stderr, "WARNING: could not match glob: '%s'\n", name);
        return result;
    }
    assert(g.gl_pathc < capacity(result.entries));
    auto dir_len = strlen(dir);
    for (usize i = 0; i < g.gl_pathc; i++) {
        c_string p = g.gl_pathv[i];
        p += dir_len + 1;
        result.entries[i] = p;
    }
    return result;
}

template <typename F>
static inline auto target(F callback) {
    return callback();
}

static inline Targets const* target_deps(Target const* target)
{
    switch (target->kind) {
    case TargetKind_Binary: return &target->binary->deps;
    case TargetKind_Library: return &target->library->deps;
    case TargetKind_Targets: return target->targets;
    }
}

static inline void recurse_targets(Target target, void* user, void(*callback)(void* user, Target target))
{
    auto seen = SmallMap<StringView, Target, 128>();
    auto rec = [&](auto rec_cb, Target target) {
        auto name = StringView::from_c_string(target.name);
        if (seen.has(name)) {
            return;
        }
        seen.append(name, target);
        callback(user, target);
        auto* deps = target_deps(&target);
        auto deps_len = len(deps->entries);
        for (usize i = 0; i < deps_len; i++) {
            rec_cb(rec_cb, deps->entries[i]);
        }
    };
    rec(rec, target);
}

static inline Targets flatten_targets(Target target)
{
    struct Context {
        usize i = 0;
        Targets result = {};
    } context {};
    recurse_targets(target, &context, [](void* context, Target target) {
        auto* self = ((Context*)context);
        self->result.entries[self->i++] = target;
    });

    return context.result;
}

static inline Strings default_cxx_args(void)
{
    return (Strings){
        "-Wall",
        "-Wextra",
        "-fcolor-diagnostics",
    };
}

static inline Strings default_cpp_args(void)
{
    auto args = default_cxx_args();
    args.entries[len(args.entries)] = "-std=c++17";
    return args;
}

static inline Strings default_c_args(void)
{
    auto args = default_cxx_args();
    args.entries[len(args.entries)] = "-std=11";
    args.entries[len(args.entries)] = "-xc";
    return args;
}

static inline Strings default_objc_args(void)
{
    auto args = default_c_args();
    args.entries[len(args.entries)] = "-xobjc";
    return args;
}

static inline Strings default_objcpp_args(void)
{
    auto args = default_cpp_args();
    args.entries[len(args.entries)] = "-xobjc++";
    return args;
}

static inline c_string system_os(void)
{
#if __APPLE__
    return "macos";
#elif __linux__
    return "linux";
#elif _WIN32
    return "windows";
#error "unknown os"
    return 0;
#endif
}

static inline c_string system_arch(void)
{
#if __aarch64__
    return "aarch64";
#elif __x86_64__
    return "x86_64";
#elif __i386__
    return "i386";
#else
#error "unknown architecture"
    return 0;
#endif
}

static inline c_string system_abi(void)
{
#ifdef __APPLE__
    return "none";
#elif __linux__
    return "gnu";
#elif _WIN32
    return "windows";
#else
#error "unknown abi"
    return 0;
#endif
}

static inline TargetTriple system_target_triple(void)
{
    return (TargetTriple) {
        .arch = system_arch(),
        .abi = system_abi(),
        .os = system_os(),
    };
}

static inline TargetTriple wasm_target_triple(void)
{
    return (TargetTriple) {
        .arch = "wasm32",
        .abi = nullptr,
        .os = nullptr,
    };
}

static inline c_string target_triple_string(TargetTriple triple)
{
    usize len = 0;
    c_string arch = triple.arch ? triple.arch : "unknown";
    c_string abi = triple.abi ? triple.abi : "unknown";
    c_string os = triple.os ? triple.os : "unknown";
    len += strlen(arch) + 1;
    len += strlen(abi) + 1;
    len += strlen(os) + 1;
    char* dest = (char*)calloc(len, 1);
    snprintf(dest, len, "%s-%s-%s", arch, abi, os);
    return dest;
}

