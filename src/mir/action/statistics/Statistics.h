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


#ifndef mir_action_statistics_Statistics_h
#define mir_action_statistics_Statistics_h

#include <iosfwd>
#include "eckit/exception/Exceptions.h"
#include "mir/action/compare/Options.h"
#include "mir/action/plan/Action.h"
#include "mir/data/MIRField.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Statistics.h"


namespace mir {
namespace context {
class Context;
}
}


namespace mir {
namespace action {
namespace statistics {


/**
 * @brief Calculate statistics on a MIRField
 */
class Statistics : public Action {
public:

    // -- Types

    /// Statistics results type
    typedef compare::Options StatisticsResults;

    // -- Exceptions
    // None

    // -- Constructors

    Statistics(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Statistics() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    void execute(context::Context&) const;

    bool sameAs(const Action&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members
    // None

    // -- Methods

    /// @return statistics results
    virtual bool calculate(const data::MIRField&, const param::MIRParametrisation&) const = 0;

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


class StatisticsFactory {
private:
    std::string name_;
    virtual Statistics* make(const param::MIRParametrisation&) = 0;
protected:
    StatisticsFactory(const std::string&);
    virtual ~StatisticsFactory();
public:
    static void list(std::ostream&);
    static Statistics* build(const std::string&, const param::MIRParametrisation&);
};


template<class T>
class StatisticsBuilder : public StatisticsFactory {
private:
    Statistics* make(const param::MIRParametrisation& param) {
        return new T(param);
    }
public:
    StatisticsBuilder(const std::string& name) : StatisticsFactory(name) {
        // register in the ActionFactory as well (Statistics is an Action)
        ActionBuilder<T> actionBuilder("statistics."+name);
    }
};


}  // namespace statistics
}  // namespace action
}  // namespace mir


#endif

