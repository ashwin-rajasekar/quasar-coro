# Quasar C++ Coroutine Support Library

This is a library for supporting C++20 coroutines by making common coroutine handle types and providing base-classes as building blocks for creating custom promise types. All library-provided types are in the `quasar::coro` namespace.

## Using quasar-coro
Building quasar-coro requires a c++23 compliant compiler. There are no external library dependencies, and the library is currently header-only (though this may change in the future due to implementation needs). The library is build with CMake and produces a single target `quasar::coro` that consumers should link against. The library can be included in a project via CMake's `FetchContent` or via git submodule reference. a CMake example is provided below:

```cmake
FetchContent_Declare(
	quasar-coro
	GIT_REPOSITORY https://github.com:/ashwin-rajasekar/quasar-coro
	GIT_TAG main
)

FetchContent_Declare(quasar-coro)

add_executable(some_exe some_source.cpp)
target_link_libraries(some_exe quasar::coro)
```
# Library Overview
- [Awaitables](#awaitables)
	- [`await::delegate<Coro>`](#awaitdelegatecoro)
	- [`await::handoff`](#awaithandoff)
- [Coroutine Handle Types](#coroutine-handle-types)
	- [`coroutine`](#coroutine)
	- [`unique_coroutine`](#unique_coroutine)
- [Promise Base Types](#coroutine-handle-types)
	- [`base`](#promisebase)
	- [Exception Support](#exception-support)
	- [Continuation Support](#continuation-support)
	- [`promise::result<T>`](#promiseresultt)
	- [Yield Support](#yield-support)
- [Common Coroutine Types](#common-coroutine-types)
	- [`task<Result>`](#taskresult)
	- [`simple_generator<Yield, Result>` & `generator<Yield, Result>`](#simple_generatoryield-result--generatoryield-result)

## Awaitables
The awaitable types provide the hook into the compiler coroutine machinery to allow control transfer.

```c++
namespace quasar::coro::await {
	template<class Coro> struct delegate;
	struct handoff;
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

### `await::handoff`
This awaitable is constructed with a `std::coroutine_handle<void>` to which control is transferred.
If the handle is `nullptr`, control remains in the awaiting coroutine without suspending.
If the handle is `std::noop_coroutine()`, control is transferred back to the resumer.

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
The if the promise type provides a `void rethrow()` function it will be invoked after every resumption of the coroutine.
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

	struct nothrow;
	struct unwind_on_exception;

	struct pause_at_finish;
	template<bool pause> struct delegatable;

	template<class T> struct result;

	template<class T> struct yield;
	template<class T, class Base, class Itr> struct delegating_yield;
}
```
### `promise::base`
This type provides the `get_return_object()` function returning a `coroutine`.
The result type of actual coroutines can be any type implicitly constructible from a `coroutine`.

```c++
struct Promise : quasar::coro::promise::base ... { ... };
struct Handle {
	using promise_type = Promise;
	Handle(quasar::coro::coroutine);
};

quasar::coro::coroutine<Promise> coro1(){ co_return; }
quasar::coro::unique_coroutine<Promise> coro2(){ co_return; }
Handle coro3(){ co_return; }
```

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
This type provides the `return_value()` function (or the `return_void()` function if `T` is `void`), and the `get_result()` function which allows `await::delegate` to pass the returned value to the awaiting coroutine.

```c++
struct Promise : quasar::coro::promise::result<int> ... { ... };

quasar::coro::unique_coroutine<Promise> coro1(){ co_return 42; }
SomeCoroutineType coro2(){ int x = co_await coro1(); }
```

### Yield Support
`yield<T>` provides the `yield_value()` function (`T` must not be `void`), and the `get_value()` function which allows `yield_iterator<T>` to pass the yielded value to the awaiting coroutine.

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
	for(auto itr = range.begin(); itr != range.end(); ++itr){
		std::println("message: '{}'", *itr);
	}
	return range.task.promise().get_result();
	// good: the yield_range has kept the coroutine frame alive long enough to read the result
}
```
