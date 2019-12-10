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


#ifndef mir_repres_other_UnstructuredGrid_h
#define mir_repres_other_UnstructuredGrid_h

#include <vector>

#include "mir/repres/Gridded.h"


namespace eckit {
class PathName;
}


namespace mir {
namespace repres {
namespace other {


class UnstructuredGrid : public Gridded {
public:
    // -- Exceptions
    // None

    // -- Contructors

    UnstructuredGrid(const eckit::PathName&);
    UnstructuredGrid(const param::MIRParametrisation&);
    UnstructuredGrid(const std::vector<double>& latitudes, const std::vector<double>& longitudes,
                     const util::BoundingBox& = util::BoundingBox());

    // -- Destructor

    ~UnstructuredGrid();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static void save(const eckit::PathName&, const std::vector<double>& latitudes,
                     const std::vector<double>& longitudes, bool binary);

    static void check(const std::string& title, const std::vector<double>& latitudes,
                      const std::vector<double>& longitudes);

    // -- Overridden methods


    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    // From Representation
    virtual bool extendBoundingBoxOnIntersect() const;
    virtual void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual void fill(grib_info&) const;
    virtual void fill(api::MIRJob&) const;
    virtual void fill(util::MeshGeneratorParameters&) const;

    virtual atlas::Grid atlasGrid() const;
    virtual void validate(const MIRValuesVector&) const;

    virtual util::Domain domain() const;
    virtual Iterator* iterator() const;
    virtual void makeName(std::ostream&) const;
    virtual bool sameAs(const Representation&) const;

    // Domain operations
    virtual bool isPeriodicWestEast() const;
    virtual bool includesNorthPole() const;
    virtual bool includesSouthPole() const;

    virtual size_t numberOfPoints() const;
    virtual const Gridded* croppedRepresentation(const util::BoundingBox&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const UnstructuredGrid& p)
    //  { p.print(s); return s; }
};


}  // namespace other
}  // namespace repres
}  // namespace mir


#endif
