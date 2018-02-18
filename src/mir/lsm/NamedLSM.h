/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date September 2017


#ifndef mir_lsm_NamedLSM_h
#define mir_lsm_NamedLSM_h

#include "mir/lsm/LSMSelection.h"

#include "eckit/utils/MD5.h"


namespace eckit {
class PathName;
}


namespace mir {
namespace lsm {


class NamedLSM : public LSMSelection {
public:

    // -- Exceptions
    // None

    // -- Contructors

    NamedLSM(const std::string& name);

    // -- Destructor

    ~NamedLSM();

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

    void print(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods

    Mask* create(const param::MIRParametrisation&,
                 const repres::Representation&,
                 const std::string& which) const;

    std::string cacheKey(const param::MIRParametrisation&,
                         const repres::Representation&,
                         const std::string& which) const;

    // -- Friends
    // None

};


class NamedMaskFactory {
    virtual Mask* make(
            const param::MIRParametrisation& param,
            const repres::Representation& representation,
            const std::string& which) = 0;
    virtual void hashCacheKey(eckit::MD5&,
                              const param::MIRParametrisation&,
                              const repres::Representation&,
                              const std::string& which) = 0;
protected:
    const std::string name_;
    const std::string path_;
    NamedMaskFactory(const std::string& name, const std::string& path);
    virtual ~NamedMaskFactory();
public:
    static Mask* build(
            const param::MIRParametrisation&,
            const repres::Representation&,
            const std::string& which );
    static std::string cacheKey(
            const param::MIRParametrisation&,
            const repres::Representation&,
            const std::string& which );
    static void list(std::ostream&);
};


template<class T>
class NamedMaskBuilder : public NamedMaskFactory {
    virtual Mask* make(
            const param::MIRParametrisation& param,
            const repres::Representation& representation,
            const std::string& which) {
        return new T(name_, path_, param, representation, which);
    }
    virtual void hashCacheKey(eckit::MD5& md5,
                              const param::MIRParametrisation& parametrisation,
                              const repres::Representation& representation,
                              const std::string& which) {
        T::hashCacheKey(md5, path_, parametrisation, representation, which);
    }
public:
    NamedMaskBuilder(const std::string& name, const std::string& path) : NamedMaskFactory(name, path) {}
};


}  // namespace lsm
}  // namespace mir


#endif

