#pragma once

#include <type_traits>
#include <concepts>
#include <cstdint>

namespace cpp_utils {

// ── enum_flags: Type-safe bit flags for scoped enums ──
//
// Usage:
//   enum class Color : uint8_t { Red = 1, Green = 2, Blue = 4 };
//   enum_flags<Color> flags{Color::Red | Color::Blue};
//   if (flags & Color::Red) { ... }
//   flags |= Color::Green;

template <typename E>
  requires std::is_enum_v<E> && std::is_integral_v<std::underlying_type_t<E>>
class enum_flags {
  using U = std::underlying_type_t<E>;

public:
  // ── Constructors ──
  constexpr enum_flags() noexcept : value_(static_cast<U>(0)) {}
  constexpr enum_flags(E flag) noexcept : value_(static_cast<U>(flag)) {}
  constexpr enum_flags& operator=(E flag) noexcept { value_ = static_cast<U>(flag); return *this; }

  // ── Bitwise OR (combine) ──
  constexpr friend enum_flags operator|(enum_flags a, enum_flags b) noexcept {
    return enum_flags(static_cast<E>(static_cast<U>(a.value_) | static_cast<U>(b.value_)));
  }
  constexpr enum_flags& operator|=(enum_flags other) noexcept {
    value_ = static_cast<U>(static_cast<U>(value_) | static_cast<U>(other.value_));
    return *this;
  }
  constexpr enum_flags& operator|=(E flag) noexcept {
    value_ = static_cast<U>(static_cast<U>(value_) | static_cast<U>(flag));
    return *this;
  }

  // ── Bitwise AND (test) ──
  constexpr friend enum_flags operator&(enum_flags a, enum_flags b) noexcept {
    return enum_flags(static_cast<E>(static_cast<U>(a.value_) & static_cast<U>(b.value_)));
  }
  constexpr friend enum_flags operator&(enum_flags a, E b) noexcept {
    return enum_flags(static_cast<E>(static_cast<U>(a.value_) & static_cast<U>(b)));
  }
  constexpr friend bool operator&(E a, enum_flags b) noexcept {
    return (static_cast<U>(a) & static_cast<U>(b.value_)) != 0;
  }

  // ── Bitwise XOR ──
  constexpr friend enum_flags operator^(enum_flags a, enum_flags b) noexcept {
    return enum_flags(static_cast<E>(static_cast<U>(a.value_) ^ static_cast<U>(b.value_)));
  }
  constexpr enum_flags& operator^=(enum_flags other) noexcept {
    value_ = static_cast<U>(static_cast<U>(value_) ^ static_cast<U>(other.value_));
    return *this;
  }

  // ── Bitwise NOT ──
  constexpr friend enum_flags operator~(enum_flags f) noexcept {
    return enum_flags(static_cast<E>(~f.value_));
  }

  // ── Equality ──
  constexpr friend bool operator==(enum_flags a, enum_flags b) noexcept = default;

  // ── Boolean conversion (for `if (flags)` idiom) ──
  constexpr explicit operator bool() const noexcept { return value_ != 0; }

  // ── Raw access ──
  constexpr U raw() const noexcept { return value_; }

  // ── Test if ALL bits of `flag` are set ──
  constexpr bool all(E flag) const noexcept {
    return (value_ & static_cast<U>(flag)) == static_cast<U>(flag);
  }

  // ── Test if ANY bit of `flag` is set ──
  constexpr bool any(E flag) const noexcept {
    return (value_ & static_cast<U>(flag)) != 0;
  }

  // ── Set/clear/toggle specific flag ──
  constexpr void set(E flag) noexcept { value_ |= static_cast<U>(flag); }
  constexpr void clear(E flag) noexcept { value_ &= ~static_cast<U>(flag); }
  constexpr void toggle(E flag) noexcept { value_ ^= static_cast<U>(flag); }
  constexpr void clear_all() noexcept { value_ = static_cast<U>(0); }

private:
  explicit constexpr enum_flags(U raw) noexcept : value_(raw) {}

  U value_;
};

// ── Free operator| for enum + enum → enum_flags ──
// Provides the convenient `Color::Red | Color::Blue` syntax.
// NOTE: `using cpp_utils::operator|;` must be in scope at the call site
// because ADL alone won't find it for enums not in the cpp_utils namespace.
template <typename E>
  requires std::is_enum_v<E>
constexpr enum_flags<E> operator|(E a, E b) noexcept {
  return enum_flags<E>{a} | enum_flags<E>{b};
}

template <typename E>
  requires std::is_enum_v<E>
constexpr enum_flags<E> operator|(enum_flags<E> a, E b) noexcept {
  return a | enum_flags<E>{b};
}

template <typename E>
  requires std::is_enum_v<E>
constexpr enum_flags<E> operator|(E a, enum_flags<E> b) noexcept {
  return enum_flags<E>{a} | b;
}

// ── Free operator^ for enum + enum → enum_flags ──
template <typename E>
  requires std::is_enum_v<E>
constexpr enum_flags<E> operator^(E a, E b) noexcept {
  return enum_flags<E>{a} ^ enum_flags<E>{b};
}

} // namespace cpp_utils
