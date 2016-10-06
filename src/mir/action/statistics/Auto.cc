/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Oct 2016


#include "mir/action/statistics/Auto.h"

#include "eckit/memory/ScopedPtr.h"


namespace mir {
namespace action {
namespace statistics {


Auto::Auto(const param::MIRParametrisation& parametrisation) :
    Statistics(parametrisation) {
}


bool Auto::sameAs(const action::Action& other) const {
    const Auto* o = dynamic_cast<const Auto*>(&other);
    return o;
}


void Auto::calculate(const data::MIRField& field, Results& results) const {
    results.reset();

    std::string stats;
    parametrisation_.get("statistics", stats);
    if (stats == "auto" || stats == "") {
        throw eckit::UserError("Cannot determine a suitable statistics method", Here());
    }

    eckit::ScopedPtr<const Statistics> statistics(StatisticsFactory::build(stats, parametrisation_));
    statistics->calculate(field, results);
}


namespace {
static StatisticsBuilder<Auto> __auto1("auto");
static StatisticsBuilder<Auto> __auto2("");
}


}  // namespace statistics
}  // namespace action
}  // namespace mir

