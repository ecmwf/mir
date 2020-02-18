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


#ifndef mir_lsm_GribFileMaskFromMIR_h
#define mir_lsm_GribFileMaskFromMIR_h

#include "mir/lsm/GribFileMask.h"


namespace mir {
namespace lsm {


class GribFileMaskFromMIR : public GribFileMask {
public:
    GribFileMaskFromMIR(const std::string& name, const eckit::PathName& path,
                        const param::MIRParametrisation& parametrisation, const repres::Representation& representation,
                        const std::string& which);

private:
    std::string name_;

    virtual bool cacheable() const;
    virtual std::string cacheName() const;
};


}  // namespace lsm
}  // namespace mir


#endif
