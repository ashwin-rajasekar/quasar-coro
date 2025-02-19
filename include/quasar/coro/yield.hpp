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

#include "coroutine.hpp"
#include "promise.hpp"

#include <iterator>

namespace quasar::coro {
	template<class T> struct yield_iterator {
		yield_iterator(yield_iterator const&)  = delete;
		yield_iterator(yield_iterator&&)       = delete;
		void operator =(yield_iterator const&) = delete;
		void operator =(yield_iterator&&)      = delete;

		yield_iterator()  noexcept = default;
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

		yield_iterator& operator ++(){
			if(*this != std::default_sentinel){ m_task.resume(); }
			m_rethrow(m_task);
			return *this;
		}

		template<class DelegaterPromise, class Delegatee>
		auto get_awaiter(DelegaterPromise& caller, Delegatee&& task) noexcept
			requires requires { await::delegate<Delegatee>{std::move(task)}; }
		{
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

		template<class Promise> void preempt(std::coroutine_handle<Promise> task){
			task.promise().set_continuation(
				[](auto& self, auto preemptor, auto preempted, auto getter, auto rethrow) -> procedure {
					// this procedure manually handles cleanup since the preemptor is not bound to the lifetime of an object
					preemptor.destroy();
					// need to restore & resume the preempted since control has fallen off then end of the preemptor
					(self.m_task = preempted).resume();
					// restore the remainder of the yield_iterator to its original state
					self.m_getter = getter;
					self.m_rethrow = rethrow;

					co_return;
				}(*this, task, m_task, m_getter, m_rethrow).release()
			);
			bind(task.promise());
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

			using getter_func = T(std::coroutine_handle<void>) noexcept;
			using rethrow_func = void(std::coroutine_handle<void>);

			std::coroutine_handle<void> m_task{};
			getter_func* m_getter = nullptr;
			rethrow_func* m_rethrow = nullptr;
	};

	template<class Generator> struct yield_range {
		Generator task;

		yield_iterator<decltype(task.promise().get_value())> begin() const noexcept { return task; }
		std::default_sentinel_t end() const noexcept { return {}; }
	};
}
