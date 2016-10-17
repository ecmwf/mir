/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @date May 2015

#ifndef mir_method_WeightCache_H
#define mir_method_WeightCache_H


#include "eckit/container/CacheManager.h"

namespace mir {

namespace method {
    class WeightMatrix;
}

namespace caching {

//----------------------------------------------------------------------------------------------------------------------


struct WeightCacheTraits {

    typedef method::WeightMatrix value_type;

    static const char* name() { return "mir/weights"; }
    static int version() { return 1; }
    static const char* extension() { return ".mat"; }

    static void save(const value_type& W, const eckit::PathName& path);

    static void load(value_type& W, const eckit::PathName& path);

};


class WeightCache : public eckit::CacheManager<WeightCacheTraits> {
public:  // methods
    explicit WeightCache();

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif
