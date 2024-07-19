#pragma once

#include "await.hpp"

#include <coroutine>
#include <iterator>

namespace quasar::coro {
	template<class T> struct yield_iterator {
		yield_iterator(yield_iterator const&)  = delete;
		yield_iterator(yield_iterator&&)       = delete;
		void operator =(yield_iterator const&) = delete;
		void operator =(yield_iterator&&)      = delete;
	
		yield_iterator() noexcept = default;
		~yield_iterator() noexcept = default;

		template<class P> yield_iterator(std::coroutine_handle<P> coro){
			bind(coro.promise());
			++*this;
		}
	
		bool operator ==(std::default_sentinel_t) const noexcept { return m_task.done(); }
	
		T operator *() const noexcept { return m_getter(m_task); }
		
		auto* operator ->() const noexcept requires std::is_lvalue_reference_v<T> { return std::addressof(**this); }
		
		auto const* operator ->() const noexcept requires std::is_rvalue_reference_v<T> {
			T tmp = **this;
			return std::addressof(tmp);
		}
	
		template<class Self> Self& operator ++(this Self& self){
			if(self != std::default_sentinel){ self.m_task.resume(); }
			self.m_rethrow(self.m_task);
			return self;
		}

		template<class DelegaterPromise, class Delegatee> auto get_awaiter(DelegaterPromise& caller, Delegatee&& task) noexcept {
			struct awaiter : await::delegate<Delegatee> {
				constexpr awaiter(yield_iterator& itr, DelegaterPromise& caller, Delegatee&& task) noexcept :
					await::delegate<Delegatee>{std::move(task)},
					m_iterator{itr},
					m_promise{caller}{}
				
				constexpr bool await_ready() const noexcept { return false; }

				std::coroutine_handle<void> await_suspend(std::coroutine_handle<DelegaterPromise> caller) noexcept {
					m_iterator.bind(this->task.promise());
					return await::delegate<Delegatee>::await_suspend(caller);
				}

				decltype(auto) await_resume() noexcept(noexcept(await::delegate<Delegatee>::await_resume())){
					m_iterator.bind(m_promise);
					return await::delegate<Delegatee>::await_resume();
				}

				private:
					yield_iterator& m_iterator;
					DelegaterPromise& m_promise;
			};

			return awaiter{*this, caller, std::move(task)};
		}

		private:
			template<class Promise> void bind(Promise& promise){
				static constexpr auto extract_promise = [](std::coroutine_handle<void> task) noexcept -> Promise& {
					return std::coroutine_handle<Promise>::from_address(task.address()).promise();
				};

				m_task = std::coroutine_handle<Promise>::from_promise(promise);
				m_getter = [](std::coroutine_handle<void> task) noexcept -> T { return extract_promise(task).get_value(); };
				m_rethrow = [](std::coroutine_handle<void> task){
					if constexpr(requires{ extract_promise(task).rethrow(); }){ return extract_promise(task).rethrow(); }
				};

				if constexpr(requires{ promise.set_iterator(*this); }){ promise.set_iterator(*this); }
			}

			std::coroutine_handle<void> m_task{};
			T (*m_getter)(std::coroutine_handle<void>) noexcept = nullptr;
			void (*m_rethrow)(std::coroutine_handle<void>) = nullptr;
	};

	template<class Generator> struct yield_range {
		Generator task;
	
		yield_iterator<decltype(task.promise().get_value())> begin() const noexcept { return task; }
		std::default_sentinel_t end() const noexcept { return {}; }
	};
}