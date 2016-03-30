#include <falcon/match.hpp>

#include <iostream>

int main() {

using namespace falcon;

//using namespace detail_::match;

match_invoke(
  3
, match_case([](int*){ return false; }, [](int*) { std::cout << "c pointer\n"; })
, match_case([](auto){ return true; }) >> []() { std::cout << "c int\n"; }
, [](int *) { std::cout << "pointer\n"; }
, [](int) { std::cout << "int\n"; }
, [](auto&&) { std::cout << "unknown\n"; }
);

(match
| match_case([](int*){ return false; }, [](int*) { std::cout << "c pointer\n"; })
| match_case([](auto){ return true; }) >> []() { std::cout << "c int\n"; }
| [](int *) { std::cout << "pointer\n"; }
| [](int) { std::cout << "int\n"; }
| [](auto&&) { std::cout << "unknown\n"; }
)(3);

3 >>= match//.result<int>()
| match_case([](int*){ return false; }, [](int*) { std::cout << "c pointer\n"; })
| match_case([](auto){ return true; }) >> []() { std::cout << "c int\n"; }
| [](int *) { std::cout << "pointer\n"; }
| [](int) { std::cout << "int\n"; }
| [](auto&&) { std::cout << "unknown\n"; }
;

}
