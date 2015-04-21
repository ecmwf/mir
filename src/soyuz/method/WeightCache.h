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

#ifndef soyuz_method_WeightCache_H
#define soyuz_method_WeightCache_H

#include "eckit/filesystem/PathName.h"
#include "eckit/memory/NonCopyable.h"

#include  "soyuz/method/MethodWeighted.h"


namespace mir {
namespace method {


class WeightCache : private eckit::NonCopyable {

  public: // methods

    /// @returns true if found cache
    static bool get(const std::string& key, MethodWeighted::Matrix& W);

    /// @returns true if addition was succcessful
    static bool add(const std::string& key, MethodWeighted::Matrix& W );

    static eckit::PathName filename(const std::string& key);

};


}  // namespace method
}  // namespace mir
#endif

