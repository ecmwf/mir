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


#ifndef mir_action_compare_CompareStatistics_h
#define mir_action_compare_CompareStatistics_h

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
 * @brief Compare MIRField's using the statistics of their field values
 */
class CompareStatistics : public Compare {
public:

    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    CompareStatistics(const param::MIRParametrisation& parametrisation) :
        Compare(parametrisation) {
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

    /// @returns report on field statistics (scalar quantity)
    CompareResults getFieldStatisticsScalar(const data::MIRField&) const;

    /// @returns report on field statistics (angular quantity)
    CompareResults getFieldStatisticsAngleDegrees(const data::MIRField&) const;


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

