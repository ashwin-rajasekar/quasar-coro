# Quasar C++ Coroutine Support Library

This is a library for supporting C++20 coroutines by making common coroutine handle types and providing base-classes as building blocks for creating custom promise types.

## Awaitables
The awaitable types provide the hook into the compiler coroutine machinery to allow control transfer.

```c++
namespace quasar::coro::await {
	template<class Coro> struct delegate;
	struct handoff;
}
```

### `await::delegate<Coro>`
This awaitable is constructed with any type `Coro` representing a valid coroutine. Control is transferred to this delgeted coroutine, which determines how control is returned to the awaiting coroutine. The `Coro` type must be compatible with the following interface, where `CallerPromise` is the promise type of the awaiting coroutine:
```c++
struct Promise {
	void set_continuation(std::coroutine_handle<CallerPromise>);
	void rethrow() const;       // optional
	AnyType get_result() const; // optional
};

struct Coro {
	bool done() const;
	Promise& promise() const;
	explicit operator std::coroutine_handle<void>() const;
};
```

### `await::handoff`
This awaitable is constructed with a `std::coroutine_handle<void>` to which control is transferred. If the handle is `nullptr`, control remains in the awaiting coroutine without suspending. If the handle is `std::noop_coroutine()`, control is transferred back to the resumer.

## Coroutine Handle Types
The 2 main coroutine handle types provided are `coroutine` and `unique_coroutine`. They represent non-owning and owning handles to coroutine frames respectively.

```c++
namespace quasar::coro {
	template<class Promise> struct coroutine;
	template<class Promise> struct unique_coroutine;
}
```
### `coroutine`
The if the promise type provides a `void rethrow()` function it will be invoked after every resumption of the coroutine. This is to allow exception-handling promise types to re-raise exceptions to the calling function.

### `unique_coroutine`
This handle type represents exclusive ownership of the coroutine frame. The frame is automatically cleaned up if the `unique_coroutine` onbject representing it is destroyed (e.g. by going out of scope). By `co_await`ing an r-value of this type, you can delegate to the represented coroutine. Upon completion of the delgeted coroutine, control will return to you

## Promise Base Types
The promise base types allow users to quickly & easily create custom promise types by simply inheriting the from 

```c++
namespace quasar::coro::promise {
	struct base;

	struct nothrow;
	struct unwind_on_exception;

	struct pause_at_finish;
	template<bool pause> stuuct delegatable;

	template<class T> struct result;

	template<class T> struct yield;
	template<class T, class Base, class Itr> struct delegating_yield;
}
```

## Common Coroutine Types
Some common use-cases have generic promise types already available
```c++
namespace quasar::coro {
	template<class Result> struct task;
	template<class Yield, class Result> struct simple_generator;
	template<class Yield, class Result> struct generator;
}
```

### `task<Result>`
A task produces a single value of type `Result` asynchronously, with lazy initialization. 
