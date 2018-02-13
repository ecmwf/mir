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


#ifndef mir_output_GribCompatibility_h
#define mir_output_GribCompatibility_h

#include <iosfwd>
#include <string>
#include "eckit/memory/NonCopyable.h"

struct grib_info;
struct grib_handle;


namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace compat {


class GribCompatibility : private eckit::NonCopyable {
public:

    static const GribCompatibility& lookup(const std::string& name);

    virtual void execute(const param::MIRParametrisation&, grib_handle*, grib_info&) const = 0;

    static void list(std::ostream& out);

protected:

    GribCompatibility(const std::string&);
    virtual ~GribCompatibility();

    virtual void print(std::ostream&) const = 0;

private:

    std::string name_;

    friend std::ostream& operator<<(std::ostream& s, const GribCompatibility& p) {
        p.print(s);
        return s;
    }

};



}  // namespace compat
}  // namespace mir


#endif

