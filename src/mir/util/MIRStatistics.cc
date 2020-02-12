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


#include "mir/util/MIRStatistics.h"

#include "eckit/serialisation/Stream.h"


namespace mir {
namespace util {


MIRStatistics::MIRStatistics() = default;


MIRStatistics::MIRStatistics(eckit::Stream& s) :
    bitmapCache_(s),
    areaCroppingCache_(s),
    transHandleCache_(s),
    matrixCache_(s),
    meshCache_(s) {
    s >> cropTiming_;
    s >> frameTiming_;
    s >> globaliseTiming_;
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
    s >> saveTiming_;
    s >> gribEncodingTiming_;
}


void MIRStatistics::encode(eckit::Stream& s) const {
    s << bitmapCache_;
    s << areaCroppingCache_;
    s << transHandleCache_;
    s << matrixCache_;
    s << meshCache_;
    s << cropTiming_;
    s << frameTiming_;
    s << globaliseTiming_;
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
    s << saveTiming_;
    s << gribEncodingTiming_;
}


MIRStatistics& MIRStatistics::operator+=(const MIRStatistics& other) {
    bitmapCache_ += other.bitmapCache_;
    areaCroppingCache_ += other.areaCroppingCache_;
    transHandleCache_ += other.transHandleCache_;
    matrixCache_ += other.matrixCache_;
    meshCache_ += other.meshCache_;
    cropTiming_ += other.cropTiming_;
    frameTiming_ += other.frameTiming_;
    globaliseTiming_ += other.globaliseTiming_;
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
    saveTiming_ += other.saveTiming_;
    gribEncodingTiming_ += gribEncodingTiming_;
    return *this;
}


MIRStatistics& MIRStatistics::operator/=(size_t n) {
    bitmapCache_ /= n;
    areaCroppingCache_ /= n;
    transHandleCache_ /= n;
    matrixCache_ /= n;
    meshCache_ /= n;
    cropTiming_ /= n;
    frameTiming_ /= n;
    globaliseTiming_ /= n;
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
    saveTiming_ /= n;
    gribEncodingTiming_ /= n;
    return *this;
}


void MIRStatistics::report(std::ostream& out, const char* indent) const {

    bitmapCache_.report("Bitmap cache", out, indent);
    areaCroppingCache_.report("Area cache", out, indent);
    transHandleCache_.report("Trans cache", out, indent);
    matrixCache_.report("Matrix cache", out, indent);
    meshCache_.report("Mesh cache", out, indent);

    reportTime(out, "Time in grid to grid interp.", grid2gridTiming_, indent);
    reportTime(out, "Time in SH to grid transform", sh2gridTiming_, indent);
    reportTime(out, "Time loading/building coeff.", coefficientTiming_, indent);
    reportTime(out, "Time in VO/D to U/V", vod2uvTiming_, indent);

    reportTime(out, "Time in basic computations", calcTiming_, indent);
    reportTime(out, "Time in area-crop", cropTiming_, indent);
    reportTime(out, "Time in extracting frames", frameTiming_, indent);
    reportTime(out, "Time in extending to globe", globaliseTiming_, indent);

    reportTime(out, "Time applying bitmaps", bitmapTiming_, indent);
    reportTime(out, "Time compute matrices", computeMatrixTiming_, indent);
    reportTime(out, "Time matrix multiply", matrixTiming_, indent);
    reportTime(out, "Time creating coefficients", createCoeffTiming_, indent);
    reportTime(out, "Time loading coefficients", loadCoeffTiming_, indent);

    reportTime(out, "Time in GRIB encoding", gribEncodingTiming_, indent);

    reportTime(out, "Time saving", saveTiming_, indent);
}


void MIRStatistics::csvHeader(std::ostream& out) const {
    out << "grid2grid,sh2grid,coefficient,vod2uv,calc,crop,frame,globalise,bitmap,"
        << "computeMatrix,matrix,createCoeff,loadCoeff,gribEncoding,save";
}


void MIRStatistics::csvRow(std::ostream& out) const {
    out << grid2gridTiming_ << "," << sh2gridTiming_ << "," << coefficientTiming_ << "," << vod2uvTiming_ << ","
        << calcTiming_ << "," << cropTiming_ << "," << frameTiming_ << "," << globaliseTiming_ << "," << bitmapTiming_
        << "," << computeMatrixTiming_ << "," << matrixTiming_ << "," << createCoeffTiming_ << "," << loadCoeffTiming_
        << "," << gribEncodingTiming_ << "," << saveTiming_;
}


}  // namespace util
}  // namespace mir
