#ifndef FALCON_MATCH_HPP
#define FALCON_MATCH_HPP

#include <utility>
#include <tuple>

// TODO utility/forwarder.hpp : Falcon.Forwarder
namespace falcon
{
  template<class T>
  struct forwarder
  {
    using value_type = T;
    using type = T &&;

    forwarder(T & x) noexcept : x_(x) {}

    T && get() const { return static_cast<T&&>(x_); }
    T & ref() const { return x_; }
    T const & cref() const { return x_; }

  private:
    T & x_;
  };

  template<class T>
  struct forwarder<T&>
  {
    using value_type = T;
    using type = T &;

    forwarder(T & x) noexcept : x_(x) {}

    T & get() const { return x_; }
    T & ref() const { return x_; }
    T const & cref() const { return x_; }

  private:
    T & x_;
  };
}

namespace falcon {

namespace detail_ { namespace ctmatch { namespace {

template<class Cond, class F = void>
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


template<class T>
void match_invoke(T const &) {
}

template<class T, class M, class... Ms>
decltype(auto) match_invoke(T x, M const & m, Ms const & ... ms);


template<class M, class T>
auto case_invoke(M const & mc, T x, int)
-> decltype(mc(x.get())) {
  return mc(x.get());
}

template<class Cond, class F, class T>
auto case_invoke(match_case<Cond, F> const & mc, T, char)
-> decltype(mc()) {
  return mc();
}


template<class T, class M, class... Ms>
void match_m0_invoke(bool b, T x, M const & m, Ms const & ... ms) {
  if (b) {
    case_invoke(m, x, 1);
  }
  else {
    match_invoke(x, ms...);
  }
}

template<class T, class M, class... Ms>
decltype(auto) match_m0_invoke(std::true_type, T x, M const & m, Ms const & ...) {
  return case_invoke(m, x, 1);
}

template<class T, class M, class... Ms>
decltype(auto) match_m0_invoke(std::false_type, T x, M const &, Ms const & ... ms) {
  return match_invoke(x, ms...);
}


template<class T, class Cond, class F>
auto is_invokable(T x, match_case<Cond, F> const & mc, int)
-> decltype(case_invoke(mc, x, 1), mc.cond(x.cref())) {
  return mc.cond(x.cref());
}

template<class T, class F>
auto is_invokable(T x, F const & fn, char)
-> decltype(case_invoke(fn, x, 1), std::true_type{}) {
  return {};
}

inline std::false_type is_invokable(...) {
  return {};
}

template<class T, class M, class... Ms>
decltype(auto) match_invoke(T x, M const & m, Ms const & ... ms) {
  return match_m0_invoke(is_invokable(x, m, 1), x, m, ms...);
}


template<class Int, std::size_t... Ints, class Tuple, class F>
decltype(auto) tuple_apply(std::integer_sequence<Int, Ints...>, Tuple & t, F f) {
  return f(std::get<Ints>(t)...);
}

template<class Tuple, class F>
decltype(auto) tuple_apply(Tuple & t, F f) {
  return tuple_apply(std::make_index_sequence<std::tuple_size<Tuple>{}>{}, t, f);
}


template<class... Cs>
struct match
{
  template<class... C>
  constexpr match(C && ... c)
  : t(std::forward<C>(c)...)
  {}

  template<class C>
  match<Cs..., C> operator|(C && c) {
    return tuple_apply(t, [&c](Cs & ... cases) {
      return match<Cs..., C>{
        std::move(cases)
#ifndef IN_IDE_PARSER
      ...
#endif
      , std::forward<C>(c)
      };
    });
  }

  template<class T>
  decltype(auto) operator()(T && x) const {
    return tuple_apply(t, [&x](Cs const & ... cases) {
      match_invoke(forwarder<T>{x}, cases...);
    });
  }

private:
  // TODO fast_tuple
  std::tuple<Cs...> t;
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

  // TODO
  template<class R>
  void result() const;
};

template<class T, class... C>
void operator>>=(T && x, match<C...> const & m) {
  return m(std::forward<T>(x));
}

} } } // detail_


namespace ctmatch
{
  template<class Cond>
  detail_::ctmatch::match_case<Cond>
  match_case(Cond && cond) const {
    return {std::forward<Cond>(cond)};
  }

  template<class Cond, class F>
  detail_::ctmatch::match_case<Cond, F>
  match_case(Cond && cond, F && f) const {
    return {std::forward<Cond>(cond), std::forward<F>(f)};
  }

  template<class T>
  auto match_value(T const & x) const {
    //return match_case(la::ref(x) == la::arg1);
    return match_case([&x](auto const & y) -> decltype(x == y) { return x == y; });
  }

  template<class T, class F>
  auto match_value(T const & x, F && f) const {
    return match_case([&x](auto const & y) -> decltype(x == y) { return x == y; }, std::forward<F>(f));
  }

  struct match_error_fn
  {
    template<class T>
    void operator()(T const & type_error) const {
      struct match_error_invocation {} is_not_invokable_with = type_error;
    }
  };

  struct match_always_fn
  {
    template<class T>
    void operator()(T const &) const {
    }
  };

  template<class T, class... Cases>
  decltype(auto) match_invoke(T && x, Cases const & ... cases) {
    detail_::ctmatch::match_invoke(forwarder<T>{x}, cases...);
  }

  using match_fn = detail_::ctmatch::match<>;

  namespace {
    constexpr match_error_fn match_error = {};
    constexpr match_always_fn match_always = {};

    constexpr match_fn match = {};
  }
} // ctmatch

}

#endif
