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


#ifndef mir_output_GeoPointsFileOutputXYV_h
#define mir_output_GeoPointsFileOutputXYV_h

#include "mir/output/GeoPointsFileOutput.h"


namespace mir {
namespace output {


class GeoPointsFileOutputXYV : public GeoPointsFileOutput {
public:
    // -- Exceptions
    // None

    // -- Contructors

    GeoPointsFileOutputXYV(const ::std::string& path, bool binary = false);

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
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    // From MIROutput
    virtual size_t copy(const param::MIRParametrisation&, context::Context&);
    virtual size_t save(const param::MIRParametrisation&, context::Context&);

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    size_t saveText(const param::MIRParametrisation&, context::Context&);
    size_t saveBinary(const param::MIRParametrisation&, context::Context&);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const GeoPointsFileOutputXYV& p)
    // { p.print(s); return s; }
};


}  // namespace output
}  // namespace mir


#endif
