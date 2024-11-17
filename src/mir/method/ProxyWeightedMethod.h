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


namespace mir::method {


class ProxyWeightedMethod : public MethodWeighted {
protected:
    // -- Constructor

    ProxyWeightedMethod(const param::MIRParametrisation&, const std::string& interpolation_type);

    // -- Destructor

    ~ProxyWeightedMethod() override = default;

protected:
    // -- Methods

    void foldSourceHalo(const atlas::Interpolation&, size_t Nr, size_t Nc, WeightMatrix&) const;

private:
    // -- Members

    const std::string type_;
    atlas::util::Config interpol_;

    // -- Overridden methods

    // From MethodWeighted
    const char* name() const override;
    int version() const override;

    void hash(eckit::MD5&) const override;
    bool sameAs(const Method&) const override;
    void print(std::ostream&) const override;
    void json(eckit::JSON&) const override;

    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
};


}  // namespace mir::method
