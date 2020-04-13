/*
 * Copyright 2019-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <unifex/detail/concept_macros.hpp>
#include <unifex/swap.hpp>

#include <functional>
#include <type_traits>

#if defined(_MSC_VER) && !defined(__clang__)
  #define UNIFEX_DIAGNOSTIC_PUSH __pragma(warning(push))
  #define UNIFEX_DIAGNOSTIC_POP __pragma(warning(pop))
  #define UNIFEX_DIAGNOSTIC_IGNORE_INIT_LIST_LIFETIME
  #define UNIFEX_DIAGNOSTIC_IGNORE_FLOAT_EQUAL
  #define UNIFEX_DIAGNOSTIC_IGNORE_CPP2A_COMPAT
#else // ^^^ defined(_MSC_VER) ^^^ / vvv !defined(_MSC_VER) vvv
  #if defined(__GNUC__) || defined(__clang__)
    #define UNIFEX_PRAGMA(X) _Pragma(#X)
    #define UNIFEX_DIAGNOSTIC_PUSH UNIFEX_PRAGMA(GCC diagnostic push)
    #define UNIFEX_DIAGNOSTIC_POP UNIFEX_PRAGMA(GCC diagnostic pop)
    #define UNIFEX_DIAGNOSTIC_IGNORE_PRAGMAS \
      UNIFEX_PRAGMA(GCC diagnostic ignored "-Wpragmas")
    #define UNIFEX_DIAGNOSTIC_IGNORE(X) \
      UNIFEX_DIAGNOSTIC_IGNORE_PRAGMAS \
      UNIFEX_PRAGMA(GCC diagnostic ignored "-Wunknown-pragmas") \
      UNIFEX_PRAGMA(GCC diagnostic ignored X)
    #define UNIFEX_DIAGNOSTIC_IGNORE_INIT_LIST_LIFETIME \
      UNIFEX_DIAGNOSTIC_IGNORE("-Wunknown-warning-option") \
      UNIFEX_DIAGNOSTIC_IGNORE("-Winit-list-lifetime")
    #define UNIFEX_DIAGNOSTIC_IGNORE_FLOAT_EQUAL UNIFEX_DIAGNOSTIC_IGNORE("-Wfloat-equal")
    #define UNIFEX_DIAGNOSTIC_IGNORE_CPP2A_COMPAT UNIFEX_DIAGNOSTIC_IGNORE("-Wc++2a-compat")
  #else
    #define UNIFEX_DIAGNOSTIC_PUSH
    #define UNIFEX_DIAGNOSTIC_POP
    #define UNIFEX_DIAGNOSTIC_IGNORE_INIT_LIST_LIFETIME
    #define UNIFEX_DIAGNOSTIC_IGNORE_FLOAT_EQUAL
    #define UNIFEX_DIAGNOSTIC_IGNORE_CPP2A_COMPAT
  #endif
#endif // MSVC/Generic configuration switch

namespace unifex {

  template <typename A, typename B>
  UNIFEX_CONCEPT
    same_as =
      UNIFEX_IS_SAME(A, B) &&
      UNIFEX_IS_SAME(B, A);

  template<typename From, typename To>
  UNIFEX_CONCEPT_FRAGMENT(
    _explicitly_convertible_to,
      requires(From(*from)()) //
      (
        static_cast<To>(from())
      ));
  template <typename From, typename To>
  UNIFEX_CONCEPT
    convertible_to =
      std::is_convertible_v<std::add_rvalue_reference_t<From>, To> &&
      UNIFEX_FRAGMENT(unifex::_explicitly_convertible_to, From, To);

  /// \cond
  namespace detail {
    template <typename T>
    using _cref_t = std::remove_reference_t<T> const &;

    template <class T>
    UNIFEX_CONCEPT_FRAGMENT(
      _boolean_testable_impl,
        requires(T && t)
        (
          requires(convertible_to<decltype(! (T &&) t), bool>)
        ));
    template <class T>
    UNIFEX_CONCEPT
      _boolean_testable =
        UNIFEX_FRAGMENT(_boolean_testable_impl, T) &&
        convertible_to<T, bool>;

    UNIFEX_DIAGNOSTIC_PUSH
    UNIFEX_DIAGNOSTIC_IGNORE_FLOAT_EQUAL

    template<typename T, typename U>
    UNIFEX_CONCEPT_FRAGMENT(
      _weakly_equality_comparable_with,
        requires(_cref_t<T> t, _cref_t<U> u) //
        (
          requires(_boolean_testable<decltype(t == u)>),
          requires(_boolean_testable<decltype(t != u)>),
          requires(_boolean_testable<decltype(u == t)>),
          requires(_boolean_testable<decltype(u != t)>)
        ));
    template<typename T, typename U>
    UNIFEX_CONCEPT
      weakly_equality_comparable_with_ =
        UNIFEX_FRAGMENT(detail::_weakly_equality_comparable_with, T, U);

    UNIFEX_DIAGNOSTIC_POP
  } // namespace detail
  /// \endcond

  template<typename T, typename U>
  UNIFEX_CONCEPT_FRAGMENT(
    _derived_from,
      requires()(0) &&
      convertible_to<T const volatile *, U const volatile *>);
  template<typename T, typename U>
  UNIFEX_CONCEPT
    derived_from =
      UNIFEX_IS_BASE_OF(U, T) &&
      UNIFEX_FRAGMENT(unifex::_derived_from, T, U);

  template<typename T, typename U>
  UNIFEX_CONCEPT_FRAGMENT(
    _assignable_from,
      requires(T t, U && u) //
      (
        t = (U &&) u,
        requires(same_as<T, decltype(t = (U &&) u)>)
      ));
  template<typename T, typename U>
  UNIFEX_CONCEPT
    assignable_from =
      std::is_lvalue_reference_v<T> &&
      UNIFEX_FRAGMENT(unifex::_assignable_from, T, U);

  template<typename T>
  UNIFEX_CONCEPT_FRAGMENT(
    _swappable,
      requires(T & t, T & u) //
      (
        unifex::swap(t, u)
      ));
  template<typename T>
  UNIFEX_CONCEPT
    swappable =
      UNIFEX_FRAGMENT(unifex::_swappable, T);

  template<typename T, typename U>
  UNIFEX_CONCEPT_FRAGMENT(
    _swappable_with,
      requires(T && t, U && u) //
      (
        unifex::swap((T &&) t, (T &&) t),
        unifex::swap((U &&) u, (U &&) u),
        unifex::swap((U &&) u, (T &&) t),
        unifex::swap((T &&) t, (U &&) u)
      ));
  template<typename T, typename U>
  UNIFEX_CONCEPT
    swappable_with =
      //common_reference_with<detail::_cref_t<T>, detail::_cref_t<U>> &&
      UNIFEX_FRAGMENT(unifex::_swappable_with, T, U);

  ////////////////////////////////////////////////////////////////////////////////////////////
  // Comparison concepts
  ////////////////////////////////////////////////////////////////////////////////////////////

  template<typename T>
  UNIFEX_CONCEPT
    equality_comparable =
      detail::weakly_equality_comparable_with_<T, T>;

  // template<typename T, typename U>
  // UNIFEX_CONCEPT_FRAGMENT(
  //   _equality_comparable_with,
  //     requires()(0) &&
  //     equality_comparable<
  //       common_reference_t<detail::_cref_t<T>, detail::_cref_t<U>>>);
  template<typename T, typename U>
  UNIFEX_CONCEPT
    equality_comparable_with =
      equality_comparable<T> &&
      equality_comparable<U> &&
      detail::weakly_equality_comparable_with_<T, U>;// &&
      //common_reference_with<detail::_cref_t<T>, detail::_cref_t<U>> &&
      //UNIFEX_FRAGMENT(unifex::_equality_comparable_with, T, U);

  template<typename T>
  UNIFEX_CONCEPT_FRAGMENT(
    _totally_ordered,
      requires(detail::_cref_t<T> t, detail::_cref_t<T> u) //
      (
        requires(detail::_boolean_testable<decltype(t < u)>),
        requires(detail::_boolean_testable<decltype(t > u)>),
        requires(detail::_boolean_testable<decltype(u <= t)>),
        requires(detail::_boolean_testable<decltype(u >= t)>)
      ));
  template<typename T>
  UNIFEX_CONCEPT
    totally_ordered =
      equality_comparable<T> &&
      UNIFEX_FRAGMENT(unifex::_totally_ordered, T);

  template<typename T, typename U>
  UNIFEX_CONCEPT_FRAGMENT(
    _totally_ordered_with,
      requires(detail::_cref_t<T> t, detail::_cref_t<U> u) //
      (
        requires(detail::_boolean_testable<decltype(t < u)>),
        requires(detail::_boolean_testable<decltype(t > u)>),
        requires(detail::_boolean_testable<decltype(t <= u)>),
        requires(detail::_boolean_testable<decltype(t >= u)>),
        requires(detail::_boolean_testable<decltype(u < t)>),
        requires(detail::_boolean_testable<decltype(u > t)>),
        requires(detail::_boolean_testable<decltype(u <= t)>),
        requires(detail::_boolean_testable<decltype(u >= t)>)
      ));
      //&& totally_ordered<
      //   common_reference_t<detail::_cref_t<T>, detail::_cref_t<U>>>
  template<typename T, typename U>
  UNIFEX_CONCEPT
    totally_ordered_with =
      totally_ordered<T> &&
      totally_ordered<U> &&
      equality_comparable_with<T, U> &&
      //common_reference_with<detail::_cref_t<T>, detail::_cref_t<U>> &&
      UNIFEX_FRAGMENT(unifex::_totally_ordered_with, T, U);

  ////////////////////////////////////////////////////////////////////////////////////////////
  // Object concepts
  ////////////////////////////////////////////////////////////////////////////////////////////

  template<typename T>
  UNIFEX_CONCEPT
    destructible =
      std::is_nothrow_destructible_v<T>;

  template<typename T, typename... Args>
  UNIFEX_CONCEPT
    constructible_from =
      destructible<T> &&
      UNIFEX_IS_CONSTRUCTIBLE(T, Args...);

  template<typename T>
  UNIFEX_CONCEPT
    default_constructible =
      constructible_from<T>;

  template<typename T>
  UNIFEX_CONCEPT
    move_constructible =
      constructible_from<T, T> &&
      convertible_to<T, T>;

  template<typename T>
  UNIFEX_CONCEPT_FRAGMENT(
    _copy_constructible,
      requires()(0) &&
      constructible_from<T, T &> &&
      constructible_from<T, T const &> &&
      constructible_from<T, T const> &&
      convertible_to<T &, T> &&
      convertible_to<T const &, T> &&
      convertible_to<T const, T>);
  template<typename T>
  UNIFEX_CONCEPT
    copy_constructible =
      move_constructible<T> &&
      UNIFEX_FRAGMENT(unifex::_copy_constructible, T);

  template<typename T>
  UNIFEX_CONCEPT_FRAGMENT(
    _move_assignable,
      requires()(0) &&
      assignable_from<T &, T>);
  template<typename T>
  UNIFEX_CONCEPT
    movable =
      std::is_object_v<T> &&
      move_constructible<T> &&
      UNIFEX_FRAGMENT(unifex::_move_assignable, T) &&
      swappable<T>;

  template<typename T>
  UNIFEX_CONCEPT_FRAGMENT(
    _copy_assignable,
      requires()(0) &&
      assignable_from<T &, T const &>);
  template<typename T>
  UNIFEX_CONCEPT
    copyable =
      copy_constructible<T> &&
      movable<T> &&
      UNIFEX_FRAGMENT(unifex::_copy_assignable, T);

  template<typename T>
  UNIFEX_CONCEPT
    semiregular =
      copyable<T> &&
      default_constructible<T>;
      // Axiom: copies are independent. See Fundamentals of Generic Programming
      // http://www.stepanovpapers.com/DeSt98.pdf

  template<typename T>
  UNIFEX_CONCEPT
    regular =
      semiregular<T> &&
      equality_comparable<T>;

  template<typename Fn, typename... As>
  UNIFEX_CONCEPT  //
    invocable = //
      std::is_invocable_v<Fn, As...>;

  namespace lazy {
    template<typename A, typename B>
    UNIFEX_CONCEPT_DEFER //
      same_as = //
        UNIFEX_DEFER(unifex::same_as, A, B);

    template<typename From, typename To>
    UNIFEX_CONCEPT_DEFER //
      convertible_to = //
        UNIFEX_DEFER(unifex::convertible_to, From, To);

    template<typename T, typename U>
    UNIFEX_CONCEPT_DEFER //
      derived_from = //
        UNIFEX_DEFER(unifex::derived_from, T, U);

    // template<typename T, typename U>
    // UNIFEX_CONCEPT_DEFER common_reference_with =
    //     UNIFEX_DEFER(unifex::common_reference_with, T, U);

    // template<typename T, typename U>
    // UNIFEX_CONCEPT_DEFER common_with =
    //     UNIFEX_DEFER(unifex::common_with, T, U);

    template<typename T, typename U>
    UNIFEX_CONCEPT_DEFER //
      assignable_from = //
        UNIFEX_DEFER(unifex::assignable_from, T, U);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      swappable = //
        UNIFEX_DEFER(unifex::swappable, T);

    template<typename T, typename U>
    UNIFEX_CONCEPT_DEFER //
      swappable_with = //
        UNIFEX_DEFER(unifex::swappable_with, T, U);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      equality_comparable = //
        UNIFEX_DEFER(unifex::equality_comparable, T);

    template<typename T, typename U>
    UNIFEX_CONCEPT_DEFER //
      equality_comparable_with = //
        UNIFEX_DEFER(unifex::equality_comparable_with, T, U);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      totally_ordered = //
        UNIFEX_DEFER(unifex::totally_ordered, T);

    template<typename T, typename U>
    UNIFEX_CONCEPT_DEFER //
      totally_ordered_with = //
        UNIFEX_DEFER(unifex::totally_ordered_with, T, U);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      destructible = //
        UNIFEX_DEFER(unifex::destructible, T);

    template<typename T, typename... Args>
    UNIFEX_CONCEPT_DEFER //
      constructible_from = //
        UNIFEX_DEFER_(unifex::constructible_from, UNIFEX_TYPE(T), Args...);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      default_constructible = //
        UNIFEX_DEFER(unifex::default_constructible, T);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      move_constructible = //
        UNIFEX_DEFER(unifex::move_constructible, T);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      copy_constructible = //
        UNIFEX_DEFER(unifex::copy_constructible, T);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      movable = //
        UNIFEX_DEFER(unifex::movable, T);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      copyable = //
        UNIFEX_DEFER(unifex::copyable, T);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      semiregular = //
        UNIFEX_DEFER(unifex::semiregular, T);

    template<typename T>
    UNIFEX_CONCEPT_DEFER //
      regular = //
        UNIFEX_DEFER(unifex::regular, T);

    template<typename Fn, typename... Args>
    UNIFEX_CONCEPT_DEFER //
      invocable = //
        UNIFEX_DEFER_(unifex::invocable, UNIFEX_TYPE(Fn), Args...);
  } // defer

} // namespace unifex