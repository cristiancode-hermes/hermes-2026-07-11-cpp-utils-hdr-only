# cpp_utils — Header-only C++20 Utility Library

[![C++20](https://img.shields.io/badge/C++-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![CMake 3.28+](https://img.shields.io/badge/CMake-3.28%2B-brightgreen)](https://cmake.org)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

**cpp_utils** es una librería header-only moderna en C++20 que proporciona utilidades ligeras y seguras para el día a día del desarrollo en C++: RAII scope guards, type-safe bit flags, manipulación de strings, manejo de errores con `std::expected`, pattern matching para variantes, y traits/concepts útiles.

## ✨ Features

| Componente | Header | Descripción |
|---|---|---|
| **scope_guard** | `scope_guard.hpp` | RAII guards: `scope_guard` (always), `scope_fail` (on exception), `scope_success` (on normal exit) |
| **enum_flags** | `enum_flags.hpp` | Type-safe bit flags para scoped enums con operadores bitwise |
| **str** | `str.hpp` | split, join, trim, to_lower/upper, iequals, replace_all, safe `to_int`/`to_float` con `std::expected` |
| **expected_ext** | `expected_ext.hpp` | `Result<T>` alias, `wrap_except`, `then`/`or_else`, `collect`, `try_all` |
| **overloaded** | `overloaded.hpp` | Pattern-matching visitor para `std::variant` (CTAD) |
| **traits** | `traits.hpp` | Conceptos (Numeric, StringLike, Iterable) + traits (is_specialization, all_same, etc.) |

## 🚀 Quick start

### Prerrequisitos
- CMake 3.28+
- Compilador C++20 (GCC 13+, Clang 16+, MSVC 2022 17.6+)
- Ninja (opcional, recomendado)

### Compilar y ejecutar

```bash
# Configurar (Debug con ASan+UBSan)
cmake --preset debug

# Compilar demo + tests
cmake --build --preset debug

# Ejecutar demo
./build/debug/cpp_utils_demo

# Ejecutar tests
ctest --preset debug
```

### Release

```bash
cmake --preset release
cmake --build --preset release
```

### Docker

```bash
docker build -t cpp-utils .
docker run --rm cpp-utils
```

## 📖 Ejemplos

### scope_guard

```cpp
#include <cpp_utils/scope_guard.hpp>

void process() {
  acquire_resource();
  // Garantiza cleanup siempre
  auto guard = cpp_utils::scope_guard([&] { release_resource(); });

  // Rollback solo si hay excepción
  auto rollback = cpp_utils::scope_fail([&] { undo_changes(); });

  do_work();
  // scope_success confirma al salir normalmente
  auto commit = cpp_utils::scope_success([&] { finalize(); });
}
```

### enum_flags

```cpp
#include <cpp_utils/enum_flags.hpp>

enum class Permission : uint8_t {
  Read = 1, Write = 2, Execute = 4, All = 7
};

cpp_utils::enum_flags<Permission> perms = Permission::Read | Permission::Write;
if (perms & Permission::Read) { /* ... */ }
perms |= Permission::Execute;
perms.clear(Permission::Write);
```

### str utilities

```cpp
#include <cpp_utils/str.hpp>

namespace s = cpp_utils::str;

auto parts = s::split("a,b,c", ',');         // ["a", "b", "c"]
auto joined = s::join(parts, " | ");         // "a | b | c"
auto trimmed = s::trim("  hello  ");         // "hello"
auto upper = s::to_upper("hello");           // "HELLO"
auto num = s::to_int<int>("42");             // Result<int>{42}
```

### expected_ext

```cpp
#include <cpp_utils/expected_ext.hpp>

auto safe_divide = [](int a, int b) -> cpp_utils::Result<int> {
  return cpp_utils::wrap_except([&] {
    if (b == 0) throw std::runtime_error("div by zero");
    return a / b;
  });
};

auto result = cpp_utils::Result<int>{10}
  .and_then([](int v) -> cpp_utils::Result<int> { return v * 2; })
  .and_then([](int v) -> cpp_utils::Result<int> { return v + 5; });
// result == 25
```

## 🧪 Tests

Los tests usan Catch2 v3 e incluyen:
- Tests unitarios para cada componente
- Benchmarks de rendimiento
- Compile-time checks con `STATIC_REQUIRE`

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug --output-on-failure
```

## 📁 Estructura del proyecto

```
2026-07-11-cpp-utils-hdr-only/
├── CMakeLists.txt           # Build principal (C++20, FetchContent)
├── CMakePresets.json        # v9: debug (ASan+UBSan), release (LTO)
├── Dockerfile               # multi-stage gcc:14 → debian slim
├── vcpkg.json               # Manifest mode (opcional)
├── .gitignore
├── .env.example
├── README.md
├── include/
│   ├── cpp_utils.hpp        # Single-include convenience header
│   └── cpp_utils/
│       ├── scope_guard.hpp
│       ├── enum_flags.hpp
│       ├── str.hpp
│       ├── expected_ext.hpp
│       ├── overloaded.hpp
│       └── traits.hpp
├── src/
│   └── main.cpp             # Demo de todas las utilidades
├── tests/
│   ├── CMakeLists.txt
│   └── test_cpp_utils.cpp   # Tests Catch2 completos
└── docs/
    └── architecture.md
```

## 📦 Integración en tu proyecto

### FetchContent (recomendado)

```cmake
include(FetchContent)
FetchContent_Declare(cpp_utils
  GIT_REPOSITORY https://github.com/cristiancode-hermes/hermes-2026-07-11-cpp-utils-hdr-only.git
  GIT_TAG        main
)
FetchContent_MakeAvailable(cpp_utils)
target_link_libraries(your_app PRIVATE cpp_utils)
```

### Copia directa

Copia `include/cpp_utils/` a tu proyecto y añade el directorio a tus include paths. No requiere compilación — es header-only.

## 🛠️ Requisitos técnicos

- **C++20**: concepts, ranges, spans, string_view, std::expected, std::format
- **RAII**: scope_guard usa `std::uncaught_exceptions()` para detección precisa
- **No dependencias externas** (spdlog y nlohmann/json son solo para demo/tests)
- **Zero-cost abstractions**: todo es inlineable, sin allocation dinámica

## 📄 Licencia

MIT
