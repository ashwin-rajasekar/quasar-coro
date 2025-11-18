/**
 *  Copyright (C) 2025 Ashwin Rajasekar
 *
 *  This file is a part of quasar-coro.
 *
 *  quasar-coro is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU Lesser Public License version 3 as published by the
 *  Free Software Foundation.
 *
 *  quasar-coro is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License & the GNU
 *  Lesser Public License along with this software; see the files COPYING and
 *  COPYING.LESSER respectively.  If not, see <http://www.gnu.org/licenses/>.
 **/

#pragma once

#include "await.hpp"
#include "fwd.hpp"

#ifndef QUASAR_CORO_MODULES
	#include <exception>
	#include <optional>
	#include <type_traits>
#endif

/** Some functions are either static or explicit-object depending on if the feature is available
 *    if explicit-object functions are not available, the promise class must define the necessary member fuction for the
 *    coroutine machinery, using the static base-class function as a default implementation */
#ifdef QUASAR_CORO_NO_EXPLICIT_OBJECT
#define eo_static static
#define eo_this

#else
#define eo_static
#define eo_this this

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
		template<class Self>
		eo_static auto get_return_object(eo_this Self& self){ return std::coroutine_handle<Self>::from_promise(self); }
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
		std::suspend_always final_suspend() const noexcept { return {}; }
	};

	struct destroy_on_finish {
		std::suspend_never final_suspend() const noexcept { return {}; }
	};

	template<bool pause_at_finish> struct delegatable {
		static constexpr std::coroutine_handle<void> default_continuation() noexcept {
			if constexpr(pause_at_finish){ return std::noop_coroutine(); }
			else { return {}; }
		}

		void set_continuation(std::coroutine_handle<void> continuation) noexcept { m_continuation = continuation; }

		await::handoff<false> intermediate_suspend() noexcept {
			if(pause_at_finish || m_continuation){ return {.task = std::exchange(m_continuation, default_continuation())}; }
			else { return {.task = std::noop_coroutine()}; }
		}

		await::handoff<false> final_suspend() const noexcept { return {.task = m_continuation}; }

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

	template<class T> requires (std::is_void_v<T>) struct result<T> { void return_void(){} };





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
			!std::convertible_to<std::remove_cvref_t<Coro>, std::remove_cvref_t<Yield>>&&
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
		#ifdef QUASAR_CORO_NO_EXPLICIT_OBJECT
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
		#ifdef QUASAR_CORO_NO_EXPLICIT_OBJECT
		auto get_return_object(){ return promise::base::get_return_object(*this); }
		#endif
	};

	template<class Yield, class Result> struct simple_generator_promise :
		task_promise<Result>,
		promise::yield<Yield>
	{
		#ifdef QUASAR_CORO_NO_EXPLICIT_OBJECT
		auto get_return_object(){ return promise::base::get_return_object(*this); }

		template<class T = Yield>
		auto yield_value(T&& yield){ return promise::yield<Yield>::yield_value(*this, std::forward<T>(yield)); }
		#endif
	};

	template<class Yield, class Result> struct generator_promise :
		task_promise<Result>,
		promise::delegating_yield<Yield>
	{
		#ifdef QUASAR_CORO_NO_EXPLICIT_OBJECT
		auto get_return_object(){ return promise::base::get_return_object(*this); }

		template<class T = Yield>
		auto yield_value(T&& yield){ return promise::delegating_yield<Yield>::yield_value(*this, std::forward<T>(yield)); }
		#endif
	};
}

#undef eo_static
#undef eo_this
