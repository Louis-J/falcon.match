/* The MIT License (MIT)

Copyright (c) 2016 Jonathan Poelen <jonathan.poelen+ctmatch@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <utility>
#include <tuple>

// TODO utility/forwarder.hpp : Falcon.Forwarder
namespace falcon {

/**
 * \brief Wraps a lvalue or a rvalue in a copyable, assignable object.
 * \ingroup utilities
 * @{
 */
template<class T>
struct forwarder
{
  using value_type = T;
  using type = T &&;

  constexpr forwarder(T & x) noexcept : x_(x) {}

  constexpr T && get() const { return static_cast<T&&>(x_); }
  constexpr T & ref() const { return x_; }
  constexpr T const & cref() const { return x_; }

private:
  T & x_;
};

template<class T>
struct forwarder<T&>
{
  using value_type = T;
  using type = T &;

  constexpr forwarder(T & x) noexcept : x_(x) {}

  constexpr T & get() const { return x_; }
  constexpr T & ref() const { return x_; }
  constexpr T const & cref() const { return x_; }

private:
  T & x_;
};
/** @} group utilities */

} // falcon


// TODO Falcon.Traits
namespace falcon {

/**
 * \defgroup metaprogramming Metaprogramming
 * \addtogroup utilities
 * @{
 */
template<class...> using void_t = void;
// template<class T> using t_ = typename T::type;

// template<class TT, class Default>
// struct eval_or_type
// { using type = Default; };
/** @} group utilities */

} // falcon


// TODO Falcon.Tuple
namespace falcon {

/**
 * \addtogroup utilities
 * @{
 */
/// \brief Invoke the callable object f with the Ith elements from the tuple of arguments.
template<class TInt, std::size_t... I, class F, class Tuple>
decltype(auto) apply(std::integer_sequence<TInt, I...>, F && f, Tuple && t) {
  // TODO falcon::invoke
  return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

/// \brief Invoke the Callable object f with a tuple of arguments.
template<class F, class Tuple>
decltype(auto) apply(F && f, Tuple && t) {
  return apply(
    std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>{}>{}
  , std::forward<F>(f), std::forward<Tuple>(t)
  );
}
/** @} group utilities */

}


namespace falcon {

/*
 * using namespace falcon::ctmatch
 *
 * match(x)//.result<Result>() or common_result()
 * | match_if([](auto const & x) { return bool(x); }) >> []{ ... }
 * | match_if([](auto const & x) { return std::is_pointer<decltype(x)>{}; }) >> []{ ... }
 * | match_if(...) >> [](auto && x){ ... }
 * | match_value(0) >> func)
 * | match_value(0, func)
 * | [](only_integral_t) { ... }
 * | [](auto && any) { ... } // always true, the following are ignored
 * | match_always // always true, the following are ignored
 * | match_error // by default with match (not with nmatch)
 *
 * (match//.result<Result>() or common_result()
 * | cases | ...
 * )(x)
 *
 * match_invoke(x, cases...)
 * match_invoke<Result>(x, cases...)
 *
 * Result is a type or a category (common_result_type or unspecified_result_type).
 */

namespace ctmatch {

namespace detail_ {
  template<class Pred, class F = void>
  struct match_if;

  template<bool CheckMismatch, class R, class... Cs>
  struct match;

  template<class R, class T>
  R match_invoke(R*, T const &) {
  }

  template<class R, class T, class M, class... Ms>
  decltype(auto) match_invoke(R*, T x, M const & m, Ms const & ... ms);
} // detail_

/**
 * \defgroup Matching Matching
 * \addtogroup utilities
 * @{
 */

/**
 * @defgroup match_result_type Matching Result Type
 * These are empty types, used to specified the result type of match.
 */
/// @{
/// \brief The result type of match is same than all invoked cases. Otherwise void.
class unspecified_result_type {};
/// \brief The result type of match is common type of invoked cases. Otherwise void.
class common_result_type {};
/// @}


/// \brief Condtional match.
template<class Pred>
detail_::match_if<Pred>
match_if(Pred && pred) {
  return {std::forward<Pred>(pred)};
}

/// \brief Condtional match.
template<class Pred, class F>
detail_::match_if<Pred, F>
match_if(Pred && pred, F && f) {
  return {std::forward<Pred>(pred), std::forward<F>(f)};
}


/// \brief Condtional match on value.
template<class T>
auto match_value(T const & x) {
  //return match_if(la::ref(x) == la::arg1);
  return match_if([&x](auto const & y) -> decltype(x == y) { return x == y; });
}

/// \brief Condtional match on value.
template<class T, class F>
auto match_value(T const & x, F && f) {
  return match_if([&x](auto const & y) -> decltype(x == y) { return x == y; }, std::forward<F>(f));
}


/// \brief Launches a compilation time error.
struct match_error_fn
{
  template<class T>
  void operator()(T const & type_error) const {
    struct match_error_invocation {} is_not_invokable_with = type_error;
  }
};

/// \brief Do nothing and do it well.
struct match_always_fn
{
  template<class T>
  void operator()(T const &) const {
  }
};


/// \brief Uses the first invokable case.
template<class R = unspecified_result_type, class T, class... Cases>
decltype(auto) match_invoke(T && x, Cases const & ... cases) {
  detail_::match_invoke(static_cast<R*>(nullptr),forwarder<T>{x}, cases...);
}


/// \brief Wraps \c match_invoke.
template<bool CheckMismatch>
struct match_fn
{
  template<class C>
  detail_::match<CheckMismatch, unspecified_result_type, C>
  operator|(C && c) const {
    return {std::forward<C>(c)};
  }

  template<class T>
  void operator()(T && x) const {
    std::conditional_t<CheckMismatch, match_always_fn, match_error_fn>{}(x);
  }

  /// \brief Specifies the result type.
  template<class R>
  detail_::match<CheckMismatch, R>
  result() const {
    return {};
  }

  /// \brief The result type is the common type.
  detail_::match<CheckMismatch, common_result_type>
  common_result() const {
    return {};
  }
};


namespace {
  constexpr match_error_fn match_error = {};
  constexpr match_always_fn match_always = {};

  /// \brief Match with check mismatch.
  constexpr match_fn<1> match = {};

  /// \brief Match without check mismatch.
  constexpr match_fn<0> nmatch = {};
}
/** @} group utilities */


//
// Implementation
//

namespace detail_ {

template<class Pred, class F>
struct match_if
{
  Pred pred;
  F f;

  template<class... T>
  auto operator()(T && ... x) const
  -> decltype(f(std::forward<T>(x)...)) {
    return f(std::forward<T>(x)...);
  }
};

template<class Pred>
struct match_if<Pred, void>
{
  Pred pred;

  template<class F>
  match_if<Pred, F> operator>>(F && f) const {
    return {std::move(pred), std::forward<F>(f)};
  }
};


template<class M, class T>
auto case_invoke(M const & mc, T x, int)
-> decltype(mc(x.get())) {
  return mc(x.get());
}

template<class Pred, class F, class T>
auto case_invoke(match_if<Pred, F> const & mc, T, char)
-> decltype(mc()) {
  return mc();
}


struct match_m0_invoke_void
{
  template<class R, class T, class M, class... Ms>
  static void impl(
    R* r, bool b, T x, M const & m, Ms const & ... ms
  ) {
    if (b) {
      case_invoke(m, x, 1);
    }
    else {
      match_invoke(r, x, ms...);
    }
  }
};

template<class Rs>
struct match_m0_invoke_with_result
{
  template<class R, class T, class M, class... Ms>
  static Rs impl(
    R* r, bool b, T x, M const & m, Ms const & ... ms
  ) {
    if (b) {
      return case_invoke(m, x, 1);
    }
    else {
      return match_invoke(r, x, ms...);
    }
  }
};


template<class R, class Rs1, class Rs2, class = void>
struct match_m0_invoke_impl
: match_m0_invoke_void
{};

template<class R, class Rs1, class Rs2>
struct match_m0_invoke_impl<
  R, Rs1, Rs2,
  std::enable_if_t<
    std::is_convertible<std::common_type_t<Rs1, Rs2>, R>::value
  >
>
: match_m0_invoke_with_result<std::common_type_t<Rs1, Rs2>>
{};

template<class Rs>
struct match_m0_invoke_impl<unspecified_result_type, Rs, Rs, void>
: match_m0_invoke_with_result<Rs>
{};

template<class Rs1, class Rs2>
struct match_m0_invoke_impl<
  common_result_type, Rs1, Rs2,
  void_t<std::common_type_t<Rs1, Rs2>>
> : match_m0_invoke_with_result<std::common_type_t<Rs1, Rs2>>
{};


template<class R, class T, class M, class... Ms>
decltype(auto)
match_m0_invoke(R*, std::true_type, T x, M const & m, Ms const & ...) {
  return case_invoke(m, x, 1);
}

template<class R, class T, class M, class... Ms>
decltype(auto)
match_m0_invoke(R* r, std::false_type, T x, M const &, Ms const & ... ms) {
  return match_invoke(r, x, ms...);
}

template<class R, class T, class M, class... Ms>
decltype(auto)
match_m0_invoke(R* r, bool b, T x, M const & m, Ms const & ... ms) {
  return match_m0_invoke_impl<
    R,
    decltype(case_invoke(m, x, 1)),
    decltype(match_invoke(r, x, ms...))
  >::impl(r, b, x, m, ms...);
}


template<class T, class Pred, class F>
auto is_invokable(T x, match_if<Pred, F> const & mc, int)
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


template<bool CheckMismatch, class R, class... Cs>
struct match
{
  template<class... C>
  constexpr match(C && ... c)
  : t(std::forward<C>(c)...)
  {}

  template<class C>
  match<CheckMismatch, R, Cs..., C> operator|(C && c) {
    return apply([&c](Cs & ... cases) {
      return match<CheckMismatch, R, Cs..., C>{
        std::move(cases)
#ifndef IN_IDE_PARSER
      ...
#endif
      , std::forward<C>(c)
      };
    }, t);
  }

  template<class T>
  decltype(auto) operator()(T && x) const {
    return apply([&x](Cs const & ... cases) {
      return match_invoke(
        static_cast<R*>(nullptr)
      , forwarder<T>{x}, cases
#ifndef IN_IDE_PARSER
      ...
#endif
      , std::conditional_t<CheckMismatch, match_error_fn, match_always_fn>{}
      );
    }, t);
  }

private:
  // TODO fast_tuple
  std::tuple<Cs...> t;
};


template<class T, bool CheckMismatch, class R, class... C>
auto operator>>=(T && x, match<CheckMismatch, R, C...> const & m)
-> decltype(m(std::forward<T>(x))) {
  return m(std::forward<T>(x));
}

} // detail_

} }
