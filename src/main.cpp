#include <cpp_utils.hpp>

#include <iostream>
#include <vector>
#include <variant>
#include <cassert>

// Bring enum_flags operators into scope
using cpp_utils::operator|;

// ── Demo of all cpp_utils components ──

// Demo 1: scope_guard
void demo_scope_guard() {
  std::cout << "\n=== scope_guard ===\n";
  int counter = 0;

  // make_scope_guard — always runs on exit
  {
    auto guard = cpp_utils::make_scope_guard([&] { ++counter; });
    assert(counter == 0);
  }
  assert(counter == 1);
  std::cout << "  make_scope_guard: counter = " << counter << " (PASS)\n";

  // make_scope_fail — doesn't run on normal exit
  {
    auto guard = cpp_utils::make_scope_fail([&] { counter = -1; });
  }
  assert(counter == 1);
  std::cout << "  make_scope_fail: did not run on normal exit (PASS)\n";

  // make_scope_success — runs on normal exit
  {
    auto guard = cpp_utils::make_scope_success([&] { ++counter; });
  }
  assert(counter == 2);
  std::cout << "  make_scope_success: counter = " << counter << " (PASS)\n";

  // Dismiss
  {
    auto guard = cpp_utils::make_scope_guard([&] { counter = -99; });
    guard.dismiss();
  }
  assert(counter == 2);
  std::cout << "  dismiss: guard did not fire (PASS)\n";
}

// Demo 2: enum_flags
enum class Permission : uint8_t {
  Read    = 1 << 0,
  Write   = 1 << 1,
  Execute = 1 << 2,
  All     = 0x07
};

void demo_enum_flags() {
  std::cout << "\n=== enum_flags ===\n";

  cpp_utils::enum_flags<Permission> perms{Permission::Read | Permission::Write};
  assert(perms & Permission::Read);
  assert(perms & Permission::Write);
  assert(!(perms & Permission::Execute));
  std::cout << "  Permissions: Read=" << ((perms & Permission::Read) ? 1 : 0)
            << " Write=" << ((perms & Permission::Write) ? 1 : 0)
            << " Execute=" << ((perms & Permission::Execute) ? 1 : 0) << " (PASS)\n";

  perms |= Permission::Execute;
  assert(perms.all(Permission::All));
  std::cout << "  All permissions set (PASS)\n";

  perms.clear(Permission::Write);
  assert(!perms.all(Permission::All));
  std::cout << "  Write cleared (PASS)\n";

  // Bitwise NOT
  auto inverted = ~perms;
  assert(!(inverted & Permission::Read));   // Read was set, now clear
  std::cout << "  Bitwise NOT works (PASS)\n";

  // Toggle
  perms.toggle(Permission::Read);
  assert(!(perms & Permission::Read));
  perms.toggle(Permission::Read);
  assert(perms & Permission::Read);
  std::cout << "  Toggle works (PASS)\n";
}

// Demo 3: str utilities
void demo_str() {
  std::cout << "\n=== str utilities ===\n";
  namespace s = cpp_utils::str;

  // Split
  auto parts = s::split("hello,world,test", ',');
  assert(parts.size() == 3);
  assert(parts[0] == "hello");
  assert(parts[1] == "world");
  assert(parts[2] == "test");
  std::cout << "  Split: " << parts[0] << ", " << parts[1] << ", " << parts[2] << " (PASS)\n";

  // Trim
  assert(s::trim("  hello  ") == "hello");
  assert(s::trim("\t\n  spaced  \n") == "spaced");
  std::cout << "  Trim: PASS\n";

  // Case
  assert(s::to_lower("Hello World") == "hello world");
  assert(s::to_upper("Hello World") == "HELLO WORLD");
  assert(s::iequals("Hello", "hELLO"));
  std::cout << "  Case conversion + iequals: PASS\n";

  // Replace
  auto replaced = s::replace_all("one two one three", "one", "X");
  assert(replaced == "X two X three");
  std::cout << "  Replace: " << replaced << " (PASS)\n";

  // Join
  std::vector<std::string> words{"a", "b", "c"};
  auto joined = s::join(words, ", ");
  assert(joined == "a, b, c");
  std::cout << "  Join: " << joined << " (PASS)\n";

  // Safe conversion
  auto val = s::to_int<int>("42");
  assert(val && *val == 42);

  auto bad = s::to_int<int>("not_a_number");
  assert(!bad);
  std::cout << "  to_int: valid=42, invalid=error (PASS)\n";
}

// Demo 4: expected_ext
void demo_expected_ext() {
  std::cout << "\n=== expected_ext ===\n";

  // Result type alias
  auto good = cpp_utils::Result<int>{42};
  assert(good && *good == 42);

  auto bad = cpp_utils::Result<int>{std::unexpected(cpp_utils::Error{"something went wrong"})};
  assert(!bad);
  std::cout << "  Result<int>: value=42, error=\"" << bad.error().message << "\" (PASS)\n";

  // and_then chain
  auto chained = cpp_utils::Result<int>{10}
    .and_then([](int v) -> cpp_utils::Result<int> { return v * 2; })
    .and_then([](int v) -> cpp_utils::Result<int> { return v + 5; });
  assert(chained && *chained == 25);
  std::cout << "  Chained .and_then(): 10*2+5 = " << *chained << " (PASS)\n";

  // wrap_except
  auto safe_div = [](int a, int b) -> cpp_utils::Result<int> {
    return cpp_utils::wrap_except([&] {
      if (b == 0) throw std::runtime_error("division by zero");
      return a / b;
    });
  };
  auto ok = safe_div(10, 2);
  assert(ok && *ok == 5);
  auto fail = safe_div(10, 0);
  assert(!fail);
  std::cout << "  wrap_except: 10/2=" << (ok ? std::to_string(*ok) : "error")
            << ", 10/0=\"" << fail.error().message << "\" (PASS)\n";
}

// Demo 5: overloaded
void demo_overloaded() {
  std::cout << "\n=== overloaded ===\n";
  std::variant<int, double, std::string> v;

  v = 42;
  std::visit(cpp_utils::overloaded{
    [](int i)    { std::cout << "  int: " << i << " (PASS)\n"; },
    [](double d) { std::cout << "  double: " << d << "\n"; },
    [](const std::string& s) { std::cout << "  string: " << s << "\n"; },
  }, v);

  v = 3.14;
  std::visit(cpp_utils::overloaded{
    [](int i)    { std::cout << "  int: " << i << "\n"; },
    [](double d) { std::cout << "  double: " << d << " (PASS)\n"; },
    [](const std::string& s) { std::cout << "  string: " << s << "\n"; },
  }, v);

  v = std::string{"hello"};
  std::visit(cpp_utils::overloaded{
    [](int i)    { std::cout << "  int: " << i << "\n"; },
    [](double d) { std::cout << "  double: " << d << "\n"; },
    [](const std::string& s) { std::cout << "  string: \"" << s << "\" (PASS)\n"; },
  }, v);
}

// Demo 6: traits concepts
void demo_traits() {
  std::cout << "\n=== traits ===\n";

  static_assert(cpp_utils::Numeric<int>);
  static_assert(cpp_utils::Numeric<double>);
  static_assert(!cpp_utils::Numeric<bool>);
  static_assert(!cpp_utils::Numeric<std::string>);
  std::cout << "  Numeric concept: PASS\n";

  static_assert(cpp_utils::StringLike<std::string>);
  static_assert(cpp_utils::StringLike<std::string_view>);
  static_assert(cpp_utils::StringLike<const char*>);
  std::cout << "  StringLike concept: PASS\n";

  static_assert(cpp_utils::is_specialization_v<std::vector<int>, std::vector>);
  static_assert(!cpp_utils::is_specialization_v<std::vector<int>, std::optional>);
  std::cout << "  is_specialization_v: PASS\n";

  static_assert(cpp_utils::all_same_v<int, int, int>);
  static_assert(!cpp_utils::all_same_v<int, double, int>);
  std::cout << "  all_same_v: PASS\n";
}

int main() {
  std::cout << "╔══════════════════════════════════════╗\n";
  std::cout << "║   cpp_utils v1.0.0 — Demo Suite     ║\n";
  std::cout << "╚══════════════════════════════════════╝\n";

  demo_scope_guard();
  demo_enum_flags();
  demo_str();
  demo_expected_ext();
  demo_overloaded();
  demo_traits();

  std::cout << "\n✅ All demos passed!\n";
  return 0;
}
