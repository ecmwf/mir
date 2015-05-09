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
/// @date May 2015


#include "mir/method/MethodWeighted.h"

#include <cmath>
#include <limits>
#include <map>
#include <string>

#include "eckit/log/BigNum.h"
#include "eckit/log/Plural.h"
#include "eckit/log/Timer.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/types/FloatCompare.h"

#include "atlas/Grid.h"

#include "mir/data/MIRField.h"
#include "mir/lsm/LandSeaMask.h"
#include "mir/method/WeightCache.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


using atlas::Grid;

static eckit::Mutex local_mutex;

namespace mir {
namespace method {


static std::map<std::string, WeightMatrix> matrix_cache;


MethodWeighted::MethodWeighted(const param::MIRParametrisation &param) :
    Method(param) {
}


MethodWeighted::~MethodWeighted() {
}


// This returns a 'const' matrix so we ensure that we don't change it and break the in-memory cache
const WeightMatrix &MethodWeighted::getMatrix(const atlas::Grid &in, const atlas::Grid &out) const {

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const lsm::LandSeaMask &mask_in = lsm::LandSeaMask::lookupInput(parametrisation_, in);
    const lsm::LandSeaMask &mask_out = lsm::LandSeaMask::lookupOutput(parametrisation_, out);

    eckit::Log::info() << "++++ Input LSM is " << mask_in << std::endl;
    eckit::Log::info() << "++++ Output LSM is " << mask_out << std::endl;

    std::string key = cache_.generateKey(name(), in, out, mask_in, mask_out);

    std::map<std::string, WeightMatrix>::iterator j = matrix_cache.find(key);
    if (j != matrix_cache.end()) {
        return (*j).second;
    }

    // calculate weights matrix, apply mask if necessary

    WeightMatrix W(out.npts(), in.npts());

    if (!cache_.retrieve(key, W)) {

        compute_weights(in, out, W);

        // TODO: Apply LSMs

        applyMasks(W, mask_in, mask_out);

        // Mask should be considered in caching
        // applyMask(W, in, out);

        cache_.insert(key, W);
    }

    std::swap(matrix_cache[key], W);

    return matrix_cache[key];
}


void MethodWeighted::execute(data::MIRField &field, const atlas::Grid &in, const atlas::Grid &out) const {

    eckit::Log::info() << "MethodWeighted::execute" << std::endl;

    size_t npts_inp = in.npts();
    size_t npts_out = out.npts();

    const WeightMatrix &W = getMatrix(in, out);

    // TODO: ASSERT matrix size is npts_inp * npts_out

    // multiply interpolant matrix with field vector
    for (size_t i = 0; i < field.dimensions(); i++) {

        eckit::StrStream os;
        os << "Interpolating field ("  << npts_inp << " -> " << npts_out << ")" << eckit::StrStream::ends;
        std::string msg(os);
        eckit::Timer t(msg);

        ASSERT(field.values(i).size() == npts_inp);
        eckit::Log::info() << "Input field is " << field.values(i).size() << std::endl;

        std::vector<double> &values = field.values(i);
        std::vector<double> result(npts_out);

        Eigen::VectorXd::MapType vi = Eigen::VectorXd::Map( &values[0], npts_inp );
        Eigen::VectorXd::MapType vo = Eigen::VectorXd::Map( &result[0], npts_out );

        if (field.hasMissing()) {
            // Assumes compiler does return value optimization
            // otherwise we need to pass result matrix as parameter
            WeightMatrix MW = applyMissingValues(W, field, i);
            vo = MW * vi;
        } else {
            vo = W * vi;
        }

        field.values(result, i);  // Update field with result
    }
}


void MethodWeighted::compute_weights(const Grid &in, const Grid &out, WeightMatrix &W) const {
    if (in.same(out))
        W.setIdentity();        // grids are the same, use identity matrix
    else
        assemble(W, in, out);   // assemble matrix of coefficients
}


WeightMatrix MethodWeighted::applyMissingValues(const WeightMatrix &W, data::MIRField &field, size_t which) const {

    ASSERT(field.hasMissing());

    eckit::Log::info() << "Field has missing values" << std::endl;
    double missing = field.missingValue();
    const std::vector<double> &values = field.values(which);

    // setup sizes & counters
    const size_t
    Nivalues = values.size(),
    Novalues = W.rows();
    size_t count = 0;

    std::vector< bool > missvalues(Nivalues);
    for (size_t i = 0; i < Nivalues; i++) {
        missvalues[i] = (values[i] == missing);
        if (values[i] == missing) {
            count++;
        }
    }

    if (count == 0) {
        return W;
    }
    eckit::Log::info() << "Field has " << eckit::Plural(count, "missing value") << " out of " << eckit::BigNum(Nivalues) << std::endl;


    // sparse matrix weigths redistribution
    WeightMatrix X(W);
    size_t fix_misssome = 0;
    size_t fix_missall  = 0;
    for (size_t i = 0; i < X.rows(); i++) {

        // count missing values and accumulate weights
        double sum = 0.;
        size_t

        Nmiss = 0,
        Ncol  = 0;
        for (WeightMatrix::InnerIterator j(X, i); j; ++j, ++Ncol) {
            if (missvalues[j.col()])
                ++Nmiss;
            else
                sum += j.value();
        }


        const bool
        weights_zero = eckit::FloatCompare::is_equal(sum,0.),
        weights_one  = eckit::FloatCompare::is_equal(sum,1.);

        if ( (Ncol!=Nmiss) && !weights_zero && !weights_one) {
            std::cout << "  i=" << i << "  weights_zero=" << weights_zero << "  weights_one=" << weights_one << std::endl;
        }


        // redistribution
        if (!Nmiss) {

            // no missing values, no redistribution

        } else if ( (std::abs(sum) < std::numeric_limits< double >::epsilon()) ||
                    (Ncol == Nmiss) ) {
            ++fix_misssome;

            // all values are missing (or weights wrongly computed), special case
            // (it covers Ncol>0 with the above condition)
            for (WeightMatrix::InnerIterator j(X, i); j; ++j) {
                j.valueRef() = 0.;
                field.values(which)[j.col()] = missing;
            }
            WeightMatrix::InnerIterator(X, i).valueRef() = 1.;

        } else {
            ++fix_missall;

            // apply linear redistribution
            // TODO: new redistribution methods
            const double invsum = 1 / sum;
            for (WeightMatrix::InnerIterator j(X, i); j; ++j) {
                if (missvalues[j.col()]) {
                    field.values(which)[j.col()] = missing;
                    j.valueRef() = 0.;
                } else {
                    j.valueRef() *= invsum;
                }
            }

        }
    }

    // log corrections and return
    eckit::Log::info() << "Missing value corrections: " << fix_missall << '/' << fix_misssome << " some/all-missing out of " << eckit::BigNum(Novalues) << std::endl;
    return X;
}


void MethodWeighted::applyMasks(WeightMatrix &W, const lsm::LandSeaMask &imask, const lsm::LandSeaMask &omask) const {
    eckit::Log::info() << "======== MethodWeighted::applyBothMask(" << imask << "," << omask << ")" << std::endl;

    if(imask.active() ) {
        const data::MIRField &imask_field = imask.field();
        ASSERT(!imask_field.hasMissing());
        const std::vector<double>& lsm = imask_field.values(0);

        // Do something with input lsm
    }

    if(omask.active() ) {
        const data::MIRField &omask_field = omask.field();

        // Do something with input lsm
    }
}


}  // namespace method
}  // namespace mir

