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

class MeshCache : public eckit::CacheManager {
public:  // methods

    explicit MeshCache();

    /// Tries to retrieve a cached WeightMatrix
    /// @returns true if found cache
    bool retrieve(const std::string &key, atlas::mesh::Mesh &) const;

    /// Inserts a cached WeightMatrix, overwriting any existing entry
    /// @returns true if insertion successful cache
    void insert(const std::string &key, const atlas::mesh::Mesh &) const;

protected:

    virtual void print(std::ostream& s) const;

private:

    virtual const char* version() const;
    virtual const char* extension() const;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace caching
} // namespace mir

#endif
