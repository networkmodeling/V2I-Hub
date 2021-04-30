/*
 * Units.h
 *
 *  Created on: Apr 17, 2017
 *      @author: gmb
 */

#ifndef INCLUDE_UNITS_H_
#define INCLUDE_UNITS_H_

#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#include <cmath>
#define __STRICT_ANSI__
#else
#include <cmath>
#endif

#if __cplusplus < 201103L
#error C++11 is minimally required for unit conversions with std::ratio.  Please use a different compiler!
#else
#include <ratio>
#endif

#include "UnitsEnumTypes.h"

namespace tmx {
namespace messages {
namespace units {

// Unit helper classes

/**
 * A class to hold a conversion ratio.
 *
 * This ratio must specifically be the number of nominal units exists per each of
 * the units.  The nominal unit is arbitrary, but should generally be the most
 * commonly used unit.
 *
 * So, for example, for Time units, the nominal unit may be seconds, so the
 * millisecond ratio would be the number of seconds per millisecond,
 * which is 1/1000, or std::milli.
 *
 * This template must be specialized for all units, unless the conversion is 1:1
 */
template <typename T, T Val>
struct UnitRatio { typedef std::ratio<1, 1> type; };

/**
 * A class which compounds a new ratio on top of an existing one in lieu of building
 * a direct conversion from the nominal unit.
 */
template <typename T, T Val, class Ratio>
struct UnitCompoundRatio {
	typedef typename std::ratio_multiply<typename UnitRatio<T, Val>::type, Ratio>::type type;
};

/**
 * An alias for un-boxing the ratio
 */
template <typename T, T Val>
using Ratio = typename UnitRatio<T, Val>::type;

/**
 * A class which calculates the conversion factor based on the ratio
 */
template <typename T, T Val>
struct UnitConversionFactor {
	static constexpr const double factor = 1.0 * Ratio<T, Val>::num / Ratio<T, Val>::den;
};

/**
 * An alias for un-boxing the conversion factor
 */
template <typename T, T Val>
using Factor = UnitConversionFactor<T, Val>;

/**
 * Function for converting between units of the same type using the unit ratios
 */
template <typename T, T From, T To>
static inline double Convert(double in) {
	static constexpr const double factor = Factor<T, From>::factor / Factor<T, To>::factor;
	return in * factor;
}

/**
 * Helper function that does a conversion the "long way" through the nominal base unit.
 * Some of the conversions require this since there is no direct multiplicative factor.
 */
template <typename T, T From, T To, T Base>
static inline double Convert(double in) {
	return Convert<T, Base, To>(Convert<T, From, Base>(in));
}

/**
 * UnitRatio template specializations
 *
 * These must be updated if new units are added
 */

// Some helper macros
#define UNIT_DECLR(A, B) template <> struct UnitRatio<A, A::B>
#define UNIT_SPEC1(A, B, C) UNIT_DECLR(A, B) { typedef C type; }
#define UNIT_SPEC2(A, B, C, D) UNIT_DECLR(A, B) { typedef std::ratio<C, D> type; }
#define UNIT_COMP1(A, B, C, D, E) UNIT_DECLR(A, B) { typedef typename UnitCompoundRatio<A, A::C, std::ratio<D, E> >::type type; }
#define UNIT_COPY1(A, B, C, D) UNIT_DECLR(A, B) { typedef typename UnitRatio<C, C::D>::type type; }
#define UNIT_CNVRT(A, B, C, D) template <> inline double Convert<A, A::B, A::C>(double in) { return Convert<A, A::B, A::C, A::D>(in); }
#define UNIT_CVRT1(A, B, C, D) UNIT_CNVRT(A, B, C, D); UNIT_CNVRT(A, C, B, D);

// Time Ratios.  Nominal unit is second (s)
UNIT_SPEC1(Time, ns, std::nano);
UNIT_SPEC1(Time, us, std::micro);
UNIT_SPEC1(Time, ms, std::milli);
UNIT_SPEC2(Time, min, 60, 1);
UNIT_COMP1(Time, hrs, min, 60, 1);
UNIT_COMP1(Time, days, hrs, 24, 1);

// Distance Ratios.  Nominal unit is meter (m)
UNIT_SPEC1(Distance, nm, std::nano);
UNIT_SPEC1(Distance, um, std::micro);
UNIT_SPEC1(Distance, mm, std::milli);
UNIT_SPEC1(Distance, cm, std::centi);
UNIT_SPEC1(Distance, km, std::kilo);
UNIT_SPEC2(Distance, in, 254, 10000);
UNIT_COMP1(Distance, ft, in, 12, 1);
UNIT_COMP1(Distance, yd, ft, 3, 1);
UNIT_COMP1(Distance, mi, ft, 5280, 1);

// Speed Ratios.  Nominal unit is mps (m/s)
UNIT_COPY1(Speed, ftpers, Distance, ft);
UNIT_DECLR(Speed, kph) { typedef typename std::ratio_divide<std::kilo, typename UnitRatio<Time, Time::hrs>::type>::type type; };
UNIT_DECLR(Speed, mph) { typedef typename std::ratio_divide<typename UnitRatio<Distance, Distance::mi>::type, typename UnitRatio<Time, Time::hrs>::type>::type type; };
UNIT_COMP1(Speed, knots, kph, 1852, 1000);

// Acceleration Ratios.  Nominal unit is meters per second per second (m/s/s or m/s^2)
UNIT_SPEC1(Acceleration, Gal, std::centi);
UNIT_COPY1(Acceleration, ftperspers, Distance, ft);
UNIT_SPEC2(Acceleration, g, 980665, 100000);
UNIT_DECLR(Acceleration, kmperhrperhr) { typedef typename std::ratio_divide<std::kilo, std::ratio_multiply<typename UnitRatio<Time, Time::hrs>::type, typename UnitRatio<Time, Time::hrs>::type> >::type type; };
UNIT_DECLR(Acceleration, miperhrperhr) { typedef typename std::ratio_divide<typename UnitRatio<Distance, Distance::mi>::type, std::ratio_multiply<typename UnitRatio<Time, Time::hrs>::type, typename UnitRatio<Time, Time::hrs>::type> >::type type; };

// Mass Ratios.  Nominal unit is kilogram (kg)
UNIT_SPEC1(Mass, g, std::milli);
UNIT_SPEC1(Mass, mg, std::micro);
UNIT_SPEC1(Mass, tonne, std::kilo);
UNIT_SPEC2(Mass, lb, 45359237, 100000000);
UNIT_COMP1(Mass, oz, lb, 1, 16);
UNIT_COMP1(Mass, ton, lb, 2000, 1);
UNIT_COMP1(Mass, st, lb, 14, 1);

// Force/Weight Ratios.  Nominal unit is Newton (N or kg*m/s^2)
UNIT_SPEC2(Force, dyn, 1, 100000);
UNIT_SPEC2(Force, kp, 980665, 100000);
UNIT_DECLR(Force, lbf) { typedef typename std::ratio_multiply<typename UnitRatio<Acceleration, Acceleration::g>::type, typename UnitRatio<Mass, Mass::lb>::type>::type type; };

// Energy Ratios.  Nominal unit is the Joule (J)
UNIT_SPEC1(Energy, kJ, std::kilo);
UNIT_SPEC2(Energy, erg, 1, 10000000);
UNIT_SPEC2(Energy, kWh, 3600000, 1);
UNIT_SPEC2(Energy, cal, 4184, 1000);
UNIT_SPEC2(Energy, BTU, 10543503, 10000);
UNIT_SPEC2(Energy, ftlb, 13558179483314004, 10000000000000000);
UNIT_SPEC2(Energy, eV, 16021766208, 10000000000);
UNIT_DECLR(Energy, kcal) { typedef typename std::ratio_multiply<std::kilo, typename UnitRatio<Energy, Energy::cal>::type>::type type; };
template <> struct UnitConversionFactor<Energy, Energy::eV> {
	static constexpr const double factor = 1e-19 * Ratio<Energy, Energy::eV>::num / Ratio<Energy, Energy::eV>::den;
};

// Power Ratios.  Nominal unit is the Watt (W)
UNIT_SPEC1(Power, mW, std::milli);
UNIT_SPEC1(Power, kW, std::kilo);
UNIT_SPEC1(Power, MegaW, std::mega);
UNIT_SPEC1(Power, GW, std::giga);
UNIT_SPEC2(Power, lbfpers, 1355817948, 1000000000);
UNIT_SPEC2(Power, dBm, 10, 1);
UNIT_COMP1(Power, hp, lbfpers, 550, 1);
UNIT_COPY1(Power, ergspers, Energy, erg);
UNIT_DECLR(Power, kcalperhr) { typedef typename std::ratio_divide<typename UnitRatio<Energy, Energy::kcal>::type, typename UnitRatio<Time, Time::hrs>::type>::type type; };
template <> inline double Convert<Power, Power::W, Power::dBm>(double in) {
	static constexpr const double factor = Factor<Power, Power::dBm>::factor;
	return ::log10(in) * factor + 30;
}

template <> inline double Convert<Power, Power::dBm, Power::W>(double in) {
	static constexpr const double factor = 1.0 / Factor<Power, Power::dBm>::factor;
	return ::pow(10, (in - 30) * factor);
}
UNIT_CVRT1(Power, dBm, mW, W)
UNIT_CVRT1(Power, dBm, kW, W)
UNIT_CVRT1(Power, dBm, MegaW, W)
UNIT_CVRT1(Power, dBm, GW, W)
UNIT_CVRT1(Power, dBm, lbfpers, W)
UNIT_CVRT1(Power, dBm, hp, W)
UNIT_CVRT1(Power, dBm, ergspers, W)
UNIT_CVRT1(Power, dBm, kcalperhr, W)


// Angle Ratios.  Nominal unit is degree (⁰)
UNIT_SPEC2(Angle, grad, 9, 10);
UNIT_SPEC2(Angle, pirad, 180, 1);
template <> struct UnitConversionFactor<Angle, Angle::rad> {
	static constexpr const double factor = Factor<Angle, Angle::pirad>::factor / M_PI;
};

// Frequency Ratios.  Nominal unit is Hertz (Hz or cycles/s)
UNIT_SPEC1(Frequency, kHz, std::kilo);
UNIT_SPEC1(Frequency, MHz, std::mega);
UNIT_SPEC1(Frequency, GHz, std::giga);
UNIT_DECLR(Frequency, rpm) { typedef typename std::ratio_divide<std::ratio<1, 1>, typename UnitRatio<Time, Time::min>::type>::type type; };
UNIT_SPEC2(Frequency, piradpers, 1, 2);
template <> struct UnitConversionFactor<Frequency, Frequency::radpers> {
	static constexpr const double factor = Factor<Frequency, Frequency::piradpers>::factor / M_PI;
};

// Temperature Ratios.  Nominal unit is degrees Celsius (⁰C)
UNIT_SPEC2(Temperature, F, 9, 5);
template <> inline double Convert<Temperature, Temperature::C, Temperature::F>(double in) {
	static constexpr const double factor = Factor<Temperature, Temperature::F>::factor;
	return (in * factor) + 32;
}

template <> inline double Convert<Temperature, Temperature::F, Temperature::C>(double in) {
	static constexpr const double factor = 1.0 / Factor<Temperature, Temperature::F>::factor;
	return (in - 32) * factor;
}

template <> inline double Convert<Temperature, Temperature::C, Temperature::K>(double in) {
	return in + 273.15;
}

template <> inline double Convert<Temperature, Temperature::K, Temperature::C>(double in) {
	return in - 273.15;
}
UNIT_CVRT1(Temperature, F, K, C)

// Percentage Ratios.  Nominal unit is the percent (%)
UNIT_SPEC1(Percent, decimal, std::centi);

// Area Ratios.  Nominal unit is square meters (m^2)
UNIT_DECLR(Area, sqcm) { typedef typename std::ratio_multiply<std::centi, std::centi>::type type; };
UNIT_DECLR(Area, sqkm) { typedef typename std::ratio_multiply<std::kilo, std::kilo>::type type; };
UNIT_DECLR(Area, sqin) { typedef typename std::ratio_multiply<typename UnitRatio<Distance, Distance::in>::type, typename UnitRatio<Distance, Distance::in>::type>::type type; };
UNIT_SPEC2(Area, ha, 10000, 1);
UNIT_COMP1(Area, sqft, sqin, 144, 1);
UNIT_COMP1(Area, sqyd, sqft, 9, 1);
UNIT_COMP1(Area, sqmi, sqft, 27878400, 1);
UNIT_COMP1(Area, acre, sqyd, 4840, 1);

// Volume Ratios.  Nominal unit is liter (l)
UNIT_SPEC1(Volume, ml, std::milli);
UNIT_SPEC1(Volume, cubicm, std::kilo);
UNIT_DECLR(Volume, cubicin) { typedef typename std::ratio_multiply<std::kilo, std::ratio_multiply<typename UnitRatio<Area, Area::sqin>::type, typename UnitRatio<Distance, Distance::in>::type>::type>::type type; };
UNIT_COMP1(Volume, cubicft, cubicin, 1728, 1);
UNIT_COMP1(Volume, cubicyd, cubicft, 27, 1);
UNIT_COMP1(Volume, tbsp, ml, 1478676478125, 100000000000);
UNIT_COMP1(Volume, tsp, tbsp, 1, 3);
UNIT_COMP1(Volume, floz, tbsp, 2, 1);
UNIT_COMP1(Volume, cup, floz, 8, 1);
UNIT_COMP1(Volume, pt, cup, 2, 1);
UNIT_COMP1(Volume, qt, pt, 2, 1);
UNIT_COMP1(Volume, gal, qt, 4, 1);

// Pressure Ratios.  Nominal unit is the Pascal (Pa or N/m^2)
UNIT_SPEC2(Pressure, bar, 100000, 1);
UNIT_SPEC2(Pressure, atm, 101325, 1);
UNIT_COMP1(Pressure, mmhg, atm, 1, 760);
UNIT_SPEC2(Pressure, psi, 44482216152605, 6451600000);

// Fuel Economy Ratios.  Nominal unit is kilometers per liter (km/l)
UNIT_DECLR(FuelEconomy, mpg) { typedef typename std::ratio_divide<typename UnitRatio<Distance, Distance::mi>::type, typename std::ratio_multiply<std::kilo, typename UnitRatio<Volume, Volume::gal>::type>::type> type; };

// Digital Ratios.  Nominal unit is the byte
UNIT_SPEC1(Digital, kB, std::kilo);
UNIT_SPEC1(Digital, MB, std::mega);
UNIT_SPEC1(Digital, GB, std::giga);
UNIT_SPEC1(Digital, TB, std::tera);
UNIT_SPEC1(Digital, PB, std::peta);
UNIT_SPEC2(Digital, bits, 1, 8);
UNIT_COMP1(Digital, kbit, kB, 1, 8);
UNIT_COMP1(Digital, Mbit, MB, 1, 8);
UNIT_COMP1(Digital, Gbit, GB, 1, 8);
UNIT_COMP1(Digital, Tbit, TB, 1, 8);
UNIT_COMP1(Digital, Pbit, PB, 1, 8);

// Bandwidth Ratios.  Nominal unit is mega bits per second (Mbps)
UNIT_SPEC1(Bandwidth, bps, std::micro);
UNIT_SPEC1(Bandwidth, kbps, std::milli);
UNIT_SPEC1(Bandwidth, Gbps, std::kilo);
UNIT_SPEC1(Bandwidth, Tbps, std::mega);
UNIT_SPEC1(Bandwidth, Pbps, std::giga);
UNIT_SPEC2(Bandwidth, Mbytespers, 8, 1);
UNIT_COMP1(Bandwidth, bytespers, bps, 8, 1);
UNIT_COMP1(Bandwidth, kbytespers, kbps, 8, 1);
UNIT_COMP1(Bandwidth, Gbytespers, Gbps, 8, 1);
UNIT_COMP1(Bandwidth, Tbytespers, Tbps, 8, 1);
UNIT_COMP1(Bandwidth, Pbytespers, Pbps, 8, 1);

} /* End namespace units */
} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_UNITS_H_ */
