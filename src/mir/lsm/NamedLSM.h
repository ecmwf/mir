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

#include "mir/lsm/LSMSelection.h"


namespace eckit {
class MD5;
}


namespace mir::lsm {


class NamedLSM : public LSMSelection {
public:
    // -- Exceptions
    // None

    // -- Constructors

    NamedLSM(const std::string& name);

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

    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods

    Mask* create(const param::MIRParametrisation&, const repres::Representation&,
                 const std::string& which) const override;

    std::string cacheKey(const param::MIRParametrisation&, const repres::Representation&,
                         const std::string& which) const override;

    // -- Friends
    // None
};


class NamedMaskFactory {
    virtual Mask* make(const param::MIRParametrisation&, const repres::Representation&, const std::string& which) = 0;
    virtual void hashCacheKey(eckit::MD5&, const param::MIRParametrisation&, const repres::Representation&,
                              const std::string& which)                                                           = 0;

protected:
    const std::string name_;
    const std::string path_;

    NamedMaskFactory(const std::string& name, const std::string& path);
    virtual ~NamedMaskFactory();

public:
    NamedMaskFactory(const NamedMaskFactory&) = delete;
    NamedMaskFactory(NamedMaskFactory&&)      = delete;

    void operator=(const NamedMaskFactory&) = delete;
    void operator=(NamedMaskFactory&&)      = delete;

    static Mask* build(const param::MIRParametrisation&, const repres::Representation&, const std::string& which);
    static std::string cacheKey(const param::MIRParametrisation&, const repres::Representation&,
                                const std::string& which);
    static void list(std::ostream&);
};


template <class T>
class NamedMaskBuilder : public NamedMaskFactory {
    Mask* make(const param::MIRParametrisation& param, const repres::Representation& representation,
               const std::string& which) override {
        return new T(name_, path_, param, representation, which);
    }
    void hashCacheKey(eckit::MD5& md5, const param::MIRParametrisation& parametrisation,
                      const repres::Representation& representation, const std::string& which) override {
        T::hashCacheKey(md5, path_, parametrisation, representation, which);
    }

public:
    NamedMaskBuilder(const std::string& name, const std::string& path) : NamedMaskFactory(name, path) {}
};


}  // namespace mir::lsm
