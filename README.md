# Falcon.CtMatch - Type matching for C++

[![Build Status](https://travis-ci.org/jonathanpoelen/falcon.match.svg?branch=master)](https://travis-ci.org/jonathanpoelen/falcon.match)

[![Build Status](https://ci.appveyor.com/api/projects/status/github/jonathanpoelen/falcon.match)](https://ci.appveyor.com/project/jonathanpoelen/falcon-match)

# Goal

Run the first function that can be (matching type), while having the possibility to condition through a predicate (matching value).

# Notes

Only predicates that return a bool type create a conditional branche at runtime. This is not the case if the return type is convertible to `std::integral_constant` or if a function `normalize_branch_value(T)` returns a `std::integral_constant` type.


# falcon::ctmatch

## Functions

 - `match_if(Pred, Action|Fn)`, `match_if(Pred) >> Action|Fn`
 - `match_value(T, Action|Fn)`, `match_value(T) >> Action|Fn`
 - `match_invoke<ResultType>(T, Fns|Cases...)`

 - `pmatch_invoke(T, Fns...)`
 - `pmatch(T) | Fns ...`


### Types categories

 - `Pred`: A function than returns std::true_type, std::false_type or boolean value.
 - `Action`: A function without paramerters.
 - `Fn`: A function with one parameter.
 - `Cases`: `Fn`, `match_if()` or `match_value()`
 - `ResultType`: `unspecified_result_type` (by default), `common_result_type` or any type.


## Classes

 - `unspecified_result_type`: Type of invoked cases if identical. Otherwise void.
 - `common_result_type`: Common type of invoked cases. Otherwise void.

 - `match_error_fn`: Launches a compilation time error if used.
 - `match_always_fn`: Do nothing and do it well.

 - `x >>= match_fn<bool CheckMismatch>{} | Cases ...`, `(match_fn<bool>{} | Cases ...)(x)`: If CheckMismatch = 1, `match_error_fn{}` is automatically added.


## Variables

 - `match_error_fn match_error`
 - `match_always_fn match_always`

 - `match_fn<1> match`
 - `match_fn<0> nmatch`


# Samples

```cpp
using namespace falcon::ctmatch;

auto _ = falcon::lambda::placeholders::_1;

template<class T>
foo(T x) {
  x >>= match//.result<ResultType>() or common_result()
  | [](bool &) { std::cout << "is a boolean\n"; }
  | match_case(_ < 0) >> [](auto x){ std::cout << x << " is a negative integer\n"; }
  | match_case(_ > 0) >> [x]{ std::cout << x << " is a positive integer\n"; }
  | match_case(_ == 0) >> []{ std::cout << "is a zero\n"; }
  | [](auto const & x) -> decltype(void(std::to_string(x))) { std::cout << "is a stringable type\n"; }
  | [](auto const &) { std::cout << "unknown type\n"; }
  ;
}
```

```cpp
using namespace falcon::ctmatch::pmatch;

template<class T>
foo(T x) {
  pmatch(x)
  | [](bool &) { std::cout << "bool\n"; }
  | [](int &) { std::cout << "int\n"; }
  | [](double &) { std::cout << "double\n"; }
  ;
}
```

