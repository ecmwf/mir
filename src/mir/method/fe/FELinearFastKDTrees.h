/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_method_fe_FELinearFastKDTrees_h
#define mir_method_fe_FELinearFastKDTrees_h

#include "mir/method/fe/FELinear.h"


namespace mir {
namespace method {
namespace fe {


struct FELinearFastKDTrees: public FELinear {
    FELinearFastKDTrees(const param::MIRParametrisation& param) :
        FELinear(param) {
    }
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const;
};


}  // namespace fe
}  // namespace method
}  // namespace mir


#endif

