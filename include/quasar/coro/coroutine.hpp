#pragma once

#include "await.hpp"
#include "fwd.hpp"

#include <utility>

namespace quasar::coro {
	template<class Promise> struct [[nodiscard]] coroutine : std::coroutine_handle<Promise> {
		using promise_type = Promise;
		using handle = std::coroutine_handle<Promise>;

		using handle::handle;
		coroutine(handle coro) noexcept : handle{coro}{} // need to explicitly enable construction from base type
		coroutine(Promise& prom) noexcept : handle{handle::from_promise(prom)}{}
		coroutine(unique_coroutine<Promise>) = delete;

		void resume() const noexcept(!requires{ this->promise().rethrow(); }){
			handle::resume();
			if constexpr(requires{ this->promise().rethrow(); }){ this->promise().rethrow(); }
		}

		void operator()() const noexcept(noexcept(resume())) { resume(); }
	};

	template<class Promise> struct unique_coroutine : coroutine<Promise> {
		using coroutine<Promise>::coroutine;

		unique_coroutine(coroutine<Promise> coro) noexcept : coroutine<Promise>{coro}{}

		unique_coroutine(unique_coroutine const&)            = delete;
		unique_coroutine& operator=(unique_coroutine const&) = delete;

		constexpr unique_coroutine(unique_coroutine&& other) noexcept :
			coroutine<Promise>{std::exchange<coroutine<Promise>>(other, nullptr)}{}

		constexpr unique_coroutine& operator=(unique_coroutine&& other) noexcept {
			std::swap<coroutine<Promise>>(*this, other);
			return *this;
		}

		constexpr ~unique_coroutine() noexcept {
			if(*this){ this->destroy(); }
		}

		await::delegate<unique_coroutine> operator co_await() && noexcept { return {std::move(*this)}; }

		[[nodiscard]] coroutine<Promise> release() noexcept { return std::exchange<coroutine<Promise>>(*this, nullptr); }

		coroutine<Promise> get() const noexcept { return *this; }
	};
}
