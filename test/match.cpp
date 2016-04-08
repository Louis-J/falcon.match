#include <falcon/match.hpp>

#include <iostream>

int main() {

using namespace falcon::ctmatch;

//using namespace detail_::match;

match_invoke(
  3
, match_if([](int*){ return false; }, [](int*) { std::cout << "c pointer\n"; })
, match_if([](auto){ return true; }) >> []() { std::cout << "c int\n"; }
, [](int *) { std::cout << "pointer\n"; }
, [](int) { std::cout << "int\n"; return 1; }
, [](auto&&) { std::cout << "unknown\n"; }
);

(match
| match_if([](int*){ return false; }, [](int*) { std::cout << "c pointer\n"; })
| match_if([](auto){ return true; }) >> []() { std::cout << "c int\n"; }
| [](int *) { std::cout << "pointer\n"; }
| [](int) { std::cout << "int\n"; }
| [](auto&&) { std::cout << "unknown\n"; }
)(3);

3 >>= match//.result<int>()
| match_if([](int*){ return false; }, [](int*) { std::cout << "c pointer\n"; })
| match_if([](auto){ return true; }) >> []() { std::cout << "c int\n"; }
| [](int *) { std::cout << "pointer\n"; }
| [](int) { std::cout << "int\n"; }
| [](auto&&) { std::cout << "unknown\n"; }
// | []() -> void_t<is_pointer<T>> { std::cout << "unknown\n"; }
;

int xx = 3 >>= match.result<int>()
| match_if([](int*){ return false; }, [](int*) { return 1; })
| match_if([](auto){ return true; }) >> []() { return 2u; }
| [](int *) { return 3u; }
| [](int) { return 4; }
| [](auto&&) { return 5; }
// | []() -> void_t<is_pointer<T>> { std::cout << "unknown\n"; }
;

3 >>= match | match_always | match_error;

}
