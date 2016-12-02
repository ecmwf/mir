/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_method_decompose_Decompose_h
#define mir_method_decompose_Decompose_h

#include <limits>
#include "mir/method/WeightMatrix.h"
#include "mir/util/Compare.h"


namespace mir {
namespace method {
namespace decompose {


class Decompose {
public:

    // -- Exceptions
    // None

    // -- Constructors

    Decompose(double missingValue=std::numeric_limits<double>::quiet_NaN());

    // -- Destructor

    virtual ~Decompose() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void setMissingValue(double missingValue=std::numeric_limits<double>::quiet_NaN()) const;

    virtual void decompose(const WeightMatrix::Matrix&, WeightMatrix::Matrix&) const = 0;

    virtual void recompose(const WeightMatrix::Matrix&, WeightMatrix::Matrix&) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    mutable util::compare::IsMissingFn isMissing_;

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


class DecomposeChooser {
private:
    std::string name_;
protected:
    DecomposeChooser(const std::string&, Decompose* choice);
    virtual ~DecomposeChooser();
public:
    static const Decompose& lookup(const std::string& name);
    static void list(std::ostream&);
};


template<class T>
class DecomposeChoice : public DecomposeChooser {
public:
    DecomposeChoice(const std::string& name) : DecomposeChooser(name, new T) {}
};


}  // namespace decompose
}  // namespace method
}  // namespace mir


#endif

