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


#include <string>
#include "eckit/log/Timer.h"
#include "atlas/Grid.h"
#include "atlas/GridSpec.h"
#include "soyuz/data/MIRField.h"
#include "soyuz/repres/Representation.h"
#include "soyuz/method/WeightCache.h"
#include <eckit/value/Params.h>


#include "soyuz/method/MethodWeighted.h"


namespace mir {
namespace method {


MethodWeighted::MethodWeighted(const param::MIRParametrisation& param, const std::string& name) :
    Method(param),
    name_(name) {
}


MethodWeighted::~MethodWeighted() {
}


void MethodWeighted::execute(data::MIRField& field, const atlas::Grid& in, const atlas::Grid& out) const {
    eckit::Log::info() << "MethodWeighted::execute" << std::endl;


    // calculate weights matrix, apply mask if necessary
    size_t npts_inp = in.npts();
    size_t npts_out = out.npts();
    MethodWeighted::Matrix W(npts_out,npts_inp);

    WeightCache cache;
    const std::string whash = hash(in.spec(), out.spec());
    if (!cache.get( whash, W )) {
        if (in.uid() == out.uid() && in.same(out))
            W.setIdentity();
        else {
            eckit::Timer t("calculating interpolation weights");
            assemble(W, in, out);
        }
        cache.add( whash, W );
    }

    applyMask(W);

    // multiply interpolant matrix with field vector
    {
        eckit::Timer t("interpolating field (" + eckit::Translator< size_t, std::string >()(npts_inp) + " -> " + eckit::Translator< size_t, std::string >()(npts_out) + ")");
#if 0
        VectorXd::MapType vi = VectorXd::Map( fi.data<double>(), fi.size() );
        VectorXd::MapType vo = VectorXd::Map( fo.data<double>(), fo.size() );
        vo = W * vi;
#endif
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


void MethodWeighted::build_sptree(atlas::Grid& in) const {
    atlas::Mesh& i_mesh = in.mesh();

    std::string uidIn = in.uid();
    if( uidIn != uid_ )
        sptree_.reset( new PointSearch(i_mesh) );

    uid_ = uidIn;
}


}  // namespace method
}  // namespace mir

