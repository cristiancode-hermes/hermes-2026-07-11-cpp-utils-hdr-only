#pragma once

#include <type_traits>
#include <exception>
#include <utility>

namespace cpp_utils {

// ── RAII scope guards (use factory functions instead of CTAD on aliases) ──
//
// auto g = scope_guard([&] { cleanup(); });   // runs on scope exit
// auto g = scope_fail([&] { rollback(); });    // runs only on exception
// auto g = scope_success([&] { commit(); });   // runs only on normal exit

enum class ScopeGuardStrategy { Always, Fail, Success };

namespace detail {

template <ScopeGuardStrategy Strategy, typename Fn>
class ScopeGuardImpl {
  static_assert(std::is_invocable_v<Fn>);

public:
  explicit ScopeGuardImpl(Fn fn) noexcept(std::is_nothrow_move_constructible_v<Fn>)
    : fn_(std::move(fn)), active_(true) {}

  ScopeGuardImpl(ScopeGuardImpl&& other) noexcept(std::is_nothrow_move_constructible_v<Fn>)
    : fn_(std::move(other.fn_)), active_(std::exchange(other.active_, false)) {}

  ScopeGuardImpl& operator=(ScopeGuardImpl&&) = delete;
  ScopeGuardImpl(const ScopeGuardImpl&) = delete;
  ScopeGuardImpl& operator=(const ScopeGuardImpl&) = delete;

  ~ScopeGuardImpl() noexcept(noexcept(std::declval<Fn&>())) {
    if (!active_) return;
    if constexpr (Strategy == ScopeGuardStrategy::Always) {
      fn_();
    } else if constexpr (Strategy == ScopeGuardStrategy::Fail) {
      if (std::uncaught_exceptions() > 0) fn_();
    } else if constexpr (Strategy == ScopeGuardStrategy::Success) {
      if (std::uncaught_exceptions() == 0) fn_();
    }
  }

  void dismiss() noexcept { active_ = false; }
  void reactivate() noexcept { active_ = true; }

private:
  Fn fn_;
  bool active_;
};

} // namespace detail

// ── Factory functions (for CTAD without alias template issues) ──
template <typename Fn>
auto make_scope_guard(Fn&& fn)
  -> detail::ScopeGuardImpl<ScopeGuardStrategy::Always, std::decay_t<Fn>>
{
  return detail::ScopeGuardImpl<ScopeGuardStrategy::Always, std::decay_t<Fn>>(std::forward<Fn>(fn));
}

template <typename Fn>
auto make_scope_fail(Fn&& fn)
  -> detail::ScopeGuardImpl<ScopeGuardStrategy::Fail, std::decay_t<Fn>>
{
  return detail::ScopeGuardImpl<ScopeGuardStrategy::Fail, std::decay_t<Fn>>(std::forward<Fn>(fn));
}

template <typename Fn>
auto make_scope_success(Fn&& fn)
  -> detail::ScopeGuardImpl<ScopeGuardStrategy::Success, std::decay_t<Fn>>
{
  return detail::ScopeGuardImpl<ScopeGuardStrategy::Success, std::decay_t<Fn>>(std::forward<Fn>(fn));
}

// ── Convenience type aliases (for member variables) ──
template <typename Fn>
using scope_guard_t = detail::ScopeGuardImpl<ScopeGuardStrategy::Always, std::decay_t<Fn>>;

template <typename Fn>
using scope_fail_t = detail::ScopeGuardImpl<ScopeGuardStrategy::Fail, std::decay_t<Fn>>;

template <typename Fn>
using scope_success_t = detail::ScopeGuardImpl<ScopeGuardStrategy::Success, std::decay_t<Fn>>;

} // namespace cpp_utils
