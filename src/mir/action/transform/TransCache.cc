/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Jan 2017


#include "mir/action/transform/TransCache.h"

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace action {
namespace transform {


TransCache::TransCache() :
    inited_(false),
    loader_(0) {
#ifdef ATLAS_HAVE_TRANS
#else
    throw eckit::SeriousBug("Spherical harmonics transforms are not supported. "
                            "Please recompile ATLAS with TRANS support enabled.");
#endif
}


TransCache::~TransCache() {
#ifdef ATLAS_HAVE_TRANS
        if (inited_) {
            eckit::Log::info() << "Delete " << *this << std::endl;
            trans_delete(&trans_);
        }
        else {
            eckit::Log::info() << "Not Deleting " << *this << std::endl;
        }
        delete loader_;
#else
    throw eckit::SeriousBug("Spherical harmonics transforms are not supported. "
                            "Please recompile ATLAS with TRANS support enabled.");
#endif
}


void TransCache::print(std::ostream& s) const {
    s << "TransCache[";
    if (loader_) s << *loader_;
    s << "]";
}


}  // namespace transform
}  // namespace action
}  // namespace mir
