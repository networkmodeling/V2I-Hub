/*
 * Measurement.h
 *
 *  Created on: Dec 24, 2018
 *      Author: gmb
 */

#ifndef INCLUDE_MEASUREMENT_H_
#define INCLUDE_MEASUREMENT_H_

#include "Units.h"
#include <atomic>
#include <ios>
#include <iostream>

namespace tmx {
namespace messages {

template <typename UnitType, UnitType Unit = static_cast<UnitType>(0), typename T = double>
class Measurement {
public:
	typedef Measurement<UnitType, Unit, T> self;
	typedef T type;

	static constexpr const UnitType unit = Unit;

	template <typename _T = T>
	Measurement(const _T value = 0): _value(static_cast<T>(value)) { }

	// Some of the data may be stored in atomic variables
	template <typename _T = T>
	Measurement(const std::atomic<_T> &value): Measurement((_T)value) { }

	Measurement(std::string value): Measurement() { if (!value.empty()) this->operator=(value); }
	Measurement(const char *value): Measurement(std::string(value ? value : "")) { }

	template <UnitType Other, typename _T = T>
	Measurement(const Measurement<UnitType, Other, _T>& copy): Measurement() { this->operator=(copy); }

	type get_value() const { return _value; }
	void set_value(const T value) { _value = value; }
	tmx::Enum<UnitType> get_unit() const { return unit; }

	type operator*() const { return get_value(); }
	type operator()() const { return operator*(); }

	operator T() const { return operator*(); }

	template <typename _T = T>
	self &operator=(const _T value) {
		this->set_value(static_cast<T>(value));
		return *this;
	}

	template <typename _T = T>
	self &operator=(const std::atomic<_T> &value) {
		return operator=((_T)value);
	}

	template <UnitType Other, typename _T = T>
	self &operator=(const Measurement<UnitType, Other, _T> &copy) {
		return operator=(get_value(copy));
	}

	self &operator=(std::string value) {
		if (!value.empty()) {
			std::stringstream ss(value);
			ss >> *this;
		}
		return *this;
	}

	// Arithmetic operator overloads.  Can work on any numeric type and any Measurement type

#define _OPER_OVERLOAD(FN, OP) template <typename _RHS = T> self FN(const _RHS rhs) { return _value OP rhs; } \
	template <UnitType Other, typename _T = T> self FN(const Measurement<UnitType, Other, _T> &rhs) { return FN(get_value(rhs)); } \
	self FN(const char *value) { return FN(self(value)); }

	_OPER_OVERLOAD(operator+, +)
	_OPER_OVERLOAD(operator-, -)
	_OPER_OVERLOAD(operator*, *)
	_OPER_OVERLOAD(operator/, /)
	_OPER_OVERLOAD(operator%, %)
	_OPER_OVERLOAD(operator&, &)
	_OPER_OVERLOAD(operator|, |)
	_OPER_OVERLOAD(operator^, ^)
	_OPER_OVERLOAD(operator>>, >>)
	_OPER_OVERLOAD(operator<<, <<)

#undef _OPER_OVERLOAD

#define _OPER_OVERLOAD(FN, OP) template <typename _RHS = T> self &FN(const _RHS rhs) { _value OP rhs; return *this; } \
	template <UnitType Other, typename _T = T> self &FN(const Measurement<UnitType, Other, _T> &rhs) { return FN(get_value(rhs)); } \
	self FN(const char *value) { return FN(self(value)); }

	_OPER_OVERLOAD(operator+=, +=)
	_OPER_OVERLOAD(operator-=, -=)
	_OPER_OVERLOAD(operator*=, *=)
	_OPER_OVERLOAD(operator/=, /=)
	_OPER_OVERLOAD(operator%=, %=)
	_OPER_OVERLOAD(operator&=, &=)
	_OPER_OVERLOAD(operator|=, |=)
	_OPER_OVERLOAD(operator^=, ^=)
	_OPER_OVERLOAD(operator>>=, >>=)
	_OPER_OVERLOAD(operator<<=, <<=)

#undef _OPER_OVERLOAD

#define _OPER_OVERLOAD(FN, OP) template <typename _RHS = T> bool FN(const _RHS rhs) { return _value OP rhs; } \
	template <UnitType Other, typename _T = T> bool FN(const Measurement<UnitType, Other, _T> &rhs) { return FN(get_value(rhs)); } \
	bool FN(const char *value) { return FN(self(value)); }

	_OPER_OVERLOAD(operator==, ==)
	_OPER_OVERLOAD(operator!=, !=)
	_OPER_OVERLOAD(operator>, >)
	_OPER_OVERLOAD(operator<, <)
	_OPER_OVERLOAD(operator>=, >=)
	_OPER_OVERLOAD(operator<=, <=)

#undef _OPER_OVERLOAD

	// Unary operators
	self &operator++() { _value++; return *this; }
	self &operator--() { _value--; return *this; }

	explicit operator bool() { return (_value != static_cast<T>(0)); }

	template <UnitType NewUnit, typename _T = T>
	Measurement<UnitType, NewUnit, T> as() {
		return Measurement<UnitType, NewUnit, _T>(*this);
	}

	friend std::ostream &operator<<(std::ostream &os, const self &m) {
		// Assumes any alphanumeric character requires a space between the number and the unit
		os << m.get_value() << (::isalnum(m.get_unit().Name()[0]) ? " " : "" ) << m.get_unit();
		return os;
	}

	friend std::istream &operator>>(std::istream &is, self &m) {
		std::string tmp;
		std::getline(is, tmp);

		char *unitPtr = NULL;
		double inVal = ::strtod(tmp.c_str(), &unitPtr);

		// The remaining portion must be the unit value
		std::string unitStr(unitPtr);

		while (unitStr.length() > 0 && ::isblank(unitStr[0]))
			unitStr.erase(0, 1);

		if (!unitStr.empty()) {
			Enum<UnitType> inUnit(unitStr.c_str());
			if (inUnit.Valid() && inUnit.Value() != unit) {
				Factory *f = Dispatcher().dispatch(inUnit.Value());
				m.set_value(static_cast<T>(f->convertFrom(inVal)));
			} else {
				m.set_value(inVal);
			}
		}

		return is;
	}

	/**
	 * Print a complete conversion table for this unit to the given output stream
	 */
	static void table(std::ostream &os, size_t colWidth = 12) {
		auto _units = Enum<UnitType>::AllValues();

		// Write the column headers
		os << std::setw(colWidth) << "Units";
		for (size_t i = 0; i < _units.size(); i++)
			os << std::setw(colWidth) << _units[i].Name();

		self _meas;
		for (size_t i = 0; i < _units.size(); i++) {
			os << std::endl;

			Factory *f = Dispatcher().dispatch(_units[i].Value());
			_meas.set_value(static_cast<T>(f->convertFrom(1.0)));

			for (size_t j = 0; j < _units.size(); j++) {
				if (j == 0) {
					// Write the row header
					os << std::setw(colWidth) << _units[i].Name();
				}

				f = Dispatcher().dispatch(_units[j].Value());
				os << std::setw(colWidth) << f->convertTo(_meas.get_value());
			}
		}

		os << std::endl;
	}
private:
	type _value;

	struct Factory {
		virtual ~Factory() { }
		virtual double convertTo(double currentVal) = 0;
		virtual double convertFrom(double currentVal) = 0;
	};

	template <UnitType Other>
	struct FactoryImpl: public Factory {
		Factory *operator()() { return this; }

		double convertTo(double currentVal) {
			return units::Convert<UnitType, Unit, Other>(currentVal);
		}

		double convertFrom(double currentVal) {
			return units::Convert<UnitType, Other, Unit>(currentVal);
		}
	};

	static tmx::EnumDispatcher<UnitType, FactoryImpl, Factory *> &Dispatcher() {
		static tmx::EnumDispatcher<UnitType, FactoryImpl, Factory *> _dispatcher;
		return _dispatcher;
	}

	template <UnitType Other, typename _T = T>
	double get_value(const Measurement<UnitType, Other, _T> copy) {
		double newVal = static_cast<double>(copy.get_value());

		// See if we need to convert
		if (this->get_unit() != copy.get_unit()) {
			newVal = units::Convert<UnitType, Other, Unit>(newVal);
		}

		return newVal;
	}
};


} /* End namespace messages */
} /* End namespace tmx */

#endif /* INCLUDE_MEASUREMENT_H_ */
