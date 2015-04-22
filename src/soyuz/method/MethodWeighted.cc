/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "soyuz/method/MethodWeighted.h"

#include <string>

#include "eckit/log/Timer.h"
#include "soyuz/method/WeightCache.h"


namespace mir {
namespace method {


MethodWeighted::~MethodWeighted() {
}


void MethodWeighted::execute(data::MIRField& field) const {
    // FIXME arguments:
    atlas::Grid*       dummy_grid = 0;
    atlas::Grid& in  (*dummy_grid);
    atlas::Grid& out (*dummy_grid);

    // calculate weights matrix, apply mask if necessary
    size_t npts_inp = in.npts();
    size_t npts_out = out.npts();
    MethodWeighted::Matrix W(npts_out,npts_inp);

    WeightCache cache;
    const std::string whash = hash();
    if (!cache.get( whash, W )) {
        if (in.uid() == out.uid() && in.same(out))
            W.setIdentity();
        else {
            eckit::Timer t("calculating interpolation weights");
            assemble(W);
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


std::string MethodWeighted::hash() const {
    // FIXME arguments:
    atlas::Grid*       dummy_grid = 0;
    atlas::Grid& in  (*dummy_grid);
    atlas::Grid& out (*dummy_grid);

    const std::string dot(".");
    return name() + dot + in.uid() + dot + out.uid();
}


}  // namespace method
}  // namespace mir

