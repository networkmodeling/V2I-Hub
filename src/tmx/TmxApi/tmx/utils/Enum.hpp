/*
 * Enum.hpp
 *
 *  Created on: Dec 4, 2018
 *      Author: gmb
 */

#ifndef TMX_UTILS_ENUM_HPP_
#define TMX_UTILS_ENUM_HPP_

#include <tmx/attributes/type_basics.hpp>
#include <tmx/TmxException.hpp>

#include <algorithm>
#include <array>
#include <map>
#include <memory.h>
#include <ostream>
#include <string>

#define UNKNOWN_NAME "????"

namespace tmx {

/**
 * Helper class for determining the name of the Enum.  Requires a C-style string called name.
 */
template <typename T, T V>
struct EnumName {
	static constexpr const char *name = UNKNOWN_NAME;
};

/**
 * Helper class for holding the enumeration sequence.
 */
template <typename T, T... S>
class EnumSequence: public std::array<T, sizeof...(S)> {
public:
	static constexpr const size_t size = sizeof...(S);

	typedef EnumSequence<T, S...> self;

	EnumSequence(bool initialize = true) { if (initialize) init<0>(*this); }
private:
	/**
	 * Write the current value of the sequence
	 */
	template <size_t Idx, T Val>
	void initValue() {
		(*this)[Idx] = Val;
	}

	/**
	 * Write the current value of the sequence, and then recurse over the rest of the sequence
	 */
	template <size_t Idx, T Val, T... Others>
	void init(const EnumSequence<T, Val, Others...> &seq) {
		static const EnumSequence<T, Others...> nextSeq(false);
		initValue<Idx, Val>();
		init<Idx+1>(nextSeq);
	}

	/**
	 * Write the last value of the sequence
	 */
	template <size_t Idx, T Val>
	void init(const EnumSequence<T, Val> &seq) {
		initValue<Idx, Val>();
	}
};

/**
 * Helper class for creating the enumeration sequence by enum type.
 */
template <typename T>
struct EnumSequenceBuilder {
	typedef EnumSequence<T> type;
};

/**
 * Enum helper class that map a enum value to its string name
 */
template <typename T, class Seq>
class EnumMap: public std::map<T, std::string> {
public:
	EnumMap(bool initialize = true) {
		if (initialize) {
			static const Seq _seq(false);
			init(_seq);
		}
	}
private:
	/**
	 * Write the enum value into the map, along with its name
	 */
	template <T Val>
	void initValue() {
		(*this)[Val] = EnumName<T, Val>::name;
	}

	/**
	 * Write the current value of the sequence into the map, and then recurse over the rest of the sequence
	 */
	template <T Val, T... Others>
	void init(const EnumSequence<T, Val, Others...> &seq) {
		static const EnumSequence<T, Others...> nextSeq(false);
		initValue<Val>();
		init(nextSeq);
	}

	/**
	 * Write the last value of the sequence into the map
	 */
	template <T Val>
	void init(const EnumSequence<T, Val> &seq) {
		initValue<Val>();
	}
};

/**
 * Base enum value class.
 */
template <typename T, typename Seq = typename EnumSequenceBuilder<T>::type>
class Enum {

public:
	static constexpr const size_t size = Seq::size;

	typedef Enum<T> self;
	typedef Seq seq;
	typedef T type;

	typedef EnumMap<T, Seq> byvalmap;
	typedef typename byvalmap::key_type byvalkey;
	typedef typename byvalmap::mapped_type byvalval;
	typedef typename std::map<byvalval, byvalkey> bynamemap;

	Enum(const char *name = NULL):
		_name(name ? name : UNKNOWN_NAME), _type(battelle::attributes::type_id_name<T>()) { validate(); }
	Enum(const std::string &name): Enum(name.empty() ? (const char *)NULL : name.c_str()) { }
	Enum(T value):
		_value(static_cast<int>(value)), _name(UNKNOWN_NAME), _type(battelle::attributes::type_id_name<T>()) { validate(); }
	Enum(const self &copy):
		_value(copy._value), _name(copy._name), _type(copy._type) { validate(); }

	// A generic constructor for auto-conversion from other types
	template <typename _T>
	Enum(_T value):
		_value(-1), _name(std::to_string(value)), _type(battelle::attributes::type_id_name<T>()) { validate(); }

	bool Valid() const {
		return _value >= 0;
	}

	T Value() const {
		if (!Valid()) {
			missing_value_exception ex(_name.c_str(), _type.c_str());
			BOOST_THROW_EXCEPTION(ex);
		}

		return static_cast<T>(_value);
	}

	std::string Name() const {
		if (!Valid()) {
			missing_value_exception ex(_name.c_str(), _type.c_str());
			BOOST_THROW_EXCEPTION(ex);
		}

		return _name;
	}

	static const std::array<self, size> &AllValues() {
		static const Seq _seq;
		static std::array<self, size> _array;

		if (_array.size() > 0 && !_array[0].Valid()) {
			for (size_t i = 0; i < _array.size(); i++)
				_array[i] = _seq[i];
		}

		return _array;
	}

	self &operator=(const T value) {
		_value = static_cast<int>(value);
		validate();
		return *this;
	}

	self &operator=(const std::string &name) {
		_value = -1;
		_name = name;
		validate();
		return *this;
	}

	self &operator=(const char *name) {
		return operator=(name ? std::string(name) : UNKNOWN_NAME);
	}

	template <typename _T>
	self &operator=(const _T value) {
		return operator=(std::to_string(value));
	}

	self &operator=(const self &copy) {
		_value = copy._value;
		_name = copy._name;
		validate();
		return *this;
	}

	bool operator==(const self &&other) const {
		return Valid() && other.Valid() && _value == other._value;
	}

	bool operator==(const self &other) const {
		return operator==(std::move(other));
	}

	bool operator!=(const self &&other) const {
		return !(operator==(other));
	}

	bool operator!=(const self &other) const {
		return !(operator==(other));
	}

	bool operator<(const self &&other) const {
		return Valid() && other.Valid() && _value < other._value;
	}

	bool operator<(const self &other) const {
		return operator<(std::move(other));
	}

	bool operator>(const self &&other) const {
		return Valid() && other.Valid() && _value > other._value;
	}

	bool operator>(const self &other) const {
		return operator>(std::move(other));
	}

	bool operator<=(const self &&other) const {
		return Valid() && other.Valid() && _value <= other._value;
	}

	bool operator<=(const self &other) const {
		return operator<=(std::move(other));
	}

	bool operator>=(const self &&other) const {
		return Valid() && other.Valid() && _value >= other._value;
	}

	bool operator>=(const self &other) const {
		return operator>=(std::move(other));
	}

	explicit operator bool() const {
		return static_cast<int>(Value());
	}

	T operator*() const { return Value(); }
	T operator()() const { return operator*(); }
	operator unsigned() const { return static_cast<unsigned>(operator*()); }
	operator int() const { return static_cast<int>(operator*()); }

	self &operator++() {
		if (Valid()) {
			if (_value == AllValues().back().Value())
				this->operator=(AllValues().front());
			else
				this->operator=(AllValues[_value + 1]);
		}

		return *this;
	}

	self &operator--() {
		if (Valid()) {
			if (_value == AllValues().front().Value())
				this->operator=(AllValues().back());
			else
				this->operator=(AllValues[_value - 1]);
		}

		return *this;
	}

	friend std::ostream &operator<<(std::ostream &os, const self &e) {
		os << e.Name();
		return os;
	}

	friend std::istream &operator>>(std::istream &is, self &e) {
		e._value = -1;
		is >> e._name;
		e.validate();
		return is;
	}

	static self ByValue(const T val) {
		return Enum(val);
	}

	static self ByName(const char *nm) {
		return Enum(nm);
	}

private:
	int _value { -1 };
	std::string _name;

	const std::string _type;

	void validate() {
		if (_name.empty()) _name = UNKNOWN_NAME;

		std::string lcName(_name);
#ifndef ENUM_CASE_SENSITIVE
		std::transform(lcName.begin(), lcName.end(), lcName.begin(), ::tolower);
#endif
		if (!Valid() && ByName().count(lcName) > 0)
			_value = static_cast<int>(ByName()[lcName]);

		if (Valid())
			_name = ByVal()[static_cast<T>(_value)];
	}

	static byvalmap &ByVal() {
		static byvalmap _map;
		return _map;
	}

	static bynamemap &ByName() {
		static bynamemap _map;

		if (_map.empty()) {
			const byvalmap &_byVal = ByVal();
			for (typename byvalmap::value_type pair: _byVal) {
				std::string lcName(pair.second);
#ifndef ENUM_CASE_SENSITIVE
				std::transform(lcName.begin(), lcName.end(), lcName.begin(), ::tolower);
#endif
				_map[lcName] = pair.first; 											// The actual name
				_map[std::to_string(static_cast<int>(pair.first))] = pair.first;	// The numeric as a string
			}
		}

		return _map;
	}

	struct missing_value_exception: tmx::TmxException {
		missing_value_exception(const char *val, const char *type):
					TmxException(err_message(val, type)) { }

		static std::string err_message(const char *val, const char *type) {
			std::string err("Unknown value: ");

			err += '\'';

			if (type) {
				err += type;
				err += "::";
			}

			err += val;
			err += '\'';

			return err;
		}
	};
};

/**
 * This class can be used to invoke a function that is templated for the enumeration type.
 *
 * For example, one functor can be used to create a object with the enum as a template parameter:
 *
 * struct MyClass;
 * template <MyEnum Val> struct MyClassImpl;
 *
 * template <MyEnum V> struct MyClassFunctor {
 *   MyClass *operator()() { return new MyClassImpl<V>(); }
 * }
 *
 * EnumDispatch<MyEnum, MyClassFunctor, MyClass *> dispatcher;
 * dispatcher.dispatch(3);	// Creates a MyClassImpl using enum 3 as template parameter
 */
template <typename T, template <T Val> class Functor, typename ReturnType = void>
class EnumDispatcher {
public:
	typedef Enum<T> enumtype;

	ReturnType dispatch(const enumtype &e) const {
		static const typename enumtype::seq _seq(false);
		return doDispatch(e.Value(), _seq);
	}

private:
	template <T Val>
	ReturnType invoke() const {
		static Functor<Val> functor;
		return functor();
	}

	template <T Val, T... Others>
	ReturnType doDispatch(const T value, const EnumSequence<T, Val, Others...> &seq) const {
		static const EnumSequence<T, Others...> _nextSeq(false);
		if (value == Val)
			return invoke<Val>();
		else
			return doDispatch(value, _nextSeq);
	}

	template <T Val>
	ReturnType doDispatch(const T value, const EnumSequence<T, Val> &seq) const {
		// This has to be the correct one, since all others have been exhausted
		return invoke<Val>();
	}
};
} /* End namespace tmx */

/**
 * These are some helpful macros that can assist in building basic enumerations.  SUpports up to 100 entries
 */
#define _Q(X) #X
#define _CAT(X, Y) X ## Y

#define _Q_V2(__1, __2) _Q(__1) _Q(__2)
#define _Q_V3(__1, more...) _Q(__1) _Q_V2(more)
#define _Q_V4(__1, more...) _Q(__1) _Q_V3(more)
#define _Q_V5(__1, more...) _Q(__1) _Q_V4(more)
#define _Q_V6(__1, more...) _Q(__1) _Q_V5(more)
#define _Q_V7(__1, more...) _Q(__1) _Q_V6(more)
#define _Q_V8(__1, more...) _Q(__1) _Q_V7(more)
#define _Q_V9(__1, more...) _Q(__1) _Q_V8(more)
#define _Q_V10(__1, more...) _Q(__1) _Q_V9(more)
#define _Q_V11(__1, more...) _Q(__1) _Q_V10(more)
#define _Q_V12(__1, more...) _Q(__1) _Q_V11(more)
#define _Q_V13(__1, more...) _Q(__1) _Q_V12(more)
#define _Q_V14(__1, more...) _Q(__1) _Q_V13(more)
#define _Q_V15(__1, more...) _Q(__1) _Q_V14(more)
#define _Q_V16(__1, more...) _Q(__1) _Q_V15(more)
#define _Q_V17(__1, more...) _Q(__1) _Q_V16(more)
#define _Q_V18(__1, more...) _Q(__1) _Q_V17(more)
#define _Q_V19(__1, more...) _Q(__1) _Q_V18(more)
#define _Q_V20(__1, more...) _Q(__1) _Q_V19(more)
#define _Q_V21(__1, more...) _Q(__1) _Q_V20(more)
#define _Q_V22(__1, more...) _Q(__1) _Q_V21(more)
#define _Q_V23(__1, more...) _Q(__1) _Q_V22(more)
#define _Q_V24(__1, more...) _Q(__1) _Q_V23(more)
#define _Q_V25(__1, more...) _Q(__1) _Q_V24(more)
#define _Q_V26(__1, more...) _Q(__1) _Q_V25(more)
#define _Q_V27(__1, more...) _Q(__1) _Q_V26(more)
#define _Q_V28(__1, more...) _Q(__1) _Q_V27(more)
#define _Q_V29(__1, more...) _Q(__1) _Q_V28(more)
#define _Q_V30(__1, more...) _Q(__1) _Q_V29(more)
#define _Q_V31(__1, more...) _Q(__1) _Q_V30(more)
#define _Q_V32(__1, more...) _Q(__1) _Q_V31(more)
#define _Q_V33(__1, more...) _Q(__1) _Q_V32(more)
#define _Q_V34(__1, more...) _Q(__1) _Q_V33(more)
#define _Q_V35(__1, more...) _Q(__1) _Q_V34(more)
#define _Q_V36(__1, more...) _Q(__1) _Q_V35(more)
#define _Q_V37(__1, more...) _Q(__1) _Q_V36(more)
#define _Q_V38(__1, more...) _Q(__1) _Q_V37(more)
#define _Q_V39(__1, more...) _Q(__1) _Q_V38(more)
#define _Q_V40(__1, more...) _Q(__1) _Q_V39(more)
#define _Q_V41(__1, more...) _Q(__1) _Q_V40(more)
#define _Q_V42(__1, more...) _Q(__1) _Q_V41(more)
#define _Q_V43(__1, more...) _Q(__1) _Q_V42(more)
#define _Q_V44(__1, more...) _Q(__1) _Q_V43(more)
#define _Q_V45(__1, more...) _Q(__1) _Q_V44(more)
#define _Q_V46(__1, more...) _Q(__1) _Q_V45(more)
#define _Q_V47(__1, more...) _Q(__1) _Q_V46(more)
#define _Q_V48(__1, more...) _Q(__1) _Q_V47(more)
#define _Q_V49(__1, more...) _Q(__1) _Q_V48(more)
#define _Q_V50(__1, more...) _Q(__1) _Q_V49(more)
#define _Q_V51(__1, more...) _Q(__1) _Q_V50(more)
#define _Q_V52(__1, more...) _Q(__1) _Q_V51(more)
#define _Q_V53(__1, more...) _Q(__1) _Q_V52(more)
#define _Q_V54(__1, more...) _Q(__1) _Q_V53(more)
#define _Q_V55(__1, more...) _Q(__1) _Q_V54(more)
#define _Q_V56(__1, more...) _Q(__1) _Q_V55(more)
#define _Q_V57(__1, more...) _Q(__1) _Q_V56(more)
#define _Q_V58(__1, more...) _Q(__1) _Q_V57(more)
#define _Q_V59(__1, more...) _Q(__1) _Q_V58(more)
#define _Q_V60(__1, more...) _Q(__1) _Q_V59(more)
#define _Q_V61(__1, more...) _Q(__1) _Q_V60(more)
#define _Q_V62(__1, more...) _Q(__1) _Q_V61(more)
#define _Q_V63(__1, more...) _Q(__1) _Q_V62(more)
#define _Q_V64(__1, more...) _Q(__1) _Q_V63(more)
#define _Q_V65(__1, more...) _Q(__1) _Q_V64(more)
#define _Q_V66(__1, more...) _Q(__1) _Q_V65(more)
#define _Q_V67(__1, more...) _Q(__1) _Q_V66(more)
#define _Q_V68(__1, more...) _Q(__1) _Q_V67(more)
#define _Q_V69(__1, more...) _Q(__1) _Q_V68(more)
#define _Q_V70(__1, more...) _Q(__1) _Q_V69(more)
#define _Q_V71(__1, more...) _Q(__1) _Q_V70(more)
#define _Q_V72(__1, more...) _Q(__1) _Q_V71(more)
#define _Q_V73(__1, more...) _Q(__1) _Q_V72(more)
#define _Q_V74(__1, more...) _Q(__1) _Q_V73(more)
#define _Q_V75(__1, more...) _Q(__1) _Q_V74(more)
#define _Q_V76(__1, more...) _Q(__1) _Q_V75(more)
#define _Q_V77(__1, more...) _Q(__1) _Q_V76(more)
#define _Q_V78(__1, more...) _Q(__1) _Q_V77(more)
#define _Q_V79(__1, more...) _Q(__1) _Q_V78(more)
#define _Q_V80(__1, more...) _Q(__1) _Q_V79(more)
#define _Q_V81(__1, more...) _Q(__1) _Q_V80(more)
#define _Q_V82(__1, more...) _Q(__1) _Q_V81(more)
#define _Q_V83(__1, more...) _Q(__1) _Q_V82(more)
#define _Q_V84(__1, more...) _Q(__1) _Q_V83(more)
#define _Q_V85(__1, more...) _Q(__1) _Q_V84(more)
#define _Q_V86(__1, more...) _Q(__1) _Q_V85(more)
#define _Q_V87(__1, more...) _Q(__1) _Q_V86(more)
#define _Q_V88(__1, more...) _Q(__1) _Q_V87(more)
#define _Q_V89(__1, more...) _Q(__1) _Q_V88(more)
#define _Q_V90(__1, more...) _Q(__1) _Q_V89(more)
#define _Q_V91(__1, more...) _Q(__1) _Q_V90(more)
#define _Q_V92(__1, more...) _Q(__1) _Q_V91(more)
#define _Q_V93(__1, more...) _Q(__1) _Q_V92(more)
#define _Q_V94(__1, more...) _Q(__1) _Q_V93(more)
#define _Q_V95(__1, more...) _Q(__1) _Q_V94(more)
#define _Q_V96(__1, more...) _Q(__1) _Q_V95(more)
#define _Q_V97(__1, more...) _Q(__1) _Q_V96(more)
#define _Q_V98(__1, more...) _Q(__1) _Q_V97(more)
#define _Q_V99(__1, more...) _Q(__1) _Q_V98(more)
#define _Q_V100(__1, more...) _Q(__1) _Q_V99(more)

#define _COUNT_ARGS_100(__1,__2,__3,__4,__5,__6,__7,__8,__9,__10,__11,__12,__13,__14,__15,__16,__17,__18,__19,__20,__21,__22,__23,__24,__25,__26,__27,__28,__29,__30,__31,__32,__33,__34,__35,__36,__37,__38,__39,__40,__41,__42,__43,__44,__45,__46,__47,__48,__49,__50,__51,__52,__53,__54,__55,__56,__57,__58,__59,__60,__61,__62,__63,__64,__65,__66,__67,__68,__69,__70,__71,__72,__73,__74,__75,__76,__77,__78,__79,__80,__81,__82,__83,__84,__85,__86,__87,__88,__89,__90,__91,__92,__93,__94,__95,__96,__97,__98,__99,__100,COUNT,...)  COUNT
#define COUNT_ARGS(args...) _COUNT_ARGS_100(args,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

#define _QUOTE_ALL(N, args...) _Q_V ## N(args)
#define _QUOTE_ALL_CALL(N, args...) _QUOTE_ALL(N, args)
#define QUOTE_ALL(args...) _QUOTE_ALL_CALL(COUNT_ARGS(args), args)

#define ENUM_BUILDER(__NS__, __NAME__, args...) \
/** Enumeration for __NAME___ */ \
enum class __NAME__ { args }; \
/** A list of all the names in the enumeration */ \
static constexpr const char *_CAT(__NAME__, _NAME_ARRAY)[] = { QUOTE_ALL(args) }; \
/** Template specialization for the names in the enumeration */ \
template <__NAME__ V> struct tmx::EnumName<__NAME__, V> { static constexpr const char *name = __NS__::_CAT(__NAME__, _NAME_ARRAY)[static_cast<size_t>(V)]; }; \
/** Template specialization for the enumeration sequence builder */ \
/** The enumeration class type */ \
typedef tmx::Enum<__NAME__> _CAT(__NAME__, Type);

// TEST:
//namespace example {
//ENUM_BUILDER(example, DeutschNumber, Null, Eins, Zwei, Drei, Vier, Funf, Sechs, Sieben, Acht, Neun, Zehn, Elf, Zwolf, Dreizehn, Vierzehn, Funfzehn, Sechzehn, Siebzehn, Achtzehn, Neunzehn, Zwanzig)
//}

#endif /* TMX_UTILS_ENUM_HPP_ */
