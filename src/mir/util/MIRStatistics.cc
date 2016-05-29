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
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace util {

//----------------------------------------------------------------------------------------------------------------------

MIRStatistics::MIRStatistics() {
}

MIRStatistics::MIRStatistics(eckit::Stream &s) {
    s >> cropTiming_;
    s >> coefficientTiming_;
    s >> sh2gridTiming_;
    s >> grid2gridTiming_;
    s >> vod2uvTiming_;
    s >> computeMatrixTiming_;
    s >> matrixTiming_;

}

void MIRStatistics::encode(eckit::Stream &s) const {
    s << cropTiming_;
    s << coefficientTiming_;
    s << sh2gridTiming_;
    s << grid2gridTiming_;
    s << vod2uvTiming_;
    s << computeMatrixTiming_;
    s << matrixTiming_;
}

MIRStatistics &MIRStatistics::operator+=(const MIRStatistics &rhs) {
    cropTiming_ += rhs.cropTiming_;
    coefficientTiming_ += rhs.coefficientTiming_;
    sh2gridTiming_ += rhs.sh2gridTiming_;
    grid2gridTiming_ += rhs.grid2gridTiming_;
    vod2uvTiming_ += rhs.vod2uvTiming_;
    computeMatrixTiming_ += rhs.computeMatrixTiming_;
    computeMatrixTiming_ += rhs.matrixTiming_;
    return *this;
}


void MIRStatistics::report(std::ostream &out, const char *indent) const {

    reportTime(out, "Time in grid to grid interpolations", grid2gridTiming_, indent);
    reportTime(out, "Time in SH to grid transform", sh2gridTiming_, indent);
    reportTime(out, "Time loading/building legendre coefficients", coefficientTiming_, indent);
    reportTime(out, "Time in VO/D to U/V", vod2uvTiming_, indent);

    reportTime(out, "Time in area-crop", cropTiming_, indent);
    reportTime(out, "Time compute matrices", computeMatrixTiming_, indent);
    reportTime(out, "Time matrix multiply", matrixTiming_, indent);


}
}

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

} // namespace pgen
