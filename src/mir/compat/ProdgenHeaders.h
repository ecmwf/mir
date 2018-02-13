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
/// @date Feb 2018


#ifndef mir_compat_ProdgenArea_h
#define mir_compat_ProdgenArea_h

#include "mir/compat/GribCompatibility.h"


namespace mir {
namespace compat {


class ProdgenHeaders : public GribCompatibility {
public:

    ProdgenHeaders(const std::string&);


private:

    virtual void execute(const param::MIRParametrisation&, grib_info&) const;
    virtual void print(std::ostream&) const;


};



}  // namespace compat
}  // namespace mir


#endif

