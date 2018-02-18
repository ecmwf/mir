/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Aug 2016


#ifndef mir_action_compare_CompareValues_h
#define mir_action_compare_CompareValues_h

#include "mir/action/compare/Compare.h"


namespace mir {
namespace data {
class MIRField;
}
namespace param {
class MIRParametrisation;
}
}


namespace mir_cmp {


/**
 * @brief Compare MIRField's using their field values
 */
class CompareValues : public Compare {
public:

    // -- Types

    /// Compare values functor type
    typedef util::compare::ACompareBinFn< double > CompareValuesFn;

    // -- Exceptions
    // None

    // -- Constructors

    CompareValues(const param::MIRParametrisation& parametrisation) :
        Compare(parametrisation) {
        options_.setFrom<std::string>(parametrisation, "compare.mode",              "absolute");
        options_.setFrom<double>     (parametrisation, "compare.eps_packingfactor", 0);
        options_.setFrom<double>     (parametrisation, "compare.eps_absolute",      0);
        options_.setFrom<double>     (parametrisation, "compare.eps_angle",         0);
        options_.setFrom<double>     (parametrisation, "compare.eps_relative",      0);
    }

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    /// Comparison options
    // None

    // -- Methods

    /// @returns comparing functor, for relative/absolute comparison of values
    CompareValuesFn* getCompareFunctor(double missingValue1, double missingValue2, double packingError) const;

    /// @returns comparing angles functor according to comparison mode, absolute comparisons only
    CompareValuesFn* getCompareAnglesFunctor(double missingValue1, double missingValue2, double packingError) const;

    /// @returns comparison results on scalar quantity field values
    bool compareFieldsValuesScalar(const data::MIRField&, const data::MIRField&, const CompareOptions&, const double& packingError=0) const;

    /// @returns comparison results on angular quantity field values
    bool compareFieldsValuesAngleDegrees(const data::MIRField&, const data::MIRField&, const CompareOptions&, const double& packingError=0) const;

    /// @returns comparison results on vector quantity (cartesian 2D) field values
    bool compareFieldsValuesVectorCartesian2D(const data::MIRField&, const data::MIRField&, const CompareOptions&, const double& packingError=0) const;

    // -- Overridden methods

    /// @return if fields compare successfuly
    bool compare(
            const data::MIRField& field1, const param::MIRParametrisation& param1,
            const data::MIRField& field2, const param::MIRParametrisation& param2 ) const;

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


}  // namespace mir_cmp


#endif

