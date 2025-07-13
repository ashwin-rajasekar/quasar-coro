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

namespace quasar::coro::await {
	struct barrier {
		constexpr bool await_ready() const noexcept { return !m_count; }

		constexpr void await_suspend(std::coroutine_handle<void> caller) noexcept { m_continuation = caller; }

		constexpr void await_resume() const noexcept {}

		template<class Coro> void wait(Coro&& coro) noexcept {
			if(!coro || coro.done()){ return; }

			++m_count;
			handler(std::forward<Coro>(coro));
		}

		private:
			procedure handler(auto task) {
				co_await std::move(task);
				co_await await::handoff<true>{.task = --m_count? nullptr : m_continuation};
			}

			std::size_t m_count = 0;
			std::coroutine_handle<void> m_continuation;
	};
}
