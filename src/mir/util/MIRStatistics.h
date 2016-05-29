/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   MIRStatistics.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef pgen_MIRStatistics_H
#define pgen_MIRStatistics_H

#include <iosfwd>

#include "mir/util/Statistics.h"
#include "eckit/log/Timer.h"

namespace eckit {
class Stream;
}

namespace mir {


//----------------------------------------------------------------------------------------------------------------------

class MIRStatistics : public Statistics {
  public:
    MIRStatistics() ;
    MIRStatistics(eckit::Stream &) ;

    size_t  batchCount_;

    size_t requirementsCount_;
    size_t failuresCount_;

    size_t filesCount_;

    size_t fieldsReadCount_;
    eckit::Length fieldsReadSize_;

    size_t winds_;

    size_t fieldsInterpolated_;
    size_t fieldsNotInterpolated_;

    size_t fieldsWrittenCount_;
    eckit::Length fieldsWrittenSize_;

    double fdbTime_;
    double mirTime_;
    double mpiTime_;
    double outputTime_;

    mutable eckit::Timer timer_;

    MIRStatistics &operator+=(const MIRStatistics &rhs) ;

    void report(std::ostream &out, const char *indent = "") const;

  private:

    void encode(eckit::Stream &) const;

    friend eckit::Stream &operator<<(eckit::Stream &s, const MIRStatistics &x) {
        x.encode(s);
        return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace pgen

#endif
