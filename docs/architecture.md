# Architecture — cpp_utils

## Design decisions

### Header-only
- **Por qué**: simplifica integración (sin compilación, sin linking, sin ABI)
- **Cómo**: todo es `inline` o definido en templates; las funciones libres son `inline`
- **Excepciones**: solo `scope_fail`/`scope_success` usan `std::uncaught_exceptions()` para diferenciar entre salida normal y excepcional

### Zero-overhead principle
- `enum_flags` es un wrapper sobre `uint8_t`/`int` — mismo layout que flags manuales
- `str::split` devuelve `std::vector<std::string_view>` — no copia los tokens
- `overloaded` es un struct vacío con herencia variádica — el compilador lo optimiza por completo

### Error handling con std::expected
- C++23 trae `std::expected` al estándar; en C++20 vía `std::experimental::expected` o el backport
- Se prefiere `Result<T>` sobre excepciones para APIs donde el error es un caso esperado (parseo, validación)
- `Error` es un struct simple con code + message, sin jerarquía de herencia

### C++20 features utilizados

| Feature | Componente | Propósito |
|---|---|---|
| `concepts` | `traits.hpp` | `Numeric`, `StringLike`, `Iterable` |
| `std::ranges` | `str.hpp` | `trim`, `to_lower/upper`, `iequals` (algoritmos sobre ranges) |
| `std::span` | `str.hpp` | `join(span<string>)` sin copia |
| `std::string_view` | Todo | Zero-copy string handling |
| `std::expected` | `expected_ext.hpp` | Error handling sin excepciones |
| `std::format` | `expected_ext.hpp` | `Error::at()` con source_location |
| `CTAD` | `overloaded.hpp`, `scope_guard.hpp` | Deducción automática de tipos |
| `std::source_location` | `expected_ext.hpp` | Ubicación precisa de errores |

## Data flow

```
┌─────────────┐     ┌──────────────┐     ┌──────────────┐
│  User Code  │────▶│  cpp_utils   │────▶│  STL / std   │
└─────────────┘     └──────────────┘     └──────────────┘
                            │
                    ┌───────┴───────┐
                    │               │
                    ▼               ▼
             scope_guard       enum_flags
                    │               │
                    ▼               ▼
             uncaught_exceptions  bitwise ops
```

## Dependency graph

```
cpp_utils.hpp
  ├── overloaded.hpp   (no deps besides <variant>)
  ├── scope_guard.hpp  (no deps besides <exception>)
  ├── enum_flags.hpp   (no deps besides <type_traits>)
  ├── str.hpp          (<string>, <string_view>, <ranges>, <expected>)
  ├── expected_ext.hpp (<expected>, <source_location>)
  └── traits.hpp       (<type_traits>, <concepts>, <ranges>)
```

## Why not Boost?

- **Peso**: Boost.Any, Boost.LEAF, Boost.Outcome son órdenes de magnitud más grandes
- **Compilación**: Boost aumenta el tiempo de compilación significativamente
- **C++20**: la STL ya proporciona la mayoría de estas utilidades (std::expected, std::format, concepts)
- **Propósito**: cpp_utils es para proyectos pequeños/medianos que necesitan lo justo sin overhead
