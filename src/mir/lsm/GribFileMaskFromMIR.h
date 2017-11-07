/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_lsm_GribFileMaskFromMIR_h
#define mir_lsm_GribFileMaskFromMIR_h

#include "mir/lsm/GribFileMask.h"


namespace mir {
namespace lsm {


class GribFileMaskFromMIR : public GribFileMask {
    bool cacheable() const;
public:
    using GribFileMask::GribFileMask;
};


}  // namespace lsm
}  // namespace mir


#endif

