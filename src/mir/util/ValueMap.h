/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#pragma once

#include "eckit/value/Value.h"


namespace eckit {
class Configured;
}  // namespace eckit

namespace mir::param {
class SimpleParametrisation;
}  // namespace mir::param


namespace mir::util {


class ValueMap : public eckit::ValueMap {
public:
    using eckit::ValueMap::ValueMap;
    using eckit::ValueMap::operator=;

    explicit ValueMap(const eckit::Value&);

    void set(eckit::Configured&) const;
    void set(param::SimpleParametrisation&) const;
};


}  // namespace mir::util
