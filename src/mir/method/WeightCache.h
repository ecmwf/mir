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
#include "eckit/filesystem/PathName.h"
#include "mir/method/WeightMatrix.h"

namespace mir {
namespace method {

class WeightCache : public eckit::CacheManager {
  public:  // methods

    WeightCache();

    /// Tries to retrieve a cached WeightMatrix
    /// @returns true if found cache
    bool retrieve(const std::string &key, WeightMatrix &W) const;

    /// Inserts a cached WeightMatrix, overwritting any existing entry
    /// @returns true if insertion successful cache
    void insert(const std::string &key, const WeightMatrix &W);


  private:

    /// @returns the path of the cache entry given the key
    virtual eckit::PathName entry(const key_t &key) const;

};

}  // namespace method
}  // namespace mir

#endif
