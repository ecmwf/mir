/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/style/TestingStyle.h"

#include <iostream>
#include <vector>
#include <string>
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/transform/mapping/Mapping.h"

namespace mir {
namespace style {


namespace {
static MIRStyleBuilder<TestingStyle> __style("testing");
}


TestingStyle::TestingStyle(const param::MIRParametrisation &parametrisation) :
    ECMWFStyle(parametrisation) {
}


TestingStyle::~TestingStyle() {
}


void TestingStyle::prepare(action::ActionPlan &) const {
    using namespace action::transform::mapping;

    for (auto m : {"table", "linear", "quadratic", "cubic"}) {
        eckit::ScopedPtr<Mapping> map(action::transform::mapping::MappingFactory::build(m, parametrisation_));
        ASSERT(map.get());

        std::cout << "" << std::endl;
        for (const size_t& T : std::vector<size_t>({2047, 1279, 799, 639, 511, 399, 319, 255, 213, 191, 159, 106, 95, 63})) {
            std::cout << "\"" << m << "\""
                      << "\tT=" << T
                      << "\tN=" << T << map->getPointsPerLatitudeFromTruncation(T) << std::endl;
        }
        std::cout << "" << std::endl;
    }
    std::cout << "" << std::endl;
}


void mir::style::TestingStyle::print(std::ostream &) const {
}


}  // namespace <>
}  // namespace <>

