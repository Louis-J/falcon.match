#include <falcon/ctmatch.hpp>

#include <iostream>
#include <type_traits>

template<int i> struct i_ : std::integral_constant<int, i> {};

class my_true_type {};
std::true_type normalize_branch_value(my_true_type) { return {}; }

template<class T, int i> struct Fi { i_<i> operator()(T) const { return {}; } };

template<class T> struct only {
  template<class U>
  std::enable_if_t<std::is_same<std::decay_t<U>, T>::value, std::true_type>
  operator()(U const &) const { return {}; }
};

int main() {

using namespace falcon::ctmatch;

i_<1> a;
i_<2> b;

Fi<int, 1> fa;
Fi<int, 2> fb;

only<int> only_int;
only<bool> only_bool;

std::true_type true_;
std::false_type false_;

auto dyn_error = [](auto &&) { throw 1; };

a = match_invoke(1, fa);
a = match_invoke(1, match_if(fa, fa));
a = match_invoke(1, match_if(fa) >> fa);
a = 1 >>= match | match_if([](int) { return my_true_type{}; }, fa);

match_invoke(1, match_value(1, fa), dyn_error);
match_invoke(1, match_value(1) >> fa, dyn_error);

b = match_invoke(1, match_if(only_bool, fa), match_if(only_int, fb));
b = match_invoke(1, match_if(only_bool) >> fa, match_if(only_int) >> fb);

b = 1 >>= match
| match_if(only_bool) >> fa
| match_if(only_int) >> fb
;

3 >>= match | match_always;
true_ = 3 >>= match | only_int;

only_bool(
  1 >>= match.result<bool>()
  | match_value(only_int, []() { return 1; })
  | match_value(only_int, []() { return 2; })
  | [](int){ return 3; }
);

a = pmatch_invoke(1,
  only<int*>{},
  [a](int) { return a; },
  [](auto x) { static_assert(!sizeof(x), ""); }
);
true_ = (pmatch(1) | match_always | match_error).is_invoked();
true_ = (pmatch(1) | only_bool | only_int | match_error).is_invoked();
false_ = (pmatch(1) | only_bool).is_invoked();

}
