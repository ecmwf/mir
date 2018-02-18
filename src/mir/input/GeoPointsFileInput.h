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


#ifndef mir_input_GeoPointsFileInput_h
#define mir_input_GeoPointsFileInput_h

#include "mir/input/MIRInput.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace input {


class GeoPointsFileInput : public MIRInput {
public:

    // -- Exceptions
    // None

    // -- Contructors
    GeoPointsFileInput(const std::string& path, int which = -1);

    // -- Destructor

    virtual ~GeoPointsFileInput(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    const std::vector<double>& latitudes() const;
    const std::vector<double>& longitudes() const;
    const std::vector<double>& values() const;

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    std::string path_;
    param::SimpleParametrisation fieldParametrisation_;
    int which_;
    size_t dimensions_;
    size_t next_;
    bool hasMissing_;
    double missingValue_;

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;
    std::vector<double> values_;

    // -- Methods

    bool resetMissingValue(double& missingValue);

    // -- Overridden methods

    // From MIRInput
    virtual void print(std::ostream&) const; // Change to virtual if base class
    virtual bool sameAs(const MIRInput& other) const;

    virtual const param::MIRParametrisation &parametrisation(size_t which) const;
    virtual data::MIRField field() const;

    virtual bool next();
    virtual size_t dimensions() const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GeoPointsFileInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir


#endif

