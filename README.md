# Quasar C++ Coroutine Support Library

This is a library for supporting C++20 coroutines by making common coroutine handle types and providing base-classes as building blocks for creating custom promise types. All library-provided types are in the `quasar::coro` namespace.

## Using quasar-coro
Building quasar-coro requires a c++20 compliant compiler. There are no external library dependencies, and the library is currently header-only (though this may change in the future due to implementation needs). The library is built with CMake and produces a single target `quasar::coro` that consumers should link against. The library can be included in a project via:
- CMake's `FetchContent`
- git submodule reference & `add_subdirectory()`
- `find_package` after installing the targets on your system

```cmake
FetchContent_Declare(
	quasar-coro
	GIT_REPOSITORY https://github.com:/ashwin-rajasekar/quasar-coro
	GIT_TAG main
)

FetchContent_MakeAvailable(quasar-coro)

add_executable(some_exe some_source.cpp)
target_link_libraries(some_exe quasar::coro)
```
# Library Overview
- [Awaitables](#awaitables)
	- [`await::delegate<Coro>`](#awaitdelegatecoro)
	- [`await::handoff`](#awaithandoff)
	- [`await::callback<Func>`](#awaitcallbackfunc)
- [Coroutine Handle Types](#coroutine-handle-types)
	- [`coroutine`](#coroutine)
	- [`unique_coroutine`](#unique_coroutine)
- [Promise Base Types](#coroutine-handle-types)
	- [`base`](#promisebase)
	- [Initialization Support](#initialization-support)
	- [Exception Support](#exception-support)
	- [Continuation Support](#continuation-support)
	- [`promise::result<T>`](#promiseresultt)
	- [Yield Support](#yield-support)
- [Utilities](#utilities)
	- [`yield_iterator<T>`](#yield_iteratort)
	- [`yield_range<Coro>`](#yield_rangecoro)
- [Common Coroutine Types](#common-coroutine-types)
	- [`task<Result>`](#taskresult)
	- [`simple_generator<Yield, Result>` & `generator<Yield, Result>`](#simple_generatoryield-result--generatoryield-result)

## Awaitables
The awaitable types provide the hook into the compiler coroutine machinery to allow control transfer.

```c++
namespace quasar::coro::await {
	template<class Coro> struct delegate;
	template<bool Destructive> struct handoff;
	template<class... Ts> struct callback;
	template<class T> struct fetch;
}
```

### `await::delegate<Coro>`
This awaitable is constructed with any type `Coro` representing a valid coroutine.
Control is transferred to this delegated coroutine, which determines how control is returned to the awaiting coroutine.
The `Coro` type must be compatible with the following interface, where `CallerPromise` is the promise type of the awaiting coroutine:
```c++
struct Promise {
	void set_continuation(std::coroutine_handle<CallerPromise>);
	void rethrow();       // optional
	AnyType get_result(); // optional
};

struct Coro {
	bool done() const;
	Promise& promise() const;
	explicit operator std::coroutine_handle<void>() const;
};
```

### `await::handoff<Destructive>`
This awaitable is constructed with a `std::coroutine_handle<void>` to which control is transferred.
If the handle is `nullptr`, control remains in the awaiting coroutine without suspending.
If the handle is `std::noop_coroutine()`, control is transferred back to the resumer.
If `Destructive` is true and control is actually transferred (i.e. the handle is not `nullptr`) then the calling coroutine frame is destroyed before the control transfer. This can be useful for coroutines not managed by the lifetime of an object.

### `await::callback<Ts...>`
This awaitable is constructed with an arbitrary function object (the functor) that accepts a callable of signature `void(Ts...)` (the completion handler).
The functor is immediately invoked upon construction of the `callback` object, and is passed the completion handler; code inside the functor is free to copy the completion handler, but must not form references to it.
If the completion handler is invoked inside the functor, the awaiting coroutine is not suspended and completes the `co_await` synchronously.
This awaitable type is meant to allow interfacing with callback-based APIs so that they can accept the current coroutine as a callback function.
Note that all types in `Ts...` must not be cv-`void` and must be move-constructible; reference types are permitted as well.

### `await::fetch<T>`
This awaitable is constructed with an arbitrary value that is immediately returned to the awaiting coroutine, without suspending.
It is meant to be used as a method of synchronously passing data in cases were a normal function return is not feasible, such as getting data into the coroutine frame from the promise object.

## Coroutine Handle Types
The 2 main coroutine handle types provided are `coroutine` and `unique_coroutine`.
They represent non-owning and owning handles to coroutine frames respectively.

```c++
namespace quasar::coro {
	template<class Promise> struct coroutine;
	template<class Promise> struct unique_coroutine;
}
```
### `coroutine`
This handle type represents a non-owning reference of the coroutine frame.
It can be constructed from a promise reference or from a `std::coroutine_handle` with the same promise type.
If the promise type provides a `void rethrow()` function it will be invoked after every resumption of the coroutine.
This is to allow exception-handling promise types to re-raise exceptions to the calling function.

### `unique_coroutine`
This handle type represents exclusive ownership of the coroutine frame.
The frame is automatically cleaned up if the `unique_coroutine` object representing it is destroyed (e.g. by going out of scope).
By `co_await`ing an r-value of this type, you can delegate to the represented coroutine.
Upon completion of the delegated coroutine, control will return to the awaiting coroutine.

## Promise Base Types
The promise base types allow users to quickly & easily create custom promise types by simply inheriting from them.

```c++
namespace quasar::coro::promise {
	struct base;

	struct eager;
	struct lazy;

	struct nothrow;
	struct unwind_on_exception;

	struct pause_on_finish;
	struct destroy_on_finish;
	template<bool pause> struct delegatable;

	template<class T> struct result;

	template<class T> struct yield;
	template<class T, class Base, class Itr> struct delegating_yield;
}
```
### `promise::base`
This type provides the `get_return_object()` function returning a `coroutine`.
The result type of actual coroutines can be any type implicitly constructible from a `std::coroutine_handle`.

```c++
struct Promise : quasar::coro::promise::base ... { ... };
struct Handle {
	using promise_type = Promise;
	Handle(std::coroutine_handle<promise_type>);
};

quasar::coro::coroutine<Promise> coro1(){ co_return; }
quasar::coro::unique_coroutine<Promise> coro2(){ co_return; }
Handle coro3(){ co_return; }
```

### Initialization Support
- `promise::eager`
- `promise::lazy`

The initialization support promise bases provide the `initial_suspend()` function required by the compiler for the coroutine machinery.
`promise::eager` will result in a coroutine that begins executing when the coroutine is called.
`promise::lazy` will result in a coroutine that suspends before beginning execution and will wait to be resumed by the caller.

### Exception Support
- `promise::nothrow`
- `promise::unwind_on_exception`

The exception handling promise bases provide the `unhandled_exception()` function required by the compiler coroutine machinery.
`promise::nothrow::unhandled_exception()` will always call `std::terminate()` and does not provide a `rethrow()` function.
`promise::unwind_on_exception::unhandled_exception()` will store the current exception and rethrow it in `rethrow()`.

### Continuation Support
- `promise::pause_on_finish`
- `promise:delegatable<bool pause>`

The continuation-support promise bases provide the `final_suspend()` function required by the compiler coroutine machinery.
`promise::pause_on_finish` will always pause the coroutine at the final suspend point.

`promise::delegatable` provides a `set_continuation()` function, allowing it to be used with `await::delegate` and will resume the caller at the final suspend point.
It takes a single bool template parameter to indicate whether it should return control to its resumer (true) or continue past the final suspend point and self-destruct (false) if there is no continuation set.

### `promise::result<T>`
This type provides the `return_value()` function (or the `return_void()` function if `T` is cv-`void`), and the `get_result()` function which allows `await::delegate` to pass the returned value to the awaiting coroutine.

```c++
struct Promise : quasar::coro::promise::result<int> ... { ... };

quasar::coro::unique_coroutine<Promise> coro1(){ co_return 42; }
SomeCoroutineType coro2(){ int x = co_await coro1(); }
```

### Yield Support
`yield<T>` provides the `yield_value()` function (`T` must not be cv-`void`), and the `get_value()` function which allows `yield_iterator<T>` to pass the yielded value to the awaiting coroutine.

`delegating_yield<T>` provides an additional overload of `yield_value()` which allows `co_yield`ing another coroutine with the same yield type.
Once the delegated coroutine has yielded all its values and completed, control returns to this coroutine.

```c++
struct Promise1 : quasar::coro::promise::yield<char const *> ... { ... };
struct Promise2 : quasar::coro::promise::delegating_yield<char const *> ... { ... };

quasar::coro::unique_coroutine<Promise1> coro1(){ co_yield "hello"; }
quasar::coro::unique_coroutine<Promise2> coro2(){
	co_yield coro1();
	co_yield "world";
}
```
## Utilities

### `yield_iterator<T>`
This type provides some iterator semantics such as the dereference, arrow & pre-increment operators and equality comparisons with `std::default_sentinel_t`.
However it is neither copyable nor movable, since coroutine promises may need to keep references to it.
As such it does not satisfy most iterator concepts besides `std::indirectly_readable`.

### `yield_range<Coro>`
This type is a simple wrapper around the provided coroutine type and provides a `begin()` & `end()` to allow it to interface with STL range functions.
`begin()` returns a `yield iterator<T>` (deducing `T` from `promise().get_value()` of the provided coroutine).
`end()` always returns `std::default_sentinel`.


## Common Coroutine Types
Some common use-cases have generic promise types already available
```c++
namespace quasar::coro {
	template<class Result> struct task;
	template<class Yield, class Result = void> struct simple_generator;
	template<class Yield, class Result = void> struct generator;
}
```

### `task<Result>`
A task produces a single value of type `Result` asynchronously, with lazy initialization.

```c++
quasar::coro::task<std::string> load_data(){
	std::string data = co_await some_io_operation();
	co_return data;
}
```

### `simple_generator<Yield, Result>` & `generator<Yield, Result>`
Both of these types yield values of type `Yield`; generators can delegate to other coroutines of any type (as long as the yielded type is also `Yield`) but simple generators cannot.
Both of these types also return a single value of type `Result`.
When `Result` is not `void`, care must be taken when using these generator types with `yield_range` as the result may be unintentionally ignored.

```c++
quasar::coro::simple_generator<std::string, int> inner(){
	co_yield "a";
	co_yield "multi-string";
	co_return 42;
}

quasar::coro::generator<std::string, int> outer(){
	co_yield "this";
	co_yield "is";
	auto x = co_yield inner(); // good: operator co_await automatically propagates the return value
	co_yield "message";
	co_return x;
}

int func(){
	for(std::string str : quasar::coro::yield_range{outer()}){
		std::println("message: '{}'", str);
	}
	// bad: the yield_range has now been destroyed by we didn't read the return value

	auto range = quasar::coro::yield_range{outer()};
	for(std::string str : range){
		std::println("message: '{}'", str);
	}
	return range.task.promise().get_result();
	// good: the yield_range has kept the coroutine frame alive long enough to read the result
}
```
