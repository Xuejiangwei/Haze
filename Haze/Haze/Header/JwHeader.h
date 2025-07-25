#pragma once

template<typename T>
using Share = std::shared_ptr<T>;

template<typename T, typename... _Types>
Share<T> MakeShare(_Types&&... _Args)
{
	return std::make_shared<T>(std::forward<_Types>(_Args)...);
}

template<typename T>
using Unique = std::unique_ptr<T>;

template<typename T>
using SharedFromThis = std::enable_shared_from_this<T>;

template<typename T, typename... _Types>
Unique<T> MakeUnique(_Types&&... _Args)
{
	return std::make_unique<T>(std::forward<_Types>(_Args)...);
}

using String = std::string;

template<typename T, size_t SIZE>
using A_Array = std::array<T, SIZE>;

template<typename T>
using V_Array = std::vector<T>;

template<typename T>
using List = std::list<T>;

template<typename T>
using Set = std::set<T>;

template<class _Kty, class _Ty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>>
using HashMap = std::unordered_map<_Kty, _Ty, _Hasher, _Keyeq>;

template<class _Kty, class _Hasher = std::hash<_Kty>, class _Keyeq = std::equal_to<_Kty>>
using HashSet = std::unordered_set<_Kty, _Hasher, _Keyeq>;

template <class _Kty, class _Ty, class _Pr = std::less<_Kty>>
using LessRBTreeMap = std::map<_Kty, _Ty, _Pr>;

template <class _Kty, class _Ty, class _Pr = std::greater<_Kty>>
using GreaterRBTreeMap = std::map<_Kty, _Ty, _Pr>;

template <class _Kty, class _Pr = std::less<_Kty>>
using LessRBTreeSet = std::set<_Kty, _Pr>;

template <class _Kty, class _Pr = std::greater<_Kty>>
using GreaterRBTreeSet = std::set<_Kty, _Pr>;

template<class Ty1, class Ty2>
using Pair = std::pair<Ty1, Ty2>;

template<typename T>
using Func = std::function<T>;

namespace
{
	template <class _Ty>
	struct remove_reference {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty;
	};

	template <class _Ty>
	struct remove_reference<_Ty&> {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty&;
	};

	template <class _Ty>
	struct remove_reference<_Ty&&> {
		using type = _Ty;
		using _Const_thru_ref_type = const _Ty&&;
	};

	template <class _Ty>
	using remove_reference_t = typename remove_reference<_Ty>::type;

	template <class _Ty>
	constexpr remove_reference_t<_Ty>&& Move(_Ty&& _Arg) noexcept
	{
		return static_cast<remove_reference_t<_Ty>&&>(_Arg);
	}
}

template <class _Ty1, class _Ty2>
Share<_Ty1> DynamicCast(const Share<_Ty2>& other) noexcept
{
	const auto ptr = dynamic_cast<typename Share<_Ty1>::element_type*>(other.get());

	if (ptr)
	{
		return Share<_Ty1>(other, ptr);
	}

	return {};
}

template <class _Ty1, class _Ty2>
Share<_Ty1> DynamicCast(Share<_Ty2>&& other) noexcept
{
	const auto ptr = dynamic_cast<typename Share<_Ty1>::element_type*>(other.get());

	if (ptr)
	{
		return Share<_Ty1>(Move(other), ptr);
	}

	return {};
}

using x_BitArray = V_Array<bool>;

using x_HByte = char;
using x_HChar = wchar_t;

using x_uchar = unsigned char;
using x_uhbyte = unsigned char;

using x_int8 = char;
using x_uint8 = unsigned char;
using x_int16 = short;
using x_uint16 = unsigned short;
using x_int32 = int;
using x_uint32 = unsigned int;
using x_int64 = long long;
using x_uint64 = unsigned long long;

using x_float32 = float;
using x_float64 = double;
