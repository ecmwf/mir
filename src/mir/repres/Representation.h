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

#include <iosfwd>
#include <string>
#include <vector>

#include "eckit/memory/Counted.h"

#include "mir/util/IndexMapping.h"
#include "mir/util/Types.h"


struct grib_info;

namespace atlas {
class Grid;
}

namespace eckit {
class JSON;
}

namespace mir {

namespace param {
class MIRParametrisation;
}

namespace repres {
class Iterator;
}

namespace util {
class BoundingBox;
class Domain;
class GridBox;
class MeshGeneratorParameters;
}  // namespace util

namespace api {
class MIRJob;
}

namespace data {
class MIRField;
}

namespace api {
class MIREstimation;
}

}  // namespace mir


namespace mir::repres {


class Representation : public eckit::Counted {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Representation();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual const std::string& uniqueName() const;
    virtual bool sameAs(const Representation&) const;

    virtual Iterator* iterator() const;

    virtual void validate(const MIRValuesVector&) const;

    virtual void fillGrib(grib_info&) const;
    virtual void fillJob(api::MIRJob&) const;
    virtual void fillMeshGen(util::MeshGeneratorParameters&) const;
    virtual void estimate(api::MIREstimation&) const;

    // Return a cropped version
    virtual const Representation* croppedRepresentation(const util::BoundingBox&) const;
    virtual util::BoundingBox extendBoundingBox(const util::BoundingBox&) const;
    virtual bool extendBoundingBoxOnIntersect() const;
    virtual bool crop(util::BoundingBox&, util::IndexMapping&) const;

    virtual size_t frame(MIRValuesVector&, size_t size, double missingValue, bool estimate = false) const;

    virtual const Representation* globalise(data::MIRField&) const;

    virtual size_t numberOfPoints() const;
    virtual size_t numberOfValues() const;
    virtual bool getLongestElementDiagonal(double&) const;

    virtual const Representation* truncate(size_t truncation, const MIRValuesVector&, MIRValuesVector&) const;

    virtual atlas::Grid atlasGrid() const;

    // Domain operations
    virtual util::Domain domain() const;
    virtual const util::BoundingBox& boundingBox() const;
    virtual bool isGlobal() const;
    virtual bool isPeriodicWestEast() const;
    virtual bool includesNorthPole() const;
    virtual bool includesSouthPole() const;

    virtual size_t truncation() const;

    virtual void comparison(std::string&) const;

    virtual void reorder(long scanningMode, MIRValuesVector&) const;

    virtual std::vector<util::GridBox> gridBoxes() const;

    virtual std::string factory() const;  // Return factory name

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Destructor

    ~Representation() override;

    // -- Members
    // None

    // -- Methods

    virtual void json(eckit::JSON&) const;
    virtual void print(std::ostream&) const = 0;
    virtual void makeName(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members

    mutable std::string uniqueName_;

    // -- Class methods
    // None

private:
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

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Representation& p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const Representation& p) {
        p.json(s);
        return s;
    }
};


class RepresentationHandle {
    const Representation* representation_;

public:
    RepresentationHandle(const Representation*);
    explicit RepresentationHandle(const RepresentationHandle&);
    ~RepresentationHandle();
    const Representation* operator->() const { return representation_; }
    operator const Representation*() const { return representation_; }
    operator bool() const { return representation_ != nullptr; }

private:
    RepresentationHandle& operator=(const RepresentationHandle&) = delete;
};


class RepresentationFactory {
    std::string name_;
    virtual Representation* make(const param::MIRParametrisation&) = 0;

    RepresentationFactory(const RepresentationFactory&)            = delete;
    RepresentationFactory& operator=(const RepresentationFactory&) = delete;

protected:
    explicit RepresentationFactory(const std::string&);
    virtual ~RepresentationFactory();

public:
    // This is 'const' as the representation uses reference counting
    // Represention should always be immutable
    static const Representation* build(const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class RepresentationBuilder : public RepresentationFactory {
    Representation* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    explicit RepresentationBuilder(const std::string& name) : RepresentationFactory(name) {}
};


}  // namespace mir::repres
