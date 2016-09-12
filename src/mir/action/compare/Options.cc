/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Aug 2016


#include "mir/action/compare/Options.h"

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace action {
namespace compare {


template<>
size_t Options::get(const std::string& name, const size_t& defaultValue) const {
    const long i = get< long >(name, long(defaultValue));
    ASSERT(i>=0);
    return size_t(i);
}


}  // namespace compare
}  // namespace action
}  // namespace mir

