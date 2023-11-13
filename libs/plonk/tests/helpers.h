#pragma once

#include <cmath>
#include <functional>
#include <iostream>

class AssertionFailure : public std::runtime_error {
public:
	AssertionFailure(const char *message) : std::runtime_error(message) {}
};

#define it(name, ...) \
	try { \
		([&]()__VA_ARGS__)(); \
		std::cerr << "\x1b[48;5;47m\x1b[38;5;0m PASS \x1b[0m " << name << "\n"; \
	} \
	catch(const std::exception &e) { \
		std::cerr << "\x1b[48;5;196m\x1b[38;5;0m FAIL \x1b[0m " << name << "\n"; \
		return 1; \
	}

#define assert_with_message(condition, message) \
	if (!(condition)) throw AssertionFailure(message);

#define assert_without_message(condition) \
	if (!(condition)) throw AssertionFailure("Assertion failed");

#define assert_select(x, A, B, FUNC, ...) FUNC
#define assert(...) assert_select(,##__VA_ARGS__, assert_with_message(__VA_ARGS__), assert_without_message(__VA_ARGS__))
#define assert_approx(actual, expected, delta, message) assert(std::abs(actual - expected) < delta, message)
