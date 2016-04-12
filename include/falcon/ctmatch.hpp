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

namespace detail_ {
  template<class T>
  struct forwarder_base {
    using value_type = T;
    using type = T&&;
  };

  template<class T>
  struct forwarder_base<T&> {
    using value_type = T;
    using type = T&;
  };
}

/**
 * \defgroup utilities Utilities
 * \ingroup utilities
 * @{
 */
/// \brief Wraps a lvalue or a rvalue in a copyable, assignable object.
template<class T>
struct forwarder : private detail_::forwarder_base<T>
{
  using value_type = typename detail_::forwarder_base<T>::value_type;
  using type = typename detail_::forwarder_base<T>::type;

  constexpr forwarder(value_type & x) noexcept : x_(x) {}

  constexpr type get() const { return static_cast<type>(x_); }
  constexpr value_type & ref() const { return x_; }
  constexpr value_type const & cref() const { return x_; }

  template<class... Args>
  constexpr decltype(auto) operator()(Args && ... args) const {
    return get()(std::forward<Args>(args)...);
  }

private:
  value_type & x_;
};
/** @} group utilities */

} // falcon


// TODO Falcon.Traits
namespace falcon {

/**
 * \defgroup metaprogramming Metaprogramming
 * \ingroup utilities
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
 * \defgroup tuple Tuple
 * \ingroup utilities
 * @{
 */
/// \brief Invoke the callable object f with the Ith elements from the tuple of arguments.
template<class TInt, std::size_t... I, class F, class Tuple>
constexpr decltype(auto) apply(std::integer_sequence<TInt, I...>, F && f, Tuple && t) {
  // TODO falcon::invoke
  return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

/// \brief Invoke the Callable object f with a tuple of arguments.
template<class F, class Tuple>
constexpr decltype(auto) apply(F && f, Tuple && t) {
  return apply(
    std::make_index_sequence<std::tuple_size<std::decay_t<Tuple>>::value>{}
  , std::forward<F>(f), std::forward<Tuple>(t)
  );
}
/** @} group utilities */

}


namespace falcon {

namespace ctmatch {

namespace detail_ {
  template<class Pred, class F = void> struct match_if;
  template<bool CheckMismatch, class R, class... Cs> struct match;

  template<class T> struct pmatch;

  template<class R, class T> void match_invoke(R*, T const &) {}

  template<class R, class T, class M, class... Ms>
  constexpr decltype(auto) match_invoke(R*, T x, M const & m, Ms const & ... ms);

  template<class T> std::false_type pmatch_invoke(T const &) { return {}; }

  template<class T, class Fn, class... Fns>
  decltype(auto) pmatch_invoke(T x, Fn fn, Fns... fns);

  template<class T> struct eq;
} // detail_


/**
 * \defgroup Matching Matching
 * \ingroup utilities
 * @{
 */

/**
 * \brief Call the first function that can be.
 * \return returns std::true_type if a function is invoked, otherwise std::false_type.
 */
template<class T, class... Fns>
auto pmatch_invoke(T && x, Fns && ... fns) {
  return detail_::pmatch_invoke(forwarder<T>{x}, forwarder<Fns>{fns}...);
}

/**
 * \brief Wraps pmatch_invoke() with operator| to add a function.
 *
 * \code
 * pmatch(x)
 * | [](int &) { ... }
 * | [](auto && x, std:enable_if_t< is_pointer< decltype(x) >::value >* = {}) { ... }
 * | ...
 * \endcode
 */
template<class T>
detail_::pmatch<forwarder<T>>
pmatch(T && x) {
  return {x};
}


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

/**
 * \brief Uses the first invokable case.
 *
 * \tparam ResultType can be unspecified_result_type, common_result_type or any type
 */
template<class ResultType = unspecified_result_type, class T, class... Cases>
constexpr decltype(auto) match_invoke(T && x, Cases const & ... cases) {
  return detail_::match_invoke(
    static_cast<ResultType*>(nullptr),
    forwarder<T>{x},
    cases...
  );
}


/**
 * \brief Wraps \c match_invoke with operator| to add a function.
 *
 * \code
 * x >>= match//.result<ResultType>() or common_result()
 * | match_if([](auto const & x) { return bool(x); }) >> []{ ... }
 * | match_if([](auto const & x) { return std::is_pointer< decltype(x)>{}; }) >> []{ ... }
 * | match_if(...) >> [](auto && x){ ... }
 * | match_value(0) >> func)
 * | match_value(0, func)
 * | [](only_integral_t) { ... }
 * | [](auto && any) { ... } // always true, the following are ignored
 * | match_always // always true, the following are ignored
 * | match_error // added by match (not with nmatch)
 * \endcode
 * \code
 * (match//.result<ResultType>() or common_result()
 * | cases | ...
 * )(x)
 * \endcode
 */
template<bool CheckMismatch>
struct match_fn
{
  template<class C>
  constexpr detail_::match<CheckMismatch, unspecified_result_type, std::decay_t<C>>
  operator|(C && c) const {
    return {std::forward<C>(c)};
  }

  template<class T>
  void operator()(T && x) const {
    std::conditional_t<CheckMismatch, match_error_fn, match_always_fn>{}(x);
  }

  /**
   * \brief Specifies the result type.
   *
   * \tparam ResultType can be unspecified_result_type, common_result_type or any type
   */
  template<class ResultType>
  constexpr detail_::match<CheckMismatch, ResultType>
  result() const {
    return {};
  }

  /// \brief The result type is the common type.
  constexpr detail_::match<CheckMismatch, common_result_type>
  common_result() const {
    return {};
  }
};


/// \brief Condtional match.
template<class Pred>
detail_::match_if<std::decay_t<Pred>>
constexpr match_if(Pred && pred) {
  return {std::forward<Pred>(pred)};
}

/// \brief Condtional match.
template<class Pred, class F>
detail_::match_if<std::decay_t<Pred>, std::decay_t<F>>
constexpr match_if(Pred && pred, F && f) {
  return {std::forward<Pred>(pred), std::forward<F>(f)};
}


/**
 * \brief Condtional match on value.
 */
template<class T>
constexpr auto match_value(T && x) {
  //return match_if(la::val(x) == la::arg1);
  return match_if(detail_::eq<std::decay_t<T>>{x});
}

/**
 * \brief Condtional match on value.
 */
template<class T, class F>
constexpr auto match_value(T && x, F && f) {
  return match_if(detail_::eq<std::decay_t<T>>{x}, std::forward<F>(f));
}


namespace {
  constexpr match_error_fn match_error = {};
  constexpr match_always_fn match_always = {};

  /// \brief Match with check mismatch.
  constexpr match_fn<1> match = {};

  /// \brief Match without check mismatch.
  constexpr match_fn<0> nmatch = {};
}


namespace ext {
  /**
   * \defgroup extension Extension
   * \ingroup extensions
   * @{
   */
  /// \brief A compile-time branchement
  template<class T, T x>
  std::integral_constant<bool, bool(x)>
  normalize_branch_value(std::integral_constant<T, x>) {
    return {};
  }

  /// \brief A runtime-time branchement
  inline bool normalize_branch_value(bool x) {
    return x;
  }
  /** @} */
}

/** @} group utilities */


//
// Implementation
//

namespace detail_ {

template<class T>
struct eq
{
  T x_;

  template<class U>
  auto operator()(U const & y) const
  noexcept(noexcept(x_ == y))
  -> decltype(x_ == y)
  {
    return x_ == y;
  }
};

template<class Pred, class F>
struct match_if
{
  Pred pred_;
  F fn_;
};

template<class Pred>
struct match_if<Pred, void>
{
  Pred pred_;

  template<class F>
  constexpr match_if<Pred, std::decay_t<F>> operator>>(F && f) {
    return {std::move(pred_), std::forward<F>(f)};
  }
};


template<class M, class T>
constexpr auto case_invoke(int, M const & mc, T x)
-> decltype(mc(x.get())) {
  return mc(x.get());
}

template<class Pred, class F, class T>
constexpr auto case_invoke(int, match_if<Pred, F> const & mc, T x)
-> decltype(mc.fn_(x.get())) {
  return mc.fn_(x.get());
}

template<class Pred, class F, class T>
constexpr auto case_invoke(char, match_if<Pred, F> const & mc, T)
-> decltype(mc.fn_()) {
  return mc.fn_();
}


struct match_m0_invoke_void
{
  template<class R, class T, class M, class... Ms>
  static void impl(
    R* r, bool b, T x, M const & m, Ms const & ... ms
  ) {
    if (b) {
      case_invoke(1, m, x);
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
  constexpr static Rs impl(
    R* r, bool b, T x, M const & m, Ms const & ... ms
  ) {
    if (b) {
      return case_invoke(1, m, x);
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
struct match_m0_invoke_impl<unspecified_result_type, Rs1, Rs2, void>
: match_m0_invoke_void
{};

template<class Rs1, class Rs2>
struct match_m0_invoke_impl<
  common_result_type, Rs1, Rs2,
  void_t<std::common_type_t<Rs1, Rs2>>
> : match_m0_invoke_with_result<std::common_type_t<Rs1, Rs2>>
{};

template<class Rs1, class Rs2>
struct match_m0_invoke_impl<void, Rs1, Rs2, void>
: match_m0_invoke_void
{};


template<class T, class M, class... Ms>
constexpr decltype(auto)
match_m0_invoke(
  unspecified_result_type*, std::true_type, T x, M const & m, Ms const & ...
) {
  return case_invoke(1, m, x);
}

template<class T, class M, class... Ms>
constexpr decltype(auto)
match_m0_invoke(
  common_result_type*, std::true_type, T x, M const & m, Ms const & ...
) {
  return case_invoke(1, m, x);
}

template<class R, class T, class M, class... Ms>
constexpr R match_m0_invoke(R*, std::true_type, T x, M const & m, Ms const & ...) {
  return case_invoke(1, m, x);
}

template<class R, class T, class M, class... Ms>
constexpr decltype(auto)
match_m0_invoke(R* r, std::false_type, T x, M const &, Ms const & ... ms) {
  return match_invoke(r, x, ms...);
}

template<class R, class T, class M, class... Ms>
constexpr decltype(auto)
match_m0_invoke(R* r, bool b, T x, M const & m, Ms const & ... ms) {
  return match_m0_invoke_impl<
    R,
    decltype(case_invoke(1, m, x)),
    decltype(match_invoke(r, x, ms...))
  >::impl(r, b, x, m, ms...);
}


template<class Pred, class F, class T>
auto is_invokable(int, match_if<Pred, F> const & mc, T x)
-> decltype(void(case_invoke(1, mc, x)), mc.pred_(x.cref())) {
  return mc.pred_(x.cref());
}

template<class Pred, class F, class T>
std::false_type is_invokable(char, match_if<Pred, F> const &, T) {
  return {};
}

template<class F, class T>
constexpr auto is_invokable(char, F const & fn, T x)
-> decltype(void(case_invoke(1, fn, x)), std::true_type{}) {
  (void)x; // GCC
  return {};
}

constexpr inline std::false_type is_invokable(...) {
  return {};
}

template<class R, class T, class M, class... Ms>
constexpr decltype(auto) match_invoke(R* r, T x, M const & m, Ms const & ... ms) {
  using ext::normalize_branch_value;
  return match_m0_invoke(r, normalize_branch_value(is_invokable(1, m, x)), x, m, ms...);
}


template<bool CheckMismatch, class R, class... Cs>
struct match
{
  template<class... C>
  constexpr match(C && ... c)
  : t(std::forward<C>(c)...)
  {}

  template<class C>
  constexpr match<CheckMismatch, R, Cs..., std::decay_t<C>> operator|(C && c) const & {
    return apply([&c](Cs const & ... cases) {
      return match<CheckMismatch, R, Cs..., std::decay_t<C>>{
        cases
#ifndef IN_IDE_PARSER
      ...
#endif
      , std::forward<C>(c)
      };
    }, t);
  }

#ifndef _MSC_VER
  template<class C>
  constexpr match<CheckMismatch, R, Cs..., std::decay_t<C>> operator|(C && c) && {
    return apply([&c](Cs & ... cases) {
      return match<CheckMismatch, R, Cs..., std::decay_t<C>>{
        std::move(cases)
#ifndef IN_IDE_PARSER
      ...
#endif
      , std::forward<C>(c)
      };
    }, t);
  }
#endif

  template<class T>
  constexpr decltype(auto) operator()(T && x) const {
    return apply([&x](Cs const & ... cases) {
      return match_invoke(
        static_cast<R*>(nullptr)
      , forwarder<T>{x}
      , cases
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
constexpr auto operator>>=(T && x, match<CheckMismatch, R, C...> const & m)
-> decltype(m(std::forward<T>(x))) {
  return m(std::forward<T>(x));
}


struct pmatch_nil
{
  template<class C>
  pmatch_nil operator|(C &&) && noexcept {
    return {};
  }

  std::true_type is_invoked() const noexcept {
    return {};
  }

private:
  template<class> friend class pmatch;

  pmatch_nil() = default;
};

template<class T>
struct pmatch
{
  pmatch(T x) noexcept
  : x(x)
  {}

  template<class C>
  auto operator|(C && c) && {
    return invokable_case(1, forwarder<C>{c});
  }

  std::false_type is_invoked() const noexcept {
    return {};
  }

private:
  pmatch(pmatch const &) = default;
  pmatch & operator = (pmatch const &) = delete;

  T x;

  template<class C>
  auto invokable_case(int, C c) const
  -> decltype(void(c.get()(x.get())), pmatch_nil{}) {
    c.get()(x.get());
    return {};
  }

  template<class C>
  pmatch invokable_case(char, C) const noexcept {
    return *this;
  }
};


template<class T, class F, class... Fs>
auto pmatch_invoke_case(int, T x, F fn, Fs...)
-> decltype(void(fn.get()(x.get())), std::true_type{}) {
  fn.get()(x.get());
  return {};
}

template<class T, class Fn, class... Fns>
auto pmatch_invoke_case(char, T x, Fn, Fns... fns) {
  return pmatch_invoke(x, fns...);
}

template<class T, class Fn, class... Fns>
decltype(auto) pmatch_invoke(T x, Fn fn, Fns... fns) {
  return pmatch_invoke_case(1, x, fn, fns...);
}

} // detail_

} }
