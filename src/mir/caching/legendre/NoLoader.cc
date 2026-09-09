// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/caching/legendre/NoLoader.h"

#include <ostream>


namespace mir::caching::legendre {


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


static const LegendreLoaderBuilder<NoLoader> loader("none");


}  // namespace mir::caching::legendre
