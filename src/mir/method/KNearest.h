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


#ifndef mir_method_KNearest_h
#define mir_method_KNearest_h

#include "mir/method/KNearestNeighbours.h"


namespace mir {
namespace method {


class KNearest: public KNearestNeighbours {
public:

    KNearest(const param::MIRParametrisation&);

    virtual ~KNearest();

private:

    virtual const char* name() const;

};


}  // namespace method
}  // namespace mir


#endif

