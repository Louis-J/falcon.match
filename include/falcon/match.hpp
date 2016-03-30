#ifndef FALCON_MATCH_HPP
#define FALCON_MATCH_HPP

#include <utility>

namespace falcon {

namespace detail_ { namespace match { namespace {

template<class Cond = void, class F = void>
struct match_case
{
  Cond cond;
  F f;

  template<class... T>
  auto operator()(T && ... x) const
  -> decltype(f(std::forward<T>(x)...)) {
    return f(std::forward<T>(x)...);
  }
};

template<class Cond>
struct match_case<Cond, void>
{
  Cond cond;

  template<class F>
  match_case<Cond, F> operator>>(F && f) const {
    return {std::move(cond), std::forward<F>(f)};
  }
};

template<>
struct match_case<void, void>
{
  template<class Cond>
  match_case<Cond> operator()(Cond && cond) const {
    return {std::forward<Cond>(cond)};
  }

  template<class Cond, class F>
  match_case<Cond, F> operator()(Cond && cond, F && f) const {
    return {std::forward<Cond>(cond), std::forward<F>(f)};
  }
};


template<class T>
void match_invoke(T const &) {
}

template<class T, class M, class... Ms>
void match_invoke(T && x, M const & m, Ms const & ... ms);


template<class T, class Cond, class F>
auto match_case_invoke(int, T && x, match_case<Cond, F> const & mc)
-> decltype(mc(std::forward<T>(x))) {
  return mc(std::forward<T>(x));
}

template<class T, class Cond, class F>
decltype(auto) match_case_invoke(char, T &&, match_case<Cond, F> const & mc) {
  return mc();
}


template<class T, class Cond, class F, class... Ms>
void match_m0_invoke(bool b, T && x, match_case<Cond, F> const & mc, Ms const & ... ms) {
  if (b) {
    match_case_invoke(1, std::forward<T>(x), mc);
  }
  else {
    match_invoke(std::forward<T>(x), ms...);
  }
}

template<class T, class M, class... Ms>
void match_m0_invoke(bool b, T && x, M const & m, Ms const & ... ms) {
  if (b) {
    m(std::forward<T>(x));
  }
  else {
    match_invoke(std::forward<T>(x), ms...);
  }
}


template<class T, class M, class... Ms>
decltype(auto) match_m0_invoke(std::true_type, T && x, M const & m, Ms const & ...) {
  return m(std::forward<T>(x));
}

template<class T, class Cond, class F, class... Ms>
auto match_m0_invoke(std::true_type, T && x, match_case<Cond, F> const & mc, Ms const & ...) {
  return match_case_invoke(1, std::forward<T>(x), mc);
}


template<class T, class M, class... Ms>
void match_m0_invoke(std::false_type, T && x, M const &, Ms const & ... ms) {
  match_invoke(x, ms...);
}


template<class T, class Cond, class F>
auto is_case_invokable(T const & x, match_case<Cond, F> const & mc)
-> decltype(mc.cond(x)) {
  return mc.cond(x);
}

template<class... Ts>
std::false_type is_case_invokable(Ts const & ...) {
  return {};
}

template<class T, class Cond, class F>
auto is_invokable(T && x, match_case<Cond, F> const & mc) {
  return is_case_invokable(x, mc);
}

template<class T, class F>
auto is_invokable(T && x, F const & fn)
-> decltype(fn(std::forward<T>(x)), std::true_type{}) {
  return {};
}

template<class... Ts>
std::false_type is_invokable(Ts const & ...) {
  return {};
}

template<class T, class M, class... Ms>
void match_invoke(T && x, M const & m, Ms const & ... ms) {
  match_m0_invoke(is_invokable(x, m), std::forward<T>(x), m, ms...);
}


template<class... Cs>
struct match : private Cs...
{
  template<class... C>
  constexpr match(C && ... c)
  : Cs(std::forward<C>(c))...
  {}

  template<class C>
  match<Cs..., C> operator|(C && c) {
    return {static_cast<Cs&&>(*this)..., std::forward<C>(c)};
  }

  template<class T>
  void operator()(T && x) const {
    return match_invoke(std::forward<T>(x), static_cast<Cs const &>(*this)...);
  }
};

template<>
struct match<>
{
  template<class C>
  match<C> operator|(C && c) const {
    return {std::forward<C>(c)};
  }

  template<class T>
  void operator()(T &&) const {
  }
};

template<class T, class... C>
void operator>>=(T && x, match<C...> const & m) {
  return m(std::forward<T>(x));
}

} } }


struct match_invoke_fn
{
  template<class T, class... M>
  void operator()(T && x, M && ... m) const {
    detail_::match::match_invoke(std::forward<T>(x), m...);
  }
};

struct match_case_fn : detail_::match::match_case<> {};

using match_fn = detail_::match::match<>;

namespace {
  constexpr match_invoke_fn match_invoke = {};
  constexpr match_case_fn match_case = {};
  constexpr match_fn match = {};
}

}

#endif
