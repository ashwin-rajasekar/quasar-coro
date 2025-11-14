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
 
module;

#include <coroutine>
#include <exception>
#include <iterator>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

export module quasar.coro;

export namespace quasar::coro {
	namespace await {
		template<class> struct delegate;
		template<bool> struct handoff;
		template<class...> struct callback;
		template<class> struct fetch;
		struct barrier;
	}



	template<class> struct coroutine;
	template<class> struct unique_coroutine;

	template<class> struct yield_iterator;
	template<class> struct yield_range;



	struct procedure_promise;
	using procedure = coroutine<procedure_promise>;

	template<class> struct task_promise;
	template<class T> using task = unique_coroutine<task_promise<T>>;

	template<class, class> struct simple_generator_promise;
	template<class Y, class R = void> using simple_generator = unique_coroutine<simple_generator_promise<Y, R>>;

	template<class, class> struct generator_promise;
	template<class Y, class R = void> using generator = unique_coroutine<generator_promise<Y, R>>;
}

#include "quasar/coro/await.hpp"
#include "quasar/coro/barrier.hpp"
#include "quasar/coro/coroutine.hpp"
#include "quasar/coro/fwd.hpp"
#include "quasar/coro/promise.hpp"
#include "quasar/coro/yield.hpp"
