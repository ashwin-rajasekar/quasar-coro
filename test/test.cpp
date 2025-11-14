#include <gtest/gtest.h>

#ifndef QUASAR_CORO_MODULES
	#include <quasar/coro/coroutine.hpp>
	#include <quasar/coro/promise.hpp>
	#include <quasar/coro/yield.hpp>

#else
	#include <coroutine>
	import quasar.coro;

#endif

using namespace quasar::coro;

namespace {
	struct test_promise : task_promise<void> {
		std::vector<int>& output;

		test_promise(std::vector<int>& vec, auto...) : output{vec}{}
		~test_promise() noexcept { output.push_back(-1); }
	};

	task<int> inner(std::vector<int>& output){
		output.push_back(1);
		co_return 2;
	}

	task<void> simple_delegate(std::vector<int>& output){
		output.push_back(3);
		output.push_back(co_await inner(output));
		output.push_back(4);
	}

	unique_coroutine<test_promise> nondestructive_handoff(std::vector<int>& output){
		output.push_back(3);
		co_await await::handoff<false>{inner(output)};
		output.push_back(4);
	}

	// the coroutine resumed must not live inside the frame of this one,
	// else it will be destroyed during the handoff before it can be resumed
	coroutine<test_promise> destructive_handoff(std::vector<int>& output, std::coroutine_handle<void> routine){
		output.push_back(3);
		co_await await::handoff<true>{routine};
	}
}

TEST(AwaiterTest, SimpleDelegate){
	std::vector<int> checkpoints, expected{3, 1, 2, 4};
	auto task = simple_delegate(checkpoints);
	while(!task.done()){ task(); }
	EXPECT_EQ(checkpoints, expected);
}

TEST(AwaiterTest, NonDestructiveHandoff){
	std::vector<int> checkpoints, expected{3, 1, 4, -1};
	{
		auto task = nondestructive_handoff(checkpoints);
		while(!task.done()){ task(); }
	}
	EXPECT_EQ(checkpoints, expected);
}

TEST(AwaiterTest, DestructiveHandoff){
	std::vector<int> checkpoints, expected{3, -1, 1};
	{
		auto routine = inner(checkpoints);
		auto task = destructive_handoff(checkpoints, routine);
		task();
	}
	EXPECT_EQ(checkpoints, expected);
}
