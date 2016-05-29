/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/util/MIRStatistics.h"
#include "eckit/serialisation/Stream.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

MIRStatistics::MIRStatistics()
   {
}

MIRStatistics::MIRStatistics(eckit::Stream &s) {

}

void MIRStatistics::encode(eckit::Stream &s) const {

}

MIRStatistics &MIRStatistics::operator+=(const MIRStatistics &rhs) {
    return *this;
}


void MIRStatistics::report(std::ostream &out, const char *indent) const {


}
}

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

} // namespace pgen
