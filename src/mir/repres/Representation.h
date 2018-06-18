/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_repres_Representation_h
#define mir_repres_Representation_h

#include <iosfwd>
#include <string>
#include "eckit/memory/Counted.h"
#include "mir/data/MIRValuesVector.h"


struct grib_info;


namespace atlas {
class Grid;
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
class MeshGeneratorParameters;
}

namespace context {
class Context;
}

namespace api {
class MIRJob;
}

namespace data {
class MIRField;
}

}


namespace mir {
namespace repres {


class Representation : public eckit::Counted {
public:

    // Scanning mode bits
    enum {
        iScansNegatively      = 1 << 7,
        jScansPositively      = 1 << 6,
        jPointsAreConsecutive = 1 << 5,
        alternateRowScanning  = 1 << 4
    };

    // -- Exceptions
    // None

    // -- Constructors

    Representation();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods


    // --------------------

    virtual const std::string& uniqueName() const;
    virtual bool sameAs(const Representation&) const;

    virtual Iterator* iterator() const;

    virtual void validate(const MIRValuesVector&) const;

    virtual void fill(grib_info&) const;
    virtual void fill(api::MIRJob&) const;
    virtual void fill(util::MeshGeneratorParameters&) const;

    // Return a cropped version
    virtual const Representation* croppedRepresentation(const util::BoundingBox&) const;
    virtual util::BoundingBox extendedBoundingBox(const util::BoundingBox&) const;

    virtual size_t frame(MIRValuesVector&, size_t size, double missingValue) const;
    virtual const Representation* globalise(data::MIRField&) const;

    virtual size_t numberOfPoints() const;
    virtual bool getLongestElementDiagonal(double&) const;

    virtual const Representation* truncate(size_t truncation, const MIRValuesVector&, MIRValuesVector&) const;

    virtual atlas::Grid atlasGrid() const;

    virtual util::Domain domain() const;
    virtual const util::BoundingBox& boundingBox() const;

    virtual bool isGlobal() const;

    virtual size_t truncation() const;

    virtual void comparison(std::string&) const;

    virtual void reorder(long scanningMode, MIRValuesVector&) const;

    virtual void setComplexPacking(grib_info&) const;
    virtual void setSimplePacking(grib_info&) const;
    virtual void setGivenPacking(grib_info&) const;

    virtual void crop(const param::MIRParametrisation&, context::Context&) const;

    virtual void shape(size_t& ni, size_t& nj) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Destructor

    virtual ~Representation();

    // -- Members
    // None

    // -- Methods

    virtual void print(std::ostream&) const = 0;
    virtual void makeName(std::ostream&) const;

    // Domain operations
    virtual bool isPeriodicWestEast() const;
    virtual bool includesNorthPole() const;
    virtual bool includesSouthPole() const;

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

};


class RepresentationHandle {
    const Representation* representation_;
public:
    RepresentationHandle(const Representation* r);
    ~RepresentationHandle();
    const Representation* operator->() const {
        return representation_;
    }
    operator const Representation*() const {
        return representation_;
    }
};


class RepresentationFactory {
    std::string name_;
    virtual Representation* make(const param::MIRParametrisation&) = 0 ;
protected:
    RepresentationFactory(const std::string&);
    virtual ~RepresentationFactory();
public:
    // This is 'const' as the representation uses reference counting
    // Represention should always be immutable
    static const Representation* build(const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template<class T>
class RepresentationBuilder : public RepresentationFactory {
    virtual Representation* make(const param::MIRParametrisation& param) {
        return new T(param);
    }
public:
    RepresentationBuilder(const std::string& name) : RepresentationFactory(name) {}
};


}  // namespace repres
}  // namespace mir


#endif
