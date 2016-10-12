/*
 * (C) Copyright 1996-2015 ECMWF.
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


#ifndef Representation_H
#define Representation_H

#include <iosfwd>
#include <string>
#include <vector>
#include "eckit/memory/Counted.h"


struct grib_info;

namespace atlas {
namespace grid {
class Grid;
class Domain;
}
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
class MIRStatistics;
}
namespace context {
class Context;
}
namespace api {
class MIRJob;
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

    virtual Iterator* rotatedIterator() const; // After rotation
    virtual Iterator* unrotatedIterator() const; // Before rotation

    virtual void validate(const std::vector<double> &values) const;

    virtual void fill(grib_info&) const;
    virtual void fill(api::MIRJob&) const;

    // Return a cropped version
    virtual const Representation* cropped(const util::BoundingBox &bbox) const;

    virtual size_t frame(std::vector<double> &values, size_t size, double missingValue) const;

    virtual const Representation* truncate(size_t truncation, const std::vector<double>&, std::vector<double>&) const;

    virtual atlas::grid::Grid* atlasGrid() const;
    virtual atlas::grid::Domain atlasDomain() const;
    virtual atlas::grid::Domain atlasDomain(const util::BoundingBox&) const;

    virtual size_t truncation() const;

    virtual void comparison(std::string&) const;

    virtual void reorder(long scanningMode, std::vector<double>& values) const;

    virtual void setComplexPacking(grib_info&) const;
    virtual void setSimplePacking(grib_info&) const;
    virtual void setGivenPacking(grib_info&) const;

    virtual void cropToDomain(const param::MIRParametrisation &parametrisation, context::Context & ctx) const;

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

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    Representation(const Representation&);
    Representation& operator=(const Representation&);

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
