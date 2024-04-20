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


#include "mir/method/healpix/HEALPixConservative.h"

#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/repres/proxy/HEALPix.h"
// #include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::method::healpix {


bool HEALPixConservative::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const HEALPixConservative*>(&other);
    return (o != nullptr) && name() == o->name() && MethodWeighted::sameAs(*o);
}


void HEALPixConservative::assemble(util::MIRStatistics& /*unused*/, WeightMatrix& W, const repres::Representation& in,
                                   const repres::Representation& out) const {
    auto& log = Log::debug();
    log << "HEALPixConservative::assemble (input: " << in << ", output: " << out << ")" << std::endl;

    auto Nside_in  = dynamic_cast<const repres::proxy::HEALPix&>(in).Nside();
    auto Nside_out = dynamic_cast<const repres::proxy::HEALPix&>(out).Nside();

    // TODO
}


void HEALPixConservative::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    std::ostringstream str;
    print(str);
    md5.add(str.str());
}


void HEALPixConservative::json(eckit::JSON& j) const {
    j.startObject();
    MethodWeighted::json(j);
    j.endObject();
}


void HEALPixConservative::print(std::ostream& out) const {
    out << "HEALPixConservative[";
    MethodWeighted::print(out);
    out << "]";
}


bool HEALPixConservative::validateMatrixWeights() const {
    return false;
}


const char* HEALPixConservative::name() const {
    return "healpix-conservative";
}


static const MethodBuilder<HEALPixConservative> __builder("healpix-conservative");


}  // namespace mir::method::healpix
