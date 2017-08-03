/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_method_decompose_DecomposeToCartesian_h
#define mir_method_decompose_DecomposeToCartesian_h

#include <complex>
#include "mir/method/decompose/Decompose.h"


namespace mir {
namespace method {
namespace decompose {


class DecomposeToCartesian : public Decompose {
public:

    // -- Exceptions
    // None

    // -- Constructors

    DecomposeToCartesian(double missingValue=std::numeric_limits<double>::quiet_NaN()) : Decompose(missingValue) {}

    // -- Destructor

    virtual ~DecomposeToCartesian() {}

    // -- Convertors
    void a();

    // -- Operators
    // None

    // -- Methods

    virtual std::complex<double> decomposeValue(const double& angle) const = 0;


    virtual double recomposeValue(const std::complex<double>&) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


class DecomposeToCartesianChooser {
private:
    std::string name_;
    DecomposeToCartesian* choice_;
protected:
    DecomposeToCartesianChooser(const std::string&, DecomposeToCartesian* choice);
    virtual ~DecomposeToCartesianChooser();
public:
    static const DecomposeToCartesian& lookup(const std::string& name);
};


template<class T>
class DecomposeToCartesianChoice : public DecomposeToCartesianChooser {
public:
    DecomposeToCartesianChoice(const std::string& name) : DecomposeToCartesianChooser(name, new T) {
        static DecomposeChoice<T> decomposeChoice(name);
    }
};


}  // namespace decompose
}  // namespace method
}  // namespace mir


#endif

