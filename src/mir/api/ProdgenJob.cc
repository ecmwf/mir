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


namespace mir {
namespace api {


ProdgenJob::ProdgenJob() {
}


ProdgenJob::~ProdgenJob() {
}


void ProdgenJob::print(std::ostream& out) const {
    out << "ProdgenJob[";
    out << "]";
}

void ProdgenJob::usewind(bool on) {
    eckit::Log::info() << "ProdgenJob::usewind " << on << std::endl;
}

void ProdgenJob::uselsm(bool on) {
    eckit::Log::info() << "ProdgenJob::uselsm " << on << std::endl;
}

void ProdgenJob::useprecip(bool on) {
    eckit::Log::info() << "ProdgenJob::useprecip " << on << std::endl;
}

void ProdgenJob::missingvalue(bool on) {
    eckit::Log::info() << "ProdgenJob::missingvalue " << on << std::endl;
}

void ProdgenJob::lsm_param(bool on) {
    eckit::Log::info() << "ProdgenJob::lsm_param " << on << std::endl;
}

void ProdgenJob::parameter(size_t n) {
    eckit::Log::info() << "ProdgenJob::parameter " << n << std::endl;
}

void ProdgenJob::table(size_t n) {
    eckit::Log::info() << "ProdgenJob::table " << n << std::endl;
}

void ProdgenJob::reduced(size_t n) {
    eckit::Log::info() << "ProdgenJob::reduced " << n << std::endl;
}

void ProdgenJob::g_pnts(size_t n) {
    eckit::Log::info() << "ProdgenJob::g_pnts " << n << std::endl;
}


}  // namespace api
}  // namespace mir

