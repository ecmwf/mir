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

#include "mir/method/Cropping.h"
#include "mir/method/Method.h"
#include "mir/util/Atlas.h"
#include "mir/util/Types.h"


namespace mir::method {


class ProxyMethod : public Method {
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

    ProxyMethod(const param::MIRParametrisation&, std::string type);

    // -- Destructor

    ~ProxyMethod() override;

    // -- Members
    // None

    // -- Methods

    const std::string& type() const { return type_; }

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    atlas_config_t options_;
    const std::string type_;
    Cropping cropping_;

    // -- Methods
    // None

    // -- Overridden methods

    // From Method
    void hash(eckit::MD5&) const override;
    int version() const override;
    void execute(context::Context&, const repres::Representation& in, const repres::Representation& out) const override;
    bool sameAs(const Method&) const override;
    bool canCrop() const override;
    void setCropping(const util::BoundingBox&) override;
    bool hasCropping() const override;
    const util::BoundingBox& getCropping() const override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::method
