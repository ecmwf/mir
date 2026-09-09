// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
