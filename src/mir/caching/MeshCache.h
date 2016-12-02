/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @date   May 2016

#ifndef atlas_MeshCache_h
#define atlas_MeshCache_h

#include "eckit/container/CacheManager.h"


namespace atlas {
namespace mesh {
class Mesh;
}
}

namespace mir {
namespace caching {

//----------------------------------------------------------------------------------------------------------------------

struct MeshCacheTraits {
    static const char* name();
    static int version();
    static const char* extension();

    // Below are dummy types and functions
    typedef int value_type;

    static void save(value_type&, const eckit::PathName& path);
    static void load(value_type&, const eckit::PathName& path);
};

class MeshCache : public eckit::CacheManager<MeshCacheTraits> {
public:  // methods

    explicit MeshCache();

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace caching
} // namespace mir

#endif
