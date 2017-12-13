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


#ifndef mir_method_knn_NearestNeighbour_h
#define mir_method_knn_NearestNeighbour_h

#include "mir/method/knn/KNearestNeighbours.h"


namespace mir {
namespace method {
namespace knn {


class NearestNeighbour: public KNearestNeighbours {
public:

    NearestNeighbour(const param::MIRParametrisation&);

    virtual ~NearestNeighbour();

private:

    virtual const char* name() const;

    virtual std::string distanceWeighting() const;
    virtual bool sameAs(const Method& other) const;

};


}  // namespace knn
}  // namespace method
}  // namespace mir


#endif

