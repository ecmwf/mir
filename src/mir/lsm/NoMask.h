/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_lsm_NoMask_h
#define mir_lsm_NoMask_h

#include "mir/lsm/Mask.h"


namespace mir {
namespace lsm {


class NoMask : public Mask {

    bool active() const;

    bool cacheable() const;

    void hash(eckit::MD5&) const;

    const std::vector<bool>& mask() const;

    void print(std::ostream& out) const;

    std::string cacheName() const;

};


}  // namespace lsm
}  // namespace mir


#endif

