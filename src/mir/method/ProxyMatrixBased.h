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

#include "mir/method/MethodWeighted.h"
#include "mir/util/Atlas.h"


namespace mir {
namespace method {


class ProxyMatrixBased : public MethodWeighted {
public:
    // -- Types

    using atlas_config_t = atlas::util::Config;

    // -- Exceptions
    // None

    // -- Constructors
    // None

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Constructors

    ProxyMatrixBased(const param::MIRParametrisation&, std::string type);

    // -- Destructor
    // None

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    atlas_config_t options_;
    std::string name_;

    // -- Methods
    // None

    // -- Overridden methods

    // From Method
    int version() const override;

    // From MethodWeighted
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
    void hash(eckit::MD5&) const override;
    bool sameAs(const Method&) const override;
    void print(std::ostream&) const override;
    const char* name() const override { return name_.c_str(); }

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace method
}  // namespace mir
