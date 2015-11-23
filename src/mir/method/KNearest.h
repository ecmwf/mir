/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_KNearest_H
#define mir_method_KNearest_H

#include "mir/method/Nearest.h"


namespace mir {
namespace method {


class KNearest: public Nearest {
public:

    KNearest(const param::MIRParametrisation&);

    virtual ~KNearest();

protected:

    virtual void hash( eckit::MD5& ) const;

private:

    // -- Members

    size_t nclosest_;

    // -- Methods

    virtual void print(std::ostream&) const;

    virtual const char* name() const;

    virtual size_t nclosest() const;

    // -- Class members
    // None

    // -- Friends
    // None

};


}  // namespace method
}  // namespace mir

#endif

