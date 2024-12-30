#pragma once

#include "await.hpp"
#include "coroutine.hpp"

#include <optional>
#include <stdexcept>
#include <type_traits>

/** Some functions are either static or explicit-object depending on if the feature is available
 *    if explicit-object functions are not available, the promise class must define the necessary member fuction for the
 *    coroutine machinery, using the static base-class function as a default implementation */
#if defined(__cpp_explicit_this_parameter) && __cpp_explicit_this_parameter >= 202110L
#define eo_static
#define eo_this this

#else
#define eo_unavailable
#define eo_static static
#define eo_this

#endif

namespace quasar::coro::promise {
	namespace detail {
		template<class T> struct capture {
			static constexpr bool ref_type = std::is_reference_v<T>;

			template<class U> void capture_value(U&& arg) requires (!ref_type){ m_value.emplace(std::forward<U>(arg)); }

			T release_value() noexcept requires (!ref_type){ return std::move(*m_value); }

			void capture_value(T arg) requires (ref_type){ m_value = &arg; }

			T release_value() noexcept requires (ref_type){ return static_cast<T>(*m_value); }

			T& get() noexcept { return *m_value; }

			private:
				std::conditional_t<
					ref_type,
					std::remove_reference_t<T>*,
					std::optional<T>
				> m_value = {};
		};
	}

	struct base {
		template<class Self> eo_static coroutine<Self> get_return_object(eo_this Self& self){ return {self}; }
	};




	/** Initialization Support **/
	struct eager {
		std::suspend_never initial_suspend(){ return {}; }
	};

	struct lazy {
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

	struct destroy_on_finish {
		std::suspend_never final_suspend() noexcept { return {}; }
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
	template<class Result> struct result {
		template<class T> void return_value(T&& arg){ m_result.capture_value(std::forward<T>(arg)); }

		Result get_result() noexcept { return m_result.release_value(); }

		protected:
			detail::capture<Result> m_result = {};
	};

	template<> struct result<void> { void return_void(){} };





	/** Yield Support **/
	template<class Yield, bool async = false> struct yield {
		template<class T = Yield> eo_static auto yield_value(eo_this auto& self, T&& value) noexcept {
			self.m_yield.capture_value(std::forward<T>(value));
			if constexpr(async){ return self.intermediate_suspend(); }
			else { return std::suspend_always{}; }
		}

		Yield get_value() noexcept { return m_yield.release_value(); }

		protected:
			detail::capture<Yield> m_yield = {};
	};

	template<class Yield, class Base = yield<Yield>, class YieldItr = yield_iterator<Yield>>
	struct delegating_yield : Base {
		using Base::Base;
		using Base::yield_value;

		template<class Coro> eo_static auto yield_value(eo_this auto& self, Coro&& task) noexcept requires (
			!std::convertible_to<std::remove_cvref_t<Coro>, std::remove_cvref_t<Yield>> &&
			requires { self.m_iterator->get_awaiter(self, std::move(task)); }
		){
			return self.m_iterator->get_awaiter(self, std::move(task));
		}

		void set_iterator(YieldItr& itr) noexcept { m_iterator = std::addressof(itr); }

		protected:
			YieldItr* m_iterator = nullptr;
	};
}

/** Common Promise Implementations **/
namespace quasar::coro {
	struct procedure_promise :
		promise::base,
		promise::eager,
		promise::nothrow,
		promise::destroy_on_finish,
		promise::result<void>
	{
		#ifdef eo_unavailable
		auto get_return_object(){ return promise::base::get_return_object(*this); }
		#endif
	};

	template<class Result> struct task_promise :
		promise::base,
		promise::lazy,
		promise::unwind_on_exception,
		promise::delegatable<true>,
		promise::result<Result>
	{
		#ifdef eo_unavailable
		auto get_return_object(){ return promise::base::get_return_object(*this); }
		#endif
	};

	template<class Yield, class Result> struct simple_generator_promise :
		task_promise<Result>,
		promise::yield<Yield>
	{
		#ifdef eo_unavailable
		auto get_return_object(){ return promise::base::get_return_object(*this); }

		template<class T = Yield>
		auto yield_value(T&& yield){ return promise::yield<Yield>::yield_value(*this, std::forward<T>(yield)); }
		#endif
	};

	template<class Yield, class Result> struct generator_promise :
		task_promise<Result>,
		promise::delegating_yield<Yield>
	{
		#ifdef eo_unavailable
		auto get_return_object(){ return promise::base::get_return_object(*this); }

		template<class T = Yield>
		auto yield_value(T&& yield){ return promise::delegating_yield<Yield>::yield_value(*this, std::forward<T>(yield)); }
		#endif
	};
}

#undef eo_unavailable
#undef eo_static
#undef eo_this
