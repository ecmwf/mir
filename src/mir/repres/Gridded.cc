/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/repres/Gridded.h"

#include "mir/util/Grib.h"


namespace mir {
namespace repres {


Gridded::Gridded() {}


Gridded::Gridded(const param::MIRParametrisation &parametrisation) {}


Gridded::~Gridded() {}

void Gridded::setComplexPacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_COMPLEX;
}

void Gridded::setSimplePacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_SIMPLE;
}

void Gridded::setSecondOrderPacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_SECOND_ORDER;
}

bool Gridded::globalDomain() const {
    std::ostringstream os;
    os << "Representation::globalDomain() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void Gridded::cropToDomain(const param::MIRParametrisation &parametrisation, data::MIRField &field) const {

    if (!globalDomain()) {
        Representation::cropToDomain(parametrisation, field); // This will throw an exception
    }
}

size_t Gridded::computeN(double first, double last, double inc, const char *n_name, const char *first_name, const char *last_name) {
    size_t n;
    if (!(first <= last)) {
        eckit::Log::info() << first_name << " (first):" << first << ", " << last_name << " (last)" << last << std::endl;
        ASSERT(first <= last);
    }
    ASSERT(inc > 0);
    size_t p = size_t((last - first) / inc);
    double d0 = fabs(last - (first + p * inc));
    double d1 = fabs(last - (first + (p + 1) * inc));

    // eckit::Log::info() << p << " " << d0 << " " << d1 << " " << inc << " " << first << " " << last << std::endl;
    ASSERT(d0 != d1);

    if (d0 < d1) {
        n = p;
    } else {
        n = p + 1;
    }

    if ((n * inc + first) != last) {
        eckit::Log::info() << "Gridded: cannot compute accuratly "
                           << n_name << ", given "
                           << first_name << "=" << first << ", "
                           << last_name << "=" << last << " and increment=" << inc << std::endl;
        eckit::Log::info() << "Last value is computed as " << (p * inc + first)
                           << ", diff=" << (last - (p * inc + first))
                           << std::endl;
    }

    return n + 1;

}
}  // namespace repres
}  // namespace mir

