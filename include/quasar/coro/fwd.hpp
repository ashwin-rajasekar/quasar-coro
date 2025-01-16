#pragma once

namespace quasar::coro {
	namespace await {
		template<class> struct delegate;
		struct handoff;
		template<class...> struct callback;
		template<class> struct fetch;
	}



	template<class> struct coroutine;
	template<class> struct unique_coroutine;

	template<class> struct yield_iterator;



	struct procedure_promise;
	using procedure = coroutine<procedure_promise>;

	template<class> struct task_promise;
	template<class T> using task = unique_coroutine<task_promise<T>>;

	template<class, class> struct simple_generator_promise;
	template<class Y, class R = void> using simple_generator = unique_coroutine<simple_generator_promise<Y, R>>;

	template<class, class> struct generator_promise;
	template<class Y, class R = void> using generator = unique_coroutine<generator_promise<Y, R>>;
}
