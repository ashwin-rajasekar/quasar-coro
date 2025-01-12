#pragma once

#include <coroutine>
#include <utility>

namespace quasar::coro::await {
	template<class Coro> struct delegate {
		Coro task;

		constexpr bool await_ready() const noexcept { return task.done(); }

		constexpr std::coroutine_handle<void> await_suspend(auto caller) const noexcept {
			task.promise().set_continuation(caller);
			return static_cast<std::coroutine_handle<void>>(task);
		}

		constexpr decltype(auto) await_resume() const noexcept(!requires{ task.promise().rethrow(); }) {
			if constexpr(requires{ task.promise().rethrow(); }){ task.promise().rethrow(); }
			if constexpr(requires{ task.promise().get_result(); }){ return task.promise().get_result(); }
		}
	};

	struct handoff {
		std::coroutine_handle<void> task;

		constexpr bool await_ready() const noexcept { return !task; }

		constexpr std::coroutine_handle<void> await_suspend(auto) const noexcept { return task; }

		constexpr void await_resume() const noexcept {}
	};

	template<class Func> struct callback {
		Func func;

		constexpr bool await_ready() const noexcept { return false; }

		constexpr auto await_suspend(auto coro) const noexcept { return func(coro); }

		constexpr void await_resume() const noexcept {}
	};

	template<class T> struct fetch {
		T value;

		constexpr bool await_ready() const noexcept { return true; }

		constexpr void await_suspend() const noexcept {}

		constexpr T await_resume() noexcept { return std::forward<T>(value); }
	};
}
