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
/// @date Oct 2013

#ifndef mir_method_WeightCache_H
#define mir_method_WeightCache_H

#include <string>

#include "eckit/container/CacheManager.h"

#include "mir/method/WeightMatrix.h"

namespace atlas { class Grid; }

namespace mir {
namespace lsm { class LandSeaMask; }
namespace method {

class Method;

//----------------------------------------------------------------------------------------------------------------------

class WeightCache : public eckit::CacheManager {

 public:  // methods

    WeightCache();

    /// Tries to retrieve a cached WeightMatrix
    /// @returns true if found cache
    bool retrieve(const std::string &key, WeightMatrix &W) const;

    /// Inserts a cached WeightMatrix, overwritting any existing entry
    /// @returns true if insertion successful cache
    void insert(const std::string &key, const WeightMatrix &W) const;

    std::string generate_key(const Method &method,
                             const atlas::Grid &in,
                             const atlas::Grid &out,
                             const lsm::LandSeaMask &maskin,
                             const lsm::LandSeaMask &maskout) const;

 protected:

    virtual void print(std::ostream& s) const;

 private:

    virtual const char* version() const;
    virtual const char* extension() const;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif
