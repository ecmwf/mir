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
/// @author Pedro Maciel
/// @date Apr 2015


#include <iostream>

#include "eckit/log/Log.h"
#include "mir/api/ProdgenJob.h"
#include "eckit/exception/Exceptions.h"
#include "mir/log/MIR.h"

#include "atlas/Grid.h"
#include "atlas/grids/grids.h"
#include "atlas/grids/GaussianLatitudes.h"

namespace mir {
namespace api {


ProdgenJob::ProdgenJob():
    gridType_("unknown"),
    N_(0),
    truncation_(0),
    gridded_(false),
    spectral_(false),
    usewind_(false),
    uselsm_(false),
    useprecip_(false),
    hasMissing_(false),
    lsm_param_(false),
    parameter_(0),
    table_(0),
    missingValue_(-55555.0),
    nj_(0) {
}


ProdgenJob::~ProdgenJob() {
}


void ProdgenJob::print(std::ostream &out) const {
    out << "ProdgenJob[";
    out << "]";
}

void ProdgenJob::usewind(bool on) {
    eckit::Log::trace<MIR>() << "ProdgenJob::usewind " << on << std::endl;
    usewind_ = on;
}

void ProdgenJob::uselsm(bool on) {
    eckit::Log::trace<MIR>() << "ProdgenJob::uselsm " << on << std::endl;
    uselsm_ = on;
}

void ProdgenJob::useprecip(bool on) {
    eckit::Log::trace<MIR>() << "ProdgenJob::useprecip " << on << std::endl;
    useprecip_ = on;
}

void ProdgenJob::hasMissing(bool on) {
    eckit::Log::trace<MIR>() << "ProdgenJob::hasMissing " << on << std::endl;
    missingValue_ = on;
}

void ProdgenJob::missingValue(double missing) {
    eckit::Log::trace<MIR>() << "ProdgenJob::missingValue " << missing << std::endl;
    missingValue_ = missing;
    hasMissing_ = true;
}

void ProdgenJob::lsm_param(bool on) {
    eckit::Log::trace<MIR>() << "ProdgenJob::lsm_param " << on << std::endl;
    lsm_param_ = on;
}

void ProdgenJob::parameter(size_t n) {
    eckit::Log::trace<MIR>() << "ProdgenJob::parameter " << n << std::endl;
    parameter_ = n;
}

void ProdgenJob::table(size_t n) {
    eckit::Log::trace<MIR>() << "ProdgenJob::table " << n << std::endl;
    table_ = n;
}

void ProdgenJob::reduced(size_t n) {
    eckit::Log::trace<MIR>() << "ProdgenJob::reduced " << n << std::endl;
    gridType_ = "reduced_gg";
    N_ = n;
    gridded_ = true;
    spectral_ = false;
}

void ProdgenJob::truncation(size_t n) {
    eckit::Log::trace<MIR>() << "ProdgenJob::truncation " << n << std::endl;
    gridType_ = "sh";
    truncation_ = n;
    spectral_ = true;
    gridded_ = false;
}

void ProdgenJob::reduced_ll(size_t nj,
                            const int pl[]) {

    eckit::Log::trace<MIR>() << "ProdgenJob::reduced_ll"  << std::endl;

    gridType_ = "reduced_ll";
    gridded_ = true;
    spectral_ = false;

    nj_ = nj;
    pl_.resize(nj);

    for (size_t i = 0; i < nj; i++) {
        pl_[i] = pl[i];
    }
}

void ProdgenJob::g_pnts(const int *pl) {
    eckit::Log::trace<MIR>() << "ProdgenJob::g_pnts " << std::endl;
    ASSERT(gridType_ == "reduced_gg");

    size_t size = 2 * N_;
    pl_.resize(size);
    for (size_t i = 0; i < size; i++) {
        pl_[i] = pl[i];
    }
}

const std::vector<long> &ProdgenJob::pl() const {
    return pl_;
}

size_t ProdgenJob::N() const {
    return N_;
}

size_t ProdgenJob::truncation() const {
    return truncation_;
}

size_t ProdgenJob::paramId() const {
    ASSERT(parameter_);
    if (table_ == 128) {
        return parameter_;
    }
    return table_ * 1000 + parameter_;
}

const util::BoundingBox &ProdgenJob::bbox() const {
    return bbox_;
}

const std::string &ProdgenJob::gridType() const {
    return gridType_;
}

bool ProdgenJob::gridded() const {
    eckit::Log::trace<MIR>() << "ProdgenJob::gridded " << gridded_ << std::endl;
    return gridded_;
}

bool ProdgenJob::spectral() const {
    eckit::Log::trace<MIR>() << "ProdgenJob::spectral " << spectral_ << std::endl;
    return spectral_;
}

size_t ProdgenJob::nj() const {
    eckit::Log::trace<MIR>() << "ProdgenJob::nj " << nj_ << std::endl;
    return nj_;
}

void ProdgenJob::auto_pl() {
    std::ostringstream os;
    os << "rgg.N" << N_;
    eckit::ScopedPtr<atlas::grids::ReducedGrid> grid(dynamic_cast<atlas::grids::ReducedGrid *>(atlas::Grid::create(os.str())));

    ASSERT(grid.get());

    const std::vector<int> &v = grid->npts_per_lat();
    pl_.resize(v.size());
    for (size_t i = 0; i < v.size(); i++) {
        pl_[i] = v[i];
    }
}

bool ProdgenJob::hasMissing() const {
    return hasMissing_;
}

double ProdgenJob::missingValue() const {
    return missingValue_;
}

}  // namespace api
}  // namespace mir

