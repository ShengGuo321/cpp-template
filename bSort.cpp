#include <tuple>
#include <type_traits>

template<int ...values>
struct Values {
	using type = std::tuple<std::integral_constant<int, values>...>;
};
template<bool, typename Then, typename Else> struct If;
template<typename Then, typename Else> struct If<true, Then, Else> {
	using type = Then;
};
template<typename Then, typename Else> struct If<false, Then, Else> {
	using type = Else;
};
template<typename T1> struct Less {
	template<typename T2> struct Apply {
		using type = typename If<(T1::value < T2::value), T1, T2>::type;
	};
};
template<typename T1> struct GE {
	template<typename T2> struct Apply {
		using type = typename If<(T1::value >= T2::value), T1, T2>::type;
	};
};

template<template <typename T1> class Comp, typename Tuples> struct FindLimit;
template<template <typename T1> class Comp, typename Head> 
struct FindLimit<Comp, std::tuple<Head>> {
	using type = Head;
};
template<template <typename T1> class Comp, typename Head, typename ...Tails>
struct FindLimit<Comp, std::tuple<Head, Tails...>> {
	using type = typename Comp<Head>::template Apply<typename FindLimit<Comp, std::tuple<Tails...>>::type>::type;
};

template <typename... Tuples> struct ConcatenateTuple {};
template <typename    Tuple0> struct ConcatenateTuple<Tuple0> {
	using type = Tuple0;
};
template <typename Tuple0, typename Tuple1>
struct ConcatenateTuple<Tuple0, Tuple1> {
	template <typename TupleA> struct ConcatenateImpl {};
	template <typename... TAs> struct ConcatenateImpl<std::tuple<TAs...> > {
		template <typename TupleB> struct Apply;
		template <typename... TBs> struct Apply<std::tuple<TBs...> > {
			using type = std::tuple<TAs..., TBs...>;
		};
	};
	using type = typename ConcatenateImpl<Tuple0>::template Apply<Tuple1>::type;
};
template <typename FirstTuple, typename... Follows> struct ConcatenateTuple<FirstTuple, Follows...> {
	using type = typename ConcatenateTuple<FirstTuple, typename ConcatenateTuple<Follows...>::type>::type;
};

template<typename Limit, typename ...Tuples> struct PopLimit{};
template<typename Limit>
struct PopLimit<Limit,std::tuple<>> {
	using type = std::tuple<>;
};
template<typename Limit, typename ...Others>
struct PopLimit<Limit, std::tuple<Limit, Others...>> {
	using type = std::tuple<Others...>;
};
template<typename Limit, typename Head, typename ...Others>
struct PopLimit<Limit, std::tuple<Head, Others...>> {
	using type = typename ConcatenateTuple<std::tuple<Head>,
		typename PopLimit<Limit, std::tuple<Others...>>::type>::type;
};

template<template <typename T1> class Comp, typename T> struct bSort;
template<template <typename T1> class Comp> struct bSort<Comp, std::tuple<>> {
	using type = std::tuple<>;
};
template<template <typename T1> class Comp,typename ...values>
struct bSort<Comp, std::tuple<values...>> {
	using limit =typename FindLimit<Comp, std::tuple<values...>>::type;
	using others =typename PopLimit<limit, std::tuple<values...>>::type;
	using type = typename ConcatenateTuple<std::tuple<limit>, typename bSort<Comp,others>::type>::type;
};

void staticTest() {
	using lst_1_3 = Values<1, 2, 3>::type;
	using lst_1_2 = Values<1, 2>::type;
	using lst_3_1 = Values<3, 2, 1>::type;
	using lst = Values<1, 7, 22, 6, 5, 3, 5>::type;
	using sorted_lst1 = Values<1, 3, 5, 5, 6, 7, 22>::type;
	using sorted_lst2 = Values<22, 7, 6, 5, 5, 3, 1>::type;

	using i1 = std::integral_constant<int, 1>::type;
	using i2 = std::integral_constant<int, 2>::type;
	using i3 = std::integral_constant<int, 3>::type;
		 
	static_assert(std::is_same<Less<i2>::Apply<i1>::type, i1>::value, "");
	static_assert(std::is_same<GE<i2>::Apply<i1>::type, i2>::value, "");

	static_assert(std::is_same<FindLimit<Less, std::tuple<i1>>::type, i1>::value, "");
	static_assert(std::is_same<FindLimit<Less, lst_1_3>::type, i1>::value, "");
	static_assert(std::is_same<FindLimit<GE, lst_1_3>::type, i3>::value, "");

	static_assert(std::is_same<ConcatenateTuple<std::tuple<i1>, std::tuple<>>::type, std::tuple<i1>>::value, "");

	static_assert(std::is_same<PopLimit<i3, lst_1_3>::type, lst_1_2>::value, "");
	static_assert(std::is_same<PopLimit<i1, Values<1>::type>::type, std::tuple<>>::value, "");

	static_assert(std::is_same<bSort<Less, lst>::type, sorted_lst1>::value, "");
	static_assert(std::is_same<bSort<GE, lst>::type, sorted_lst2>::value, "");
}
