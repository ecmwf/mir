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


#ifndef mir_method_ProxyMethod_h
#define mir_method_ProxyMethod_h

#include "mir/api/Atlas.h"
#include "mir/method/Cropping.h"
#include "mir/method/Method.h"


namespace mir {
namespace method {


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

    virtual ~ProxyMethod() = default;

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
    virtual void hash(eckit::MD5&) const;
    virtual int version() const;
    virtual void execute(context::Context&, const repres::Representation& in, const repres::Representation& out) const;
    virtual bool sameAs(const Method&) const;
    virtual bool canCrop() const;
    virtual void setCropping(const util::BoundingBox&);
    virtual bool hasCropping() const;
    virtual const util::BoundingBox& getCropping() const;
    virtual void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace method
}  // namespace mir


#endif
