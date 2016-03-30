# falcon.match
Pattern matching like in C++

```cpp
using namespace falcon;
using namespace falcon::lambda::placeholders; // falcon::placeholders::_

template<class T>
foo(T && x) {
  x >>= match
  | [](explicit_<bool>) { std::cout << "is a boolean"; }
  | match_case(_ < 0) >> []{ std::cout << "is a negative integer"; }
  | match_case(_ > 0) >> []{ std::cout << "is a positive integer"; }
  | match_case(_ == 0) >> []{ std::cout << "is a zero"; }
  | [](auto const & x) -> decltype(void(std::to_string(x))) { std::cout << "is a stringable type"; }
  | match_error
}
```

match_invoke(x, patterns...)
match(patterns...)(x)
x >>= match(patterns...)
(match | patterns | ...)(x)
x >>= match | patterns | ...
