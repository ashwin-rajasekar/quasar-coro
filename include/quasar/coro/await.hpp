#pragma once

#include <coroutine>
#include <optional>
#include <tuple>
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

	template<class... Ts> requires ((!std::is_void_v<Ts>) && ...) struct callback {
		constexpr callback(auto&& func){
			func([this](Ts... args){
				m_results.emplace(std::move(args)...);
				if(m_task){ m_task.resume(); }
			});
		}

		constexpr bool await_ready() const noexcept { return !!m_results; }

		constexpr void await_suspend(auto coro) noexcept { m_task = coro; }

		constexpr auto await_resume() noexcept {
			if constexpr(sizeof...(Ts) == 1){ return std::move(std::get<0>(*m_results)); }
			else if constexpr(sizeof...(Ts) > 1){ return std::move(*m_results); }
		}

		private:
			std::coroutine_handle<void> m_task = nullptr;
			std::optional<std::tuple<Ts...>> m_results = std::nullopt;
	};

	template<class T> struct fetch {
		T value;

		constexpr bool await_ready() const noexcept { return true; }

		constexpr void await_suspend(auto) const noexcept {}

		constexpr T await_resume() noexcept { return std::forward<T>(value); }
	};
}
