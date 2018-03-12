/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_compat_GribExpandBoundingBox_h
#define mir_compat_GribExpandBoundingBox_h

#include "mir/compat/GribCompatibility.h"


namespace mir {
namespace compat {


class GribExpandBoundingBox : public GribCompatibility {
public:

    GribExpandBoundingBox(const std::string&);

protected:

    void execute(const param::MIRParametrisation&, grib_handle*, grib_info&) const;
    void printParametrisation(std::ostream&, const param::MIRParametrisation&) const;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const;
    void initialise(const metkit::MarsRequest&, std::map<std::string, std::string>& postproc) const;
    void print(std::ostream&) const;

};


}  // namespace compat
}  // namespace mir


#endif

