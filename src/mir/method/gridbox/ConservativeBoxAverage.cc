/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/gridbox/ConservativeBoxAverage.h"

#include "eckit/log/Log.h"
#include "eckit/utils/MD5.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace method {
namespace gridbox {


ConservativeBoxAverage::~ConservativeBoxAverage() = default;


#if 0
bool ConservativeBoxAverage::sameAs(const Method& other) const {
    auto o = dynamic_cast<const ConservativeBoxAverage*>(&other);
    return o && GridBoxMethod::sameAs(*o);
}


void ConservativeBoxAverage::assemble(util::MIRStatistics& statistics, WeightMatrix& W,
                                      const repres::Representation& in, const repres::Representation& out) const {
    eckit::Channel& log = eckit::Log::debug<LibMir>();
    log << "ConservativeBoxAverage::assemble (input: " << in << ", output: " << out << ")" << std::endl;
}


const char* ConservativeBoxAverage::name() const {
    return "conservative-box-average";
}


void ConservativeBoxAverage::hash(eckit::MD5& md5) const {
    GridBoxMethod::hash(md5);
    md5.add(name());
}


void ConservativeBoxAverage::print(std::ostream& out) const {
    out << "ConservativeBoxAverage[";
    GridBoxMethod::print(out);
    out << "]";
}
#endif


static MethodBuilder<ConservativeBoxAverage> __builder("conservative-box-average");


}  // namespace gridbox
}  // namespace method
}  // namespace mir
