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

//----------------------------------------------------------------------------------------------------------------------

MIRStatistics::MIRStatistics():
    batchCount_(0),
    requirementsCount_(0),
    failuresCount_(0),
    filesCount_(0),

    fieldsReadCount_(0),
    fieldsReadSize_(0),

    winds_(0),

    fieldsInterpolated_(0),
    fieldsNotInterpolated_(0),

    fieldsWrittenCount_(0),
    fieldsWrittenSize_(0),

    fdbTime_(0),
    mirTime_(0),
    mpiTime_(0),
    outputTime_(0) {
}

MIRStatistics::MIRStatistics(eckit::Stream &s) {
    s >> batchCount_;
    s >> requirementsCount_;
    s >> failuresCount_;
    s >> filesCount_;

    s >> fieldsReadCount_;
    s >> fieldsReadSize_;

    s >> winds_;

    s >> fieldsInterpolated_;
    s >> fieldsNotInterpolated_;

    s >> fdbTime_;
    s >> mirTime_;

    s >> mpiTime_;
    s >> outputTime_;
}

void MIRStatistics::encode(eckit::Stream &s) const {
    s << batchCount_;
    s << requirementsCount_;
    s << failuresCount_;
    s << filesCount_;

    s << fieldsReadCount_;
    s << fieldsReadSize_;

    s << winds_;

    s << fieldsInterpolated_;
    s << fieldsNotInterpolated_;

    s << fieldsWrittenCount_;
    s << fieldsWrittenSize_;

    s << fdbTime_;
    s << mirTime_;

    s << mpiTime_;
    s << outputTime_;
}

MIRStatistics &MIRStatistics::operator+=(const MIRStatistics &rhs) {
    batchCount_ += rhs.batchCount_;
    requirementsCount_ += rhs.requirementsCount_;
    failuresCount_ +=  rhs.failuresCount_;
    filesCount_ += rhs.filesCount_;
    fieldsReadCount_ += rhs.fieldsReadCount_;
    fieldsReadSize_ += rhs.fieldsReadSize_;
    winds_ += rhs.winds_;

    fieldsInterpolated_ += rhs.fieldsInterpolated_;
    fieldsNotInterpolated_ += rhs.fieldsNotInterpolated_;

    fieldsWrittenCount_ += rhs.fieldsWrittenCount_;
    fieldsWrittenSize_ += rhs.fieldsWrittenSize_;

    fdbTime_ += fdbTime_;
    mirTime_ += mirTime_;
    mpiTime_ += mpiTime_;
    outputTime_ += outputTime_;

    return *this;
}


void MIRStatistics::report(std::ostream &out, const char *indent) const {
    reportCount(out, "Requirement batches", batchCount_, indent);

    reportCount(out, "Total requirements", requirementsCount_, indent);
    reportCount(out, "Total failures", failuresCount_, indent);
    reportCount(out, "Number of target files opened", filesCount_, indent);

    reportCount(out, "Total fields read", fieldsReadCount_, indent);
    reportBytes(out, "Size fields read", fieldsReadSize_, indent);
    reportCount(out, "UV convertions", winds_, indent);

    reportCount(out, "Fields interpolated", fieldsInterpolated_, indent);
    reportCount(out, "Fields not interpolated", fieldsNotInterpolated_, indent);

    reportCount(out, "Total fields written", fieldsWrittenCount_, indent);
    reportBytes(out, "Size fields written", fieldsWrittenSize_, indent);

    reportTime(out, "Time in FDB", fdbTime_, indent);
    reportTime(out, "Time in MIR", mirTime_, indent);
    reportTime(out, "Time in MPI", mpiTime_, indent);
    reportTime(out, "Time in writting", outputTime_, indent);

}

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

} // namespace pgen
