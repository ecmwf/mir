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


#include "mir/caching/legendre/NoLoader.h"

#include <ostream>


namespace mir {
namespace caching {
namespace legendre {


NoLoader::NoLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName& path) :
    LegendreLoader(parametrisation, path) {}

NoLoader::~NoLoader() = default;

void NoLoader::print(std::ostream& out) const {
    out << "NoLoader[]";
}

const void* NoLoader::address() const {
    return nullptr;
}

size_t NoLoader::size() const {
    return 0;
}

bool NoLoader::inSharedMemory() const {
    return false;
}

bool NoLoader::shared() {
    return false;
}


static LegendreLoaderBuilder<NoLoader> loader("none");


}  // namespace legendre
}  // namespace caching
}  // namespace mir
