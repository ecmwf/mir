/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Oct 2016

#ifndef mir_method_LegendreCache_H
#define mir_method_LegendreCache_H


#include "eckit/container/CacheManager.h"

namespace mir {
namespace caching {

//----------------------------------------------------------------------------------------------------------------------

struct LegendreCacheTraits {

    static const char* name();
    static int version();
    static const char* extension();

    // Below are dummy types and functions
    typedef int value_type;

    static void save(const eckit::CacheManagerBase&, value_type&, const eckit::PathName&);
    static void load(const eckit::CacheManagerBase&, value_type&, const eckit::PathName&);
};

class LegendreCache : public eckit::CacheManager<LegendreCacheTraits> {
public:  // methods

    LegendreCache();
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif
