// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/Method.h"


namespace mir::method {


class FailMethod final : public Method {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    using Method::Method;

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

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    // From Method
    const char* type() const override;
    void hash(eckit::MD5&) const override;
    bool sameAs(const Method&) const override;
    int version() const override;

    bool canCrop() const override;
    void setCropping(const util::BoundingBox&) override;
    bool hasCropping() const override;
    const util::BoundingBox& getCropping() const override;

    void execute(context::Context&, const repres::Representation& in, const repres::Representation& out) const override;

    void print(std::ostream&) const override;
    void json(eckit::JSON&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::method
