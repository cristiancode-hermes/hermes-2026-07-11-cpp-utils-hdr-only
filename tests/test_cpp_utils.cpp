#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include <cpp_utils/scope_guard.hpp>
#include <cpp_utils/enum_flags.hpp>
#include <cpp_utils/str.hpp>
#include <cpp_utils/expected_ext.hpp>
#include <cpp_utils/overloaded.hpp>
#include <cpp_utils/traits.hpp>

#include <variant>
#include <vector>
#include <string>
#include <stdexcept>
#include <type_traits>

// ── Bring enum_flags operators into scope for the test ──
// This makes operator|(E, E) findable via unqualified lookup
using cpp_utils::operator|;
using cpp_utils::operator^;

// ──────────────────────────────────────────────
// scope_guard tests
// ──────────────────────────────────────────────

TEST_CASE("make_scope_guard runs on scope exit", "[scope_guard]") {
  int counter = 0;
  {
    auto g = cpp_utils::make_scope_guard([&] { ++counter; });
    REQUIRE(counter == 0);
  }
  REQUIRE(counter == 1);
}

TEST_CASE("make_scope_guard dismiss prevents execution", "[scope_guard]") {
  int counter = 0;
  {
    auto g = cpp_utils::make_scope_guard([&] { ++counter; });
    g.dismiss();
  }
  REQUIRE(counter == 0);
}

TEST_CASE("make_scope_fail does not run on normal exit", "[scope_guard]") {
  int counter = 0;
  {
    auto g = cpp_utils::make_scope_fail([&] { counter = -1; });
  }
  REQUIRE(counter == 0);
}

TEST_CASE("make_scope_fail runs on exception", "[scope_guard]") {
  int counter = 0;
  try {
    auto g = cpp_utils::make_scope_fail([&] { counter = 99; });
    throw std::runtime_error("test");
  } catch (...) {}
  REQUIRE(counter == 99);
}

TEST_CASE("make_scope_success runs on normal exit", "[scope_guard]") {
  int counter = 0;
  {
    auto g = cpp_utils::make_scope_success([&] { ++counter; });
  }
  REQUIRE(counter == 1);
}

TEST_CASE("make_scope_success does not run on exception", "[scope_guard]") {
  int counter = 0;
  try {
    auto g = cpp_utils::make_scope_success([&] { counter = -1; });
    throw std::runtime_error("test");
  } catch (...) {}
  REQUIRE(counter == 0);
}

// ──────────────────────────────────────────────
// enum_flags tests
// ──────────────────────────────────────────────

enum class Color : uint8_t {
  Red   = 1 << 0,
  Green = 1 << 1,
  Blue  = 1 << 2,
  All   = 0x07
};

TEST_CASE("enum_flags default constructed is empty", "[enum_flags]") {
  cpp_utils::enum_flags<Color> flags;
  REQUIRE_FALSE(flags);
  REQUIRE(flags.raw() == 0);
}

TEST_CASE("enum_flags single flag", "[enum_flags]") {
  cpp_utils::enum_flags<Color> flags{Color::Red};
  REQUIRE(flags & Color::Red);
  REQUIRE_FALSE(flags & Color::Green);
}

TEST_CASE("enum_flags operator| combines flags", "[enum_flags]") {
  // operator|(E, E) found via `using cpp_utils::operator|` above
  auto flags = Color::Red | Color::Blue;
  STATIC_REQUIRE(std::is_same_v<decltype(flags), cpp_utils::enum_flags<Color>>);
  REQUIRE(flags & Color::Red);
  REQUIRE(flags & Color::Blue);
  REQUIRE_FALSE(flags & Color::Green);
}

TEST_CASE("enum_flags |= adds flags", "[enum_flags]") {
  cpp_utils::enum_flags<Color> flags{Color::Red};
  flags |= Color::Green;
  bool ok = flags.any(Color::Red) && flags.any(Color::Green) && !flags.any(Color::Blue);
  REQUIRE(ok);
}

TEST_CASE("enum_flags clear removes flags", "[enum_flags]") {
  cpp_utils::enum_flags<Color> flags{Color::All};
  flags.clear(Color::Red);
  REQUIRE_FALSE(flags & Color::Red);
  REQUIRE(flags & Color::Green);
  REQUIRE(flags & Color::Blue);
}

TEST_CASE("enum_flags toggle flips flags", "[enum_flags]") {
  cpp_utils::enum_flags<Color> flags{Color::Red};
  flags.toggle(Color::Red);
  REQUIRE_FALSE(flags & Color::Red);
  flags.toggle(Color::Red);
  REQUIRE(flags & Color::Red);
}

TEST_CASE("enum_flags all() tests all bits set", "[enum_flags]") {
  cpp_utils::enum_flags<Color> flags{Color::Red | Color::Green};
  REQUIRE_FALSE(flags.all(Color::All));
  flags |= Color::Blue;
  REQUIRE(flags.all(Color::All));
}

// ──────────────────────────────────────────────
// str utilities tests
// ──────────────────────────────────────────────

TEST_CASE("str::split by char", "[str]") {
  namespace s = cpp_utils::str;
  auto parts = s::split("a,b,c", ',');
  REQUIRE(parts.size() == 3);
  REQUIRE(parts[0] == "a");
  REQUIRE(parts[1] == "b");
  REQUIRE(parts[2] == "c");
}

TEST_CASE("str::split empty string", "[str]") {
  namespace s = cpp_utils::str;
  auto parts = s::split("", ',');
  REQUIRE(parts.size() == 0);
}

TEST_CASE("str::split by string delimiter", "[str]") {
  namespace s = cpp_utils::str;
  auto parts = s::split("a::b::c", "::");
  REQUIRE(parts.size() == 3);
  REQUIRE(parts[0] == "a");
  REQUIRE(parts[2] == "c");
}

TEST_CASE("str::split_n limits parts", "[str]") {
  namespace s = cpp_utils::str;
  auto parts = s::split_n("a,b,c,d", ',', 3);
  REQUIRE(parts.size() == 3);
  REQUIRE(parts[2] == "c,d");
}

TEST_CASE("str::trim removes whitespace", "[str]") {
  namespace s = cpp_utils::str;
  REQUIRE(s::trim("  hello  ") == "hello");
  REQUIRE(s::trim("no-trim") == "no-trim");
  REQUIRE(s::trim("") == "");
  REQUIRE(s::trim("   ") == "");
}

TEST_CASE("str::to_lower and to_upper", "[str]") {
  namespace s = cpp_utils::str;
  REQUIRE(s::to_lower("Hello World") == "hello world");
  REQUIRE(s::to_upper("Hello World") == "HELLO WORLD");
}

TEST_CASE("str::iequals", "[str]") {
  namespace s = cpp_utils::str;
  REQUIRE(s::iequals("Hello", "hELLo"));
  REQUIRE(s::iequals("", ""));
  REQUIRE_FALSE(s::iequals("Hello", "World"));
}

TEST_CASE("str::replace_all", "[str]") {
  namespace s = cpp_utils::str;
  auto r = s::replace_all("one two one three", "one", "X");
  REQUIRE(r == "X two X three");
  r = s::replace_all("hello", "nonexistent", "X");
  REQUIRE(r == "hello");
}

TEST_CASE("str::join", "[str]") {
  namespace s = cpp_utils::str;
  std::vector<std::string> v{"a", "b", "c"};
  REQUIRE(s::join(v, ", ") == "a, b, c");
  REQUIRE(s::join(std::vector<std::string>{}, ",") == "");
}

TEST_CASE("str::to_int valid input", "[str]") {
  namespace s = cpp_utils::str;
  auto r = s::to_int<int>("42");
  REQUIRE(r.has_value());
  REQUIRE(*r == 42);
}

TEST_CASE("str::to_int invalid input", "[str]") {
  namespace s = cpp_utils::str;
  auto r = s::to_int<int>("not-a-number");
  REQUIRE_FALSE(r.has_value());
}

TEST_CASE("str::to_float valid input", "[str]") {
  namespace s = cpp_utils::str;
  auto r = s::to_float<double>("3.14");
  REQUIRE(r.has_value());
  REQUIRE(*r == 3.14);
}

TEST_CASE("str::to_float invalid input", "[str]") {
  namespace s = cpp_utils::str;
  auto r = s::to_float<double>("not-a-number");
  REQUIRE_FALSE(r.has_value());
}

TEST_CASE("str::contains_any and contains_all", "[str]") {
  namespace s = cpp_utils::str;
  std::string_view text = "hello world foo bar";
  std::string_view needles[] = {"foo", "xyz"};
  REQUIRE(s::contains_any(text, needles));
  std::string_view all_needles[] = {"hello", "world"};
  REQUIRE(s::contains_all(text, all_needles));
}

// ──────────────────────────────────────────────
// expected_ext tests
// ──────────────────────────────────────────────

TEST_CASE("Result type alias", "[expected_ext]") {
  cpp_utils::Result<int> good{42};
  REQUIRE(good.has_value());
  REQUIRE(*good == 42);

  cpp_utils::Result<int> bad{std::unexpected(cpp_utils::Error{"fail"})};
  REQUIRE_FALSE(bad.has_value());
  REQUIRE(bad.error().message == "fail");
}

TEST_CASE("Error from errc", "[expected_ext]") {
  cpp_utils::Error e{std::errc::invalid_argument};
  REQUIRE(e.code > 0);
  REQUIRE_FALSE(e.message.empty());
}

TEST_CASE("Error from source_location", "[expected_ext]") {
  auto e = cpp_utils::Error::at("test error");
  REQUIRE(e.code == 1);
  // Split the || check into a bool to avoid Catch2 chained comparison issue
  bool found_in_filename = e.message.find("test_main.cpp") != std::string_view::npos
                        || e.message.find("test_cpp_utils.cpp") != std::string_view::npos;
  REQUIRE(found_in_filename);
}

TEST_CASE("and_then chaining", "[expected_ext]") {
  auto r = cpp_utils::Result<int>{10}
    .and_then([](int v) -> cpp_utils::Result<int> { return v * 2; })
    .and_then([](int v) -> cpp_utils::Result<int> { return v + 5; });
  REQUIRE(r.has_value());
  REQUIRE(*r == 25);
}

TEST_CASE("and_then short-circuits on error", "[expected_ext]") {
  auto r = cpp_utils::Result<int>{std::unexpected(cpp_utils::Error{"fail"})}
    .and_then([](int v) -> cpp_utils::Result<int> { return v * 2; });
  REQUIRE_FALSE(r.has_value());
}

TEST_CASE("wrap_except catches exceptions", "[expected_ext]") {
  auto safe_div = [](int a, int b) -> cpp_utils::Result<int> {
    return cpp_utils::wrap_except([&] {
      if (b == 0) throw std::runtime_error("div by zero");
      return a / b;
    });
  };
  REQUIRE(safe_div(10, 2).has_value());
  REQUIRE(*safe_div(10, 2) == 5);
  REQUIRE_FALSE(safe_div(10, 0).has_value());
}

TEST_CASE("Result collect", "[expected_ext]") {
  auto r = cpp_utils::collect(
    cpp_utils::Result<int>{1},
    cpp_utils::Result<double>{2.5},
    cpp_utils::Result<std::string>{"hello"}
  );
  REQUIRE(r.has_value());
  auto [i, d, s] = *r;
  REQUIRE(i == 1);
  REQUIRE(d == 2.5);
  REQUIRE(s == "hello");
}

TEST_CASE("Result collect with error", "[expected_ext]") {
  auto r = cpp_utils::collect(
    cpp_utils::Result<int>{1},
    cpp_utils::Result<double>{std::unexpected(cpp_utils::Error{"fail"})}
  );
  REQUIRE_FALSE(r.has_value());
}

TEST_CASE("try_all returns first success", "[expected_ext]") {
  auto fallible = []() -> cpp_utils::Result<int> {
    return cpp_utils::try_all<int>(
      []() -> cpp_utils::Result<int> { return std::unexpected(cpp_utils::Error{"fail1"}); },
      []() -> cpp_utils::Result<int> { return 42; },
      []() -> cpp_utils::Result<int> { return 99; }
    );
  };
  auto r = fallible();
  REQUIRE(r.has_value());
  REQUIRE(*r == 42);
}

// ──────────────────────────────────────────────
// overloaded tests
// ──────────────────────────────────────────────

TEST_CASE("overloaded visits variant correctly", "[overloaded]") {
  std::variant<int, double, std::string> v;

  v = 42;
  std::visit(cpp_utils::overloaded{
    [](int i)    { REQUIRE(i == 42); },
    [](double)   { FAIL("should not be called"); },
    [](const std::string&) { FAIL("should not be called"); },
  }, v);

  v = 3.14;
  std::visit(cpp_utils::overloaded{
    [](int)      { FAIL("should not be called"); },
    [](double d) { REQUIRE(d == 3.14); },
    [](const std::string&) { FAIL("should not be called"); },
  }, v);

  v = std::string{"hello"};
  std::visit(cpp_utils::overloaded{
    [](int)      { FAIL("should not be called"); },
    [](double)   { FAIL("should not be called"); },
    [](const std::string& s) { REQUIRE(s == "hello"); },
  }, v);
}

// ──────────────────────────────────────────────
// traits tests
// ──────────────────────────────────────────────

TEST_CASE("Numeric concept", "[traits]") {
  STATIC_REQUIRE(cpp_utils::Numeric<int>);
  STATIC_REQUIRE(cpp_utils::Numeric<double>);
  STATIC_REQUIRE_FALSE(cpp_utils::Numeric<bool>);
  STATIC_REQUIRE_FALSE(cpp_utils::Numeric<std::string>);
}

TEST_CASE("StringLike concept", "[traits]") {
  STATIC_REQUIRE(cpp_utils::StringLike<std::string>);
  STATIC_REQUIRE(cpp_utils::StringLike<std::string_view>);
  STATIC_REQUIRE(cpp_utils::StringLike<const char*>);
}

TEST_CASE("is_specialization_v", "[traits]") {
  STATIC_REQUIRE(cpp_utils::is_specialization_v<std::vector<int>, std::vector>);
  STATIC_REQUIRE_FALSE(cpp_utils::is_specialization_v<std::vector<int>, std::optional>);
  STATIC_REQUIRE(cpp_utils::is_specialization_v<std::optional<int>, std::optional>);
}

TEST_CASE("all_same_v and any_same_v", "[traits]") {
  STATIC_REQUIRE(cpp_utils::all_same_v<int, int, int>);
  STATIC_REQUIRE_FALSE(cpp_utils::all_same_v<int, double, int>);
  STATIC_REQUIRE(cpp_utils::any_same_v<int, double, int, float>);
  STATIC_REQUIRE_FALSE(cpp_utils::any_same_v<int, double, float>);
}

TEST_CASE("first_type_t and last_type_t", "[traits]") {
  STATIC_REQUIRE(std::is_same_v<cpp_utils::first_type_t<int, double, char>, int>);
  STATIC_REQUIRE(std::is_same_v<cpp_utils::last_type_t<int, double, char>, char>);
}

TEST_CASE("is_detected_v", "[traits]") {
  // std::is_void<void> is a valid type → detected = true
  STATIC_REQUIRE(cpp_utils::is_detected_v<std::is_void, void>);
  // std::is_void<int> is also a valid type (it's std::false_type)
  STATIC_REQUIRE(cpp_utils::is_detected_v<std::is_void, int>);
}

TEST_CASE("HasValueType concept", "[traits]") {
  STATIC_REQUIRE(cpp_utils::HasValueType<std::vector<int>>);
  STATIC_REQUIRE(cpp_utils::HasValueType<std::optional<int>>);
  STATIC_REQUIRE_FALSE(cpp_utils::HasValueType<int>);
}

// ──────────────────────────────────────────────
// Benchmarks
// ──────────────────────────────────────────────

TEST_CASE("str::split benchmark", "[benchmark]") {
  namespace s = cpp_utils::str;
  constexpr std::string_view input = "a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p";
  BENCHMARK("split 16 parts") {
    return s::split(input, ',');
  };
}

TEST_CASE("str::to_int benchmark", "[benchmark]") {
  namespace s = cpp_utils::str;
  BENCHMARK("parse int from string") {
    return s::to_int<int>("1234567890");
  };
}

TEST_CASE("enum_flags ops benchmark", "[benchmark]") {
  cpp_utils::enum_flags<Color> flags = Color::Red | Color::Green;
  BENCHMARK("bitwise ops") {
    flags |= Color::Blue;
    flags ^= Color::Red;
    return flags.raw();
  };
}
