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
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Types.h"


namespace mir {
namespace method {
namespace fe {


class FiniteElement : public MethodWeighted {
public:
    // -- Types

    struct ProjectionFail {
        static void list(std::ostream& out);
        enum type : unsigned int
        {
            failure = 0,
            missingValue,
            increaseEpsilon
        };
    };

    // -- Exceptions
    // None

    // -- Constructors

    FiniteElement(const param::MIRParametrisation&, const std::string& label = "input");

    // -- Destructor

    ~FiniteElement() override;

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
    // -- Members
    // None

    // -- Methods

    const util::MeshGeneratorParameters& meshGeneratorParams() const { return meshGeneratorParams_; }
    util::MeshGeneratorParameters& meshGeneratorParams() { return meshGeneratorParams_; }

    atlas::Mesh atlasMesh(util::MIRStatistics&, const repres::Representation&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    util::MeshGeneratorParameters meshGeneratorParams_;
    ProjectionFail::type projectionFail_;
    bool validateMesh_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MethodWeighted
    void hash(eckit::MD5&) const override;
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
    bool sameAs(const Method&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;

    int version() const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend struct L2Projection;
};


class FiniteElementFactory : public MethodFactory {
    std::string name_;
    virtual FiniteElement* make(const param::MIRParametrisation&, const std::string& label) = 0;
    FiniteElement* make(const param::MIRParametrisation&) override                          = 0;

protected:
    FiniteElementFactory(const std::string&);
    ~FiniteElementFactory() override;

public:
    FiniteElementFactory(const FiniteElementFactory&) = delete;
    FiniteElementFactory(FiniteElementFactory&&)      = delete;

    void operator=(const FiniteElementFactory&) = delete;
    void operator=(FiniteElementFactory&&)      = delete;

    static void list(std::ostream&);
    static FiniteElement* build(std::string& names, const std::string& label, const param::MIRParametrisation&);
};


template <class T>
class FiniteElementBuilder : public FiniteElementFactory {
    FiniteElement* make(const param::MIRParametrisation& param, const std::string& label) override {
        return new T(param, label);
    }

    FiniteElement* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    FiniteElementBuilder(const std::string& name) : FiniteElementFactory(name) {}
};


}  // namespace fe
}  // namespace method
}  // namespace mir
