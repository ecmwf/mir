/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/lsm/LandSeaMasks.h"

#include <ostream>

#include "eckit/utils/MD5.h"

#include "mir/lsm/Mask.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Trace.h"


namespace mir {
namespace lsm {


LandSeaMasks::LandSeaMasks(const Mask& input, const Mask& output) : input_(input), output_(output) {
    ASSERT(input_.active() == output_.active());
}


LandSeaMasks::LandSeaMasks(const LandSeaMasks&) = default;


void LandSeaMasks::print(std::ostream& out) const {
    out << "LandSeaMasks[in=" << input_ << ",output=" << output_ << "]";
}


void LandSeaMasks::hash(eckit::MD5& md5) const {
    md5 << input_ << output_;
}

std::string LandSeaMasks::cacheName() const {
    return input_.cacheName() + "+" + output_.cacheName();
}


LandSeaMasks LandSeaMasks::lookup(const param::MIRParametrisation& parametrisation, const repres::Representation& in,
                                  const repres::Representation& out) {
    trace::Timer("LandSeaMasks::lookup");
    return LandSeaMasks(Mask::lookupInput(parametrisation, in), Mask::lookupOutput(parametrisation, out));
}


bool LandSeaMasks::sameLandSeaMasks(const param::MIRParametrisation& parametrisation1,
                                    const param::MIRParametrisation& parametrisation2) {
    return Mask::sameInput(parametrisation1, parametrisation2) && Mask::sameOutput(parametrisation1, parametrisation2);
}


const std::vector<bool>& LandSeaMasks::inputMask() const {
    return input_.mask();
}


const std::vector<bool>& LandSeaMasks::outputMask() const {
    return output_.mask();
}


bool LandSeaMasks::cacheable() const {
    return input_.cacheable() && output_.cacheable();
}


bool LandSeaMasks::active() const {
    return input_.active() && output_.active();
}


}  // namespace lsm
}  // namespace mir
