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
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_KNearestLSM_h
#define mir_method_KNearestLSM_h

#include "mir/method/KNearestNeighbours.h"


namespace mir {
namespace method {


class KNearestLSM: public KNearestNeighbours {
public:

    KNearestLSM(const param::MIRParametrisation&);

    virtual ~KNearestLSM();

protected:

    virtual void hash(eckit::MD5&) const;

private:

    size_t nclosest_;

    virtual void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const;

    /// Update interpolation weigths matrix to account for field masked values
    virtual void applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const;

    virtual lsm::LandSeaMasks getMasks(const repres::Representation& in, const repres::Representation& out) const;

    virtual void print(std::ostream&) const;

    virtual const char* name() const;

    virtual size_t nclosest() const;

    virtual std::string distanceWeighting() const;

};


}  // namespace method
}  // namespace mir


#endif

