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

#include <string>

#include "eckit/container/CacheManager.h"

#include "mir/method/WeightMatrix.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace caching {

//----------------------------------------------------------------------------------------------------------------------
struct CroppingCacheEntry {

    ~CroppingCacheEntry();

    std::vector<size_t> mapping_;
    util::BoundingBox bbox_;
    void print(std::ostream& s) const;
    friend std::ostream& operator<<(std::ostream& out, const CroppingCacheEntry& e) { e.print(out); return out; }

    size_t footprint() const;

    void save(const eckit::PathName& path) const;
    void load(const eckit::PathName& path);


};

struct CroppingCacheTraits {

    typedef CroppingCacheEntry value_type;

    static const char* name() { return "mir/cropping"; }
    static int version() { return 1; }
    static const char* extension() { return ".area"; }

    static void save(const value_type& c, const eckit::PathName& path) {
        c.save(path);
    }

    static void load(value_type& c, const eckit::PathName& path) {
        c.load(path);
    }
};


class CroppingCache : public eckit::CacheManager<CroppingCacheTraits> {
public:  // methods
    explicit CroppingCache();



};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif
