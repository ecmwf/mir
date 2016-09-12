/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Aug 2016


#ifndef mir_action_compare_Compare_h
#define mir_action_compare_Compare_h

#include <iosfwd>
#include "eckit/exception/Exceptions.h"
#include "mir/action/compare/Options.h"
#include "mir/action/plan/Action.h"
#include "mir/data/MIRField.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Compare.h"


namespace mir {
namespace context {
class Context;
}
namespace input {
class MIRInput;
}
}


namespace mir {
namespace action {
namespace compare {


// query of field represents a 2D cartesian vector (FIXME: find better solution)
bool field_is_vector_cartedian2d(const data::MIRField&);


// query of field represents an angle [°] (FIXME: find better solution)
bool field_is_angle_degrees(const data::MIRField&);


/**
 * @brief Compare action performs MIRField's comparisons
 */
class Compare : public Action {
public:

    // -- Types

    /// Compare options type
    typedef compare::Options CompareOptions;

    /// Compare results type
    typedef compare::Options CompareResults;

    // -- Exceptions
    // None

    // -- Constructors

    Compare(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Compare() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    /// @return maximum number of allowed differences to still return comparison success
    static size_t getNMaxDifferences(size_t N, const CompareOptions&);

    /// @returns comparison of field reports
    static bool compareResults(const CompareResults&, const CompareResults&, const CompareOptions&);

    /// Comparison options get value
    /// @returns requested option value (if option is not set returns defaultValue)
    template< typename T >
    T optionGet(const std::string& optionName, const T& defaultValue=T()) const {
        return options_.get<T>(optionName, defaultValue);
    }

    /// Comparison options set value
    template< typename T >
    void optionSet(const std::string& optionName, const T& optionValue) {
        options_.set(optionName, optionValue);
    }

    // -- Overridden methods

    void execute(context::Context&) const;

    bool sameAs(const Action&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    /// Comparison options
    CompareOptions options_;

    // -- Methods

    /// @return if fields compare successfuly
    virtual bool compare(
            const data::MIRField& field1, const param::MIRParametrisation& param1,
            const data::MIRField& field2, const param::MIRParametrisation& param2 ) const = 0;

    /// Comparison options reset
    void optionsReset();

    /// Comparison options set from another parametrisation
    virtual void optionsSetFrom(const param::MIRParametrisation&);

    // -- Overridden methods

    void print(std::ostream&) const;

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


class ComparisonFactory {
private:
    std::string name_;
    virtual Compare* make(const param::MIRParametrisation&) = 0;
protected:
    ComparisonFactory(const std::string&);
    virtual ~ComparisonFactory();
public:
    static void list(std::ostream&);
    static Compare* build(const std::string&, const param::MIRParametrisation&);
};


template<class T>
class ComparisonBuilder : public ComparisonFactory {
private:
    Compare* make(const param::MIRParametrisation& param) {
        return new T(param);
    }
public:
    ComparisonBuilder(const std::string& name) : ComparisonFactory(name) {
        // register in the ActionFactory as well (Compare is an Action) under a more descriptive name
        ActionBuilder<T> actionBuilder("compare." + name);
    }
};


}  // namespace compare
}  // namespace action
}  // namespace mir


#endif

