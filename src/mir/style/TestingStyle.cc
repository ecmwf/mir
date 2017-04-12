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

        std::cout << std::endl;
        map->print(std::cout);
        std::cout << std::endl;

        for (const size_t& T : std::vector<size_t>({
            2048, 2047, 2046,
            1280, 1279, 1278,
             800,  799,  798,
             640,  639,  638,
             512,  511,  510,
             400,  399,  398,
             320,  319,  318,
             256,  255,  254,
             214,  213,  212,
             192,  191,  190,
             160,  159,  158,
             107,  106,  105,
              96,   95,   94,
              64,   63 //,   62
            })) {
            size_t N = map->getPointsPerLatitudeFromTruncation(T);
            std::cout << "\tT=" << T
                      << "\tN=" << N
                      << "\tT=" << map->getTruncationFromPointsPerLatitude(N)
                      << std::endl;
        }
    }
    std::cout << "" << std::endl;
}


void mir::style::TestingStyle::print(std::ostream &) const {
}


}  // namespace <>
}  // namespace <>

