/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Oct 2016


#include "mir/action/statistics/SHStatistics.h"

#include <sstream>
#include "mir/repres/sh/SphericalHarmonics.h"


namespace mir {
namespace action {
namespace statistics {


SHStatistics::SHStatistics(const param::MIRParametrisation& parametrisation) :
    Statistics(parametrisation) {
}


bool SHStatistics::sameAs(const action::Action& other) const {
    const SHStatistics* o = dynamic_cast<const SHStatistics*>(&other);
    return o; //(o && options_ == o->options_);
}


void SHStatistics::calculate(const data::MIRField& field, Results& results) const {
    results.reset();
    ASSERT(!field.hasMissing());

    for (size_t w = 0; w < field.dimensions(); ++w) {
        const std::vector<double>& values = field.values(w);
        ASSERT(values.size());

        std::string head;
        if (field.dimensions()>1) {
            std::ostringstream s;
            s << '#' << (w+1) << ' ';
            head = s.str();
        }


        // set truncation
        // Note: assumes triangular truncation (from GribInput.cc)
        size_t truncation = field.representation()->truncation();
        size_t N = repres::sh::SphericalHarmonics::number_of_complex_coefficients(truncation);
        ASSERT(2*N == values.size());

        const size_t J = truncation;


        // set mean
        const double mean = values[0];


        // set variances
        // Note: the _alt variant is pending GRIB-283 decision on how to calculate correctly spectral variance
        double var       = 0;
        double var_alt   = 0;

        for (size_t i = 2; i < 2*J ; i += 2) {
            var += values[i]*values[i];
        }
        var_alt = var;

        for (size_t i = 2*J; i < values.size(); i += 2) {
            var     += values[i]*values[i] - values[i+1]*values[i+1];
            var_alt += values[i]*values[i] + values[i+1]*values[i+1];
        }


        // set spectral energy norms
        ASSERT(var >= 0);
        ASSERT(var_alt >= 0);
        const double enorm     = std::sqrt(mean * mean + var    );
        const double enorm_alt = std::sqrt(mean * mean + var_alt);


        // set statistics results
        results.set(head + "mean",              mean);
        results.set(head + "variance",          var);
        results.set(head + "standardDeviation", std::sqrt(var));

        results.set(head + "enorm",             enorm);
        results.set(head + "enorm_alt",         enorm_alt);
        results.set(head + "const",             static_cast<bool>(var == 0));

    }
}


namespace {
static StatisticsBuilder<SHStatistics> statistics("SHStatistics");
}


}  // namespace statistics
}  // namespace action
}  // namespace mir

