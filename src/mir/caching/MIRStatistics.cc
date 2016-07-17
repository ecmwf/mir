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
    s >> frameTiming_;
    s >> bitmapTiming_;
    s >> coefficientTiming_;
    s >> sh2gridTiming_;
    s >> grid2gridTiming_;
    s >> vod2uvTiming_;
    s >> computeMatrixTiming_;
    s >> matrixTiming_;
    s >> loadCoeffTiming_;
    s >> createCoeffTiming_;
    s >> calcTiming_;

}

void MIRStatistics::encode(eckit::Stream &s) const {
    s << cropTiming_;
    s << frameTiming_;
    s << bitmapTiming_;
    s << coefficientTiming_;
    s << sh2gridTiming_;
    s << grid2gridTiming_;
    s << vod2uvTiming_;
    s << computeMatrixTiming_;
    s << matrixTiming_;
    s << loadCoeffTiming_;
    s << createCoeffTiming_;
    s << calcTiming_;
}

MIRStatistics &MIRStatistics::operator+=(const MIRStatistics &other) {
    cropTiming_ += other.cropTiming_;
    frameTiming_ += other.frameTiming_;
    bitmapTiming_ += other.bitmapTiming_;
    coefficientTiming_ += other.coefficientTiming_;
    sh2gridTiming_ += other.sh2gridTiming_;
    grid2gridTiming_ += other.grid2gridTiming_;
    vod2uvTiming_ += other.vod2uvTiming_;
    computeMatrixTiming_ += other.computeMatrixTiming_;
    matrixTiming_ += other.matrixTiming_;
    loadCoeffTiming_ += other.loadCoeffTiming_;
    createCoeffTiming_ += other.createCoeffTiming_;
    calcTiming_ += other.calcTiming_;
    return *this;
}


MIRStatistics &MIRStatistics::operator/=(size_t n) {
    cropTiming_ /= n;
    frameTiming_ /= n;
    bitmapTiming_ /= n;
    coefficientTiming_ /= n;
    sh2gridTiming_ /= n;
    grid2gridTiming_ /= n;
    vod2uvTiming_ /= n;
    computeMatrixTiming_ /= n;
    matrixTiming_ /= n;
    loadCoeffTiming_ /= n;
    createCoeffTiming_ /= n;
    calcTiming_ /= n;
    return *this;
}

void MIRStatistics::report(std::ostream &out, const char *indent) const {

    reportTime(out, "Time in grid to grid interp.", grid2gridTiming_, indent);
    reportTime(out, "Time in SH to grid transform", sh2gridTiming_, indent);
    reportTime(out, "Time loading/building coeff.", coefficientTiming_, indent);
    reportTime(out, "Time in VO/D to U/V", vod2uvTiming_, indent);

    reportTime(out, "Time in basic computations", calcTiming_, indent);
    reportTime(out, "Time in area-crop", cropTiming_, indent);
    reportTime(out, "Time in extracting frames", frameTiming_, indent);
    reportTime(out, "Time applying bitmaps", bitmapTiming_, indent);

    reportTime(out, "Time compute matrices", computeMatrixTiming_, indent);
    reportTime(out, "Time matrix multiply", matrixTiming_, indent);
    reportTime(out, "Time creating coefficients", createCoeffTiming_, indent);
    reportTime(out, "Time loading coefficients", loadCoeffTiming_, indent);


}
}

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

} // namespace pgen
