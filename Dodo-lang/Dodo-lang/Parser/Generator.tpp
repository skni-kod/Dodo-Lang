#ifndef GENERIC_GENERATOR_TPP
#define GENERIC_GENERATOR_TPP

#include <coroutine>
#include <utility>
#include <exception>
#include <concepts>
#include "Parser.hpp"

// This is a generic template for a generator coroutine for use in my projects
// it might be replaced when C++ 23 is fully implemented.

template<typename T>
struct Generator {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        T value_;
        std::exception_ptr exception_;

        Generator get_return_object() {
            return Generator(handle_type::from_promise(*this));
        }
        std::suspend_always initial_suspend() {
            return {};
        }
        std::suspend_always final_suspend() noexcept {
            return {};
        }
        void unhandled_exception() {
            exception_ = std::current_exception();
        }
        //template<std::convertible_to<T> From>
        std::suspend_always yield_value(T &&from) {
            value_ = std::forward<T>(from);
            return {};
        }
        void return_void() {}
    };

    handle_type h_;

    explicit Generator(handle_type h) : h_(h) {}
    Generator(const Generator &) = delete;
    ~Generator() {
        h_.destroy();
    }
    explicit operator bool() {
        fill();
        return !h_.done();
    }
    T operator()() {
        fill();
        full_ = false;
        return std::move(h_.promise().value_);
    }

private:
    bool full_ = false;

    void fill() {
        if (!full_) {
            if (h_.done()) {
                ParserError("unexpected end of file!");
            }
            h_();
            if (h_.promise().exception_)
                std::rethrow_exception(h_.promise().exception_);
            full_ = true;
        }
    }
};

#endif //GENERIC_GENERATOR_TPP