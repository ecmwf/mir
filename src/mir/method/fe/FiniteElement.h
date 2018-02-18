/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @author Willem Deconinck
/// @date May 2015


#ifndef mir_method_fe_FiniteElement_h
#define mir_method_fe_FiniteElement_h

#include "mir/method/MethodWeighted.h"
#include "mir/util/MIRGrid.h"


namespace mir {
namespace method {
namespace fe {


class FiniteElement: public MethodWeighted {

public:

    FiniteElement(const param::MIRParametrisation&);

    virtual ~FiniteElement();

protected:

    virtual void hash(eckit::MD5&) const;

protected: // methods

    virtual void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const;
    virtual bool sameAs(const Method& other) const = 0;
    virtual void print(std::ostream &out) const = 0;


protected: // members

    util::MIRGrid::MeshGenParams inputMeshGenerationParams_;

    // FIXME: this does not belong in that class
    util::MIRGrid::MeshGenParams outputMeshGenerationParams_;

};


}  // namespace fe
}  // namespace method
}  // namespace mir


#endif

