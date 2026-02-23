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


namespace mir::method::fe {


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

    explicit FiniteElement(const param::MIRParametrisation&);

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


}  // namespace mir::method::fe
