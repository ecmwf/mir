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
// #include "atlas/GridSpec.h"

#include "eckit/log/Timer.h"
#include "eckit/log/Plural.h"

#include "mir/data/MIRField.h"
#include "mir/method/WeightCache.h"
#include "mir/repres/Representation.h"
#include "mir/lsm/LandSeaMask.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {


MethodWeighted::MethodWeighted(const param::MIRParametrisation& param) :
    Method(param) {
}


MethodWeighted::~MethodWeighted() {
}


void MethodWeighted::execute(data::MIRField& field, const atlas::Grid& in, const atlas::Grid& out) const {
    eckit::Log::info() << "MethodWeighted::execute" << std::endl;


    // calculate weights matrix, apply mask if necessary
    size_t npts_inp = in.npts();
    size_t npts_out = out.npts();
    MethodWeighted::Matrix W(npts_out, npts_inp);

    WeightCache cache;

    const std::string whash = hash(in, out);
    if (!cache.get(whash, W)) {
      if (in.unique_id() == out.unique_id() && in.same(out)) {
        W.setIdentity();
      } else {
        eckit::Timer t("Calculating interpolation weights");
        assemble(W, in, out);
      }
      cache.add(whash, W);
    }

    // Mask should be considered in caching
    applyMask(W, in, out);

    // multiply interpolant matrix with field vector
    for(size_t i = 0; i < field.dimensions(); i++) {
        eckit::StrStream os;
        os << "Interpolating field ("  << npts_inp << " -> " << npts_out << ")" << eckit::StrStream::ends;
        std::string msg(os);
        eckit::Timer t(msg);

        ASSERT(field.values(i).size() == npts_inp);
        eckit::Log::info() << "Input field is " << field.values(i).size() << std::endl;

        std::vector<double>& values = field.values(i);
        std::vector<double> result(npts_out);

        Eigen::VectorXd::MapType vi = Eigen::VectorXd::Map( &values[0], npts_inp );
        Eigen::VectorXd::MapType vo = Eigen::VectorXd::Map( &result[0], npts_out );

        if(!field.hasMissing()) {
            // Assumes compiler does return value optimization
            // otherwise we need to pass result matrix as parameter
            Matrix MW = applyMissingValues(W, field, i);
            vo = MW * vi;
        }
        else {
            vo = W * vi;
        }

        field.values(result, i); // Update field with result
    }
}


MethodWeighted::Matrix MethodWeighted::applyMissingValues(const MethodWeighted::Matrix& W, data::MIRField& field, size_t which) const {
    ASSERT(field.hasMissing());

    eckit::Log::info() << "Field " << field << " has missing values" << std::endl;
    double missing = field.missingValue();
    const std::vector<double>& values = field.values(which);

    size_t count = 0;
    for(size_t i = 0; i < values.size(); i++) {
        if(values[i] == missing) {
            count++; // For now, just count
            // redistributesWeights(W, i);
        }
    }

    eckit::Log::info() << "Field " << field << " has " << eckit::Plural(count, "missing value") << std::endl;
    if(count == 0) {
        return W;
    }
    // More code here

    return W;
}


void MethodWeighted::applyMask(MethodWeighted::Matrix& W, const atlas::Grid& in, const atlas::Grid& out) const {

    bool use_lsm = false;
    if(parametrisation_.get("use.lsm", use_lsm) && use_lsm) {
        // std::auto_ptr<LandSeaMask> mask_in = lsm::LandSeaMaskFactory::build(parametrisation_);
        // std::auto_ptr<LandSeaMask> mask_out = lsm::LandSeaMaskFactory::build(parametrisation_);
    }
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


std::string MethodWeighted::hash(const atlas::Grid& in, const atlas::Grid& out) const {
    std::ostringstream os;
    os << name() << "." << in.unique_id() << "." << out.unique_id();
    return os.str();
}

}  // namespace method
}  // namespace mir

