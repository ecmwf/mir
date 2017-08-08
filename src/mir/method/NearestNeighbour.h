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


#ifndef mir_method_NearestNeighbour_H
#define mir_method_NearestNeighbour_H

#include "mir/method/Nearest.h"


namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

class NearestNeighbour: public Nearest {

public:

    NearestNeighbour(const param::MIRParametrisation&);

    virtual ~NearestNeighbour();

protected:

    virtual void hash(eckit::MD5&) const;

private:

    virtual void print(std::ostream&) const;
    virtual const char* name() const;
    virtual size_t nclosest() const;
    virtual std::string distanceWeighting() const;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif

