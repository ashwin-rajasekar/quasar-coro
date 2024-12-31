#pragma once

#include <coroutine>
#include <utility>

namespace quasar::coro::await {
	template<class Coro> struct delegate {
		Coro task;

		bool await_ready() const noexcept { return task.done(); }

		std::coroutine_handle<void> await_suspend(auto caller) const noexcept {
			task.promise().set_continuation(caller);
			return static_cast<std::coroutine_handle<void>>(task);
		}

		decltype(auto) await_resume() const noexcept(!requires{ task.promise().rethrow(); }) {
			if constexpr(requires{ task.promise().rethrow(); }){ task.promise().rethrow(); }
			if constexpr(requires{ task.promise().get_result(); }){ return task.promise().get_result(); }
		}
	};

	struct handoff : std::suspend_always {
		std::coroutine_handle<void> task;

		bool await_ready() const noexcept { return !task; }

		std::coroutine_handle<void> await_suspend(auto) const noexcept { return task; }
	};

	template<class Func> struct callback : std::suspend_always {
		Func func;

		auto await_suspend(auto coro) const noexcept { return func(coro); }
	};

	template<class T> struct fetch : std::suspend_never {
		T value;

		T await_resume() noexcept { return std::forward<T>(value); }
	};
}
