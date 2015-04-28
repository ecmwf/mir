/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/method/MethodWeighted.h"

#include <string>
#include "atlas/Grid.h"
#include "atlas/GridSpec.h"

#include "eckit/log/Timer.h"

#include "mir/data/MIRField.h"
#include "mir/method/WeightCache.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace method {


MethodWeighted::MethodWeighted(const param::MIRParametrisation& param, const std::string& name) :
    Method(param),
    name_(name) {
}


MethodWeighted::~MethodWeighted() {
}


void MethodWeighted::execute(data::MIRField& field, size_t which, const atlas::Grid& in, const atlas::Grid& out, std::vector<double>& result) const {
    eckit::Log::info() << "MethodWeighted::execute" << std::endl;


    // calculate weights matrix, apply mask if necessary
    size_t npts_inp = in.npts();
    size_t npts_out = out.npts();
    MethodWeighted::Matrix W(npts_out, npts_inp);

    WeightCache cache;
    const std::string whash = hash(in.spec(), out.spec());
    if (!cache.get( whash, W )) {
        if (in.uid() == out.uid() && in.same(out))
            W.setIdentity();
        else {
            eckit::Timer t("Calculating interpolation weights");
            assemble(W, in, out);
        }
        cache.add( whash, W );
    }

    applyMask(W);

    // multiply interpolant matrix with field vector
    {
        eckit::StrStream os;
        os << "Interpolating field ("  << npts_inp << " -> " << npts_out << ")" << eckit::StrStream::ends;
        std::string msg(os);
        eckit::Timer t(msg);

        ASSERT(field.values(which).size() == npts_inp);
        eckit::Log::info() << "Input field is " << field.values(which).size() << std::endl;

        std::vector<double>& values = field.values(which);
        result.resize(npts_out);

        Eigen::VectorXd::MapType vi = Eigen::VectorXd::Map( &values[0], npts_inp );
        Eigen::VectorXd::MapType vo = Eigen::VectorXd::Map( &result[0], npts_out );
        vo = W * vi;

    }
}


void MethodWeighted::applyMask(MethodWeighted::Matrix& W) const {
#if 0
//FIXME
    // FIXME arguments:
    atlas::Grid*       dummy_grid = 0;
    atlas::Grid& in  (*dummy_grid);
    atlas::Grid& out (*dummy_grid);

    if( params().has("MaskPath") ) {
        PathName mask_path = params()["MaskPath"];

        FieldSet::Ptr fmask( new FieldSet( mask_path ) );
        ASSERT( fmask );

        if( fmask->size() != 1 )
            throw UserError( "User provided mask file with multiple fields", Here() );

        Masks m;
        m.assemble( (*fmask)[0], in, out, W );
    }
#endif
}


std::string MethodWeighted::hash(const atlas::GridSpec& inspec, const atlas::GridSpec& outspec) const {
    const std::string dot(".");
    return name_ + dot + inspec.uid() + dot + outspec.uid();
}


void MethodWeighted::print(std::ostream&) const {

}


}  // namespace method
}  // namespace mir

