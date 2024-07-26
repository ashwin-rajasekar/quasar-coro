#pragma once

#include "await.hpp"
#include "coroutine.hpp"

#include <optional>
#include <stdexcept>
#include <type_traits>

namespace quasar::coro::promise {
	struct base {
		template<class Self> coroutine<Self> get_return_object(this Self& self){ return {self}; }

		std::suspend_always initial_suspend(){ return {}; }
	};







	/** Exception Support **/
	struct nothrow {
		void unhandled_exception() const noexcept { std::terminate(); }
	};

	struct unwind_on_exception {
		void unhandled_exception() noexcept { m_except = std::current_exception(); }

		void rethrow(){ if(m_except){ std::rethrow_exception(std::exchange(m_except, nullptr)); } }

		protected:
			std::exception_ptr m_except = nullptr;
	};





	/** Continuation Support **/
	struct pause_on_finish {
		std::suspend_always final_suspend() noexcept { return {}; }
	};

	template<bool pause_at_finish> struct delegatable {
		static constexpr std::coroutine_handle<void> default_continuation() noexcept {
			if constexpr(pause_at_finish){ return std::noop_coroutine(); }
			else { return {}; }
		}

		void set_continuation(std::coroutine_handle<void> continuation) noexcept { m_continuation = continuation; }

		await::handoff intermediate_suspend() noexcept {
			if(pause_at_finish || m_continuation){ return {.task = std::exchange(m_continuation, default_continuation())}; }
			else { return {.task = std::noop_coroutine()}; }
		}

		await::handoff final_suspend() noexcept { return {.task = m_continuation}; }

		protected:
			std::coroutine_handle<void> m_continuation = default_continuation();
	};





	/** Result Support **/
	template<class T> struct result {
		void return_value(auto&& arg){ m_result.emplace(std::move(arg)); }

		T get_result() noexcept { return std::move(*m_result); }

		protected:
			std::optional<T> m_result;
	};

	template<class T> requires (std::is_trivially_default_constructible_v<T>) struct result<T> {
		void return_value(auto&& arg){ m_result = std::move(arg); }

		T get_result() noexcept { return std::move(m_result); }

		protected:
			T m_result;
	};

	template<class T> requires (std::is_reference_v<T>) struct result<T> {
		void return_value(T arg){ m_result = &arg; }

		T get_result() noexcept { return static_cast<T>(*m_result); }

		protected:
			std::remove_reference_t<T>* m_result;
	};

	template<> struct result<void> { void return_void(){} };





	/** Yield Support **/
	template<typename T> struct yield_base {
		T get_value() noexcept { return static_cast<T>(*m_yield); }

		protected:
			void capture_value(T&& value) noexcept { m_yield = std::addressof(value); }

			std::remove_reference_t<T>* m_yield;
	};

	template<typename T> requires ((sizeof(T) <= 2 * sizeof(void*)) && std::is_trivially_move_constructible_v<T>)
	struct yield_base<T> {
		T get_value() noexcept { return std::move(m_yield); }

		protected:
			void capture_value(auto&& value) noexcept { std::construct_at(&m_yield, std::move(value)); }

			union {
				std::byte dummy{};
				T m_yield;
			};
	};

	template<class T, bool async = false> struct yield : yield_base<T> {
		auto yield_value(this auto& self, T&& value) noexcept {
			self.capture_value(std::forward<T>(value));
			if constexpr(async){ return self.intermediate_suspend(); }
			else { return std::suspend_always{}; }
		}
	};

	template<class T, class Base = yield<T>, class YieldItr = yield_iterator<T>>
	struct delegating_yield : Base {
		using Base::Base;
		using Base::yield_value;

		template<class Coro> requires (std::derived_from<Coro, std::coroutine_handle<typename Coro::promise_type>>)
		auto yield_value(this auto& self, Coro&& task) noexcept {
			return self.m_iterator->get_awaiter(self, std::move(task));
		}

		void set_iterator(YieldItr& itr) noexcept { m_iterator = std::addressof(itr); }

		protected:
			YieldItr* m_iterator = nullptr;
	};
}

/** Common Promise Implementations **/
namespace quasar::coro {
	template<class Result> struct task_promise :
		promise::base,
		promise::unwind_on_exception,
		promise::delegatable<true>,
		promise::result<Result>{};

	template<class Yield, class Result> struct simple_generator_promise :
		task_promise<Result>,
		promise::yield<Yield>{};

	template<class Yield, class Result> struct generator_promise :
		task_promise<Result>,
		promise::delegating_yield<Yield>{};
}
