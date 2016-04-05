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

class unspecified_result_type {};

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


template<class R, class T>
R match_invoke(R*, T const &) {
}

template<class R, class T, class M, class... Ms>
decltype(auto) match_invoke(R*, T x, M const & m, Ms const & ... ms);


template<class R, class M, class T>
auto case_invoke(R*, M const & mc, T x, int)
-> decltype(mc(x.get())) {
  return mc(x.get());
}

template<class R, class Cond, class F, class T>
auto case_invoke(R*, match_case<Cond, F> const & mc, T, char)
-> decltype(mc()) {
  return mc();
}


template<class R, class T, class M, class... Ms>
void match_m0_invoke(R r, bool b, T x, M const & m, Ms const & ... ms) {
  if (b) {
    return case_invoke(r, m, x, 1);
  }
  else {
    return match_invoke(r, x, ms...);
  }
}

template<class R, class T, class M, class... Ms>
decltype(auto)
match_m0_invoke(R r, std::true_type, T x, M const & m, Ms const & ...) {
  return case_invoke(r, m, x, 1);
}

template<class R, class T, class M, class... Ms>
decltype(auto)
match_m0_invoke(R r, std::false_type, T x, M const &, Ms const & ... ms) {
  return match_invoke(r, x, ms...);
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

template<class R, class T, class M, class... Ms>
decltype(auto) match_invoke(R* r, T x, M const & m, Ms const & ... ms) {
  return match_m0_invoke(r, is_invokable(x, m, 1), x, m, ms...);
}


template<class Int, std::size_t... Ints, class Tuple, class F>
decltype(auto) tuple_apply(std::integer_sequence<Int, Ints...>, Tuple & t, F f) {
  return f(std::get<Ints>(t)...);
}

template<class Tuple, class F>
decltype(auto) tuple_apply(Tuple & t, F f) {
  return tuple_apply(std::make_index_sequence<std::tuple_size<Tuple>{}>{}, t, f);
}


template<class R, class... Cs>
struct match
{
  template<class... C>
  constexpr match(C && ... c)
  : t(std::forward<C>(c)...)
  {}

  template<class C>
  match<R, Cs..., C> operator|(C && c) {
    return tuple_apply(t, [&c](Cs & ... cases) {
      return match<R, Cs..., C>{
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
      return match_invoke(
        static_cast<R*>(nullptr),
        forwarder<T>{x}, cases...
      );
    });
  }

private:
  // TODO fast_tuple
  std::tuple<Cs...> t;
};


template<class T, class R, class... C>
R operator>>=(T && x, match<R, C...> const & m) {
  return m(std::forward<T>(x));
}

} } } // detail_


namespace ctmatch
{
  template<class Cond>
  detail_::ctmatch::match_case<Cond>
  match_case(Cond && cond) {
    return {std::forward<Cond>(cond)};
  }

  template<class Cond, class F>
  detail_::ctmatch::match_case<Cond, F>
  match_case(Cond && cond, F && f) {
    return {std::forward<Cond>(cond), std::forward<F>(f)};
  }

  template<class T>
  auto match_value(T const & x) {
    //return match_case(la::ref(x) == la::arg1);
    return match_case([&x](auto const & y) -> decltype(x == y) { return x == y; });
  }

  template<class T, class F>
  auto match_value(T const & x, F && f) {
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

  // TODO
  //using unspecified_result_type = detail_::ctmatch::unspecified_result_type;
  using unspecified_result_type = void;

  template<class R = unspecified_result_type, class T, class... Cases>
  decltype(auto) match_invoke(T && x, Cases const & ... cases) {
    detail_::ctmatch::match_invoke(
      static_cast<R*>(nullptr),
      forwarder<T>{x}, cases...
    );
  }

  struct match_fn
  {
    template<class C>
    detail_::ctmatch::match<unspecified_result_type, C>
    operator|(C && c) const {
      return {std::forward<C>(c)};
    }

    template<class T>
    void operator()(T &&) const {
    }

    template<class R>
    detail_::ctmatch::match<R>
    result() const {
      return {};
    }
  };

  namespace {
    constexpr match_error_fn match_error = {};
    constexpr match_always_fn match_always = {};

    constexpr match_fn match = {};
  }
} // ctmatch

}

#endif
