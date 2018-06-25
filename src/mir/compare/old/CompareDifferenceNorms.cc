/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/compare/CompareDifferenceNorms.h"

#include <limits>
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "mir/util/Statistics.h"


namespace mir_cmp {


namespace  {


namespace statistics = util::statistics;


/// Calculate values difference norms, for a pair of fields (scalar quantities)
struct ReportDifferencesOnFieldsNorms :
        statistics::CountMissingValuesFn< double >,
        statistics::ScalarpNormsFn< double > {
public:
    ReportDifferencesOnFieldsNorms(const double& missingValue1, const double& missingValue2) :
        statistics::CountMissingValuesFn< double >(missingValue1, missingValue2) {
        reset();
    }
    void reset() {
        statistics::CountMissingValuesFn< double >::reset();
        statistics::ScalarpNormsFn< double >::reset();
    }
    Compare::CompareOptions results() const {
        Compare::CompareOptions r;
        r.set("diff_norm_L1",       normL1());
        r.set("diff_norm_L2",       normL2());
        r.set("diff_norm_Li",       normLinfinity());
        r.set("count",              countNonMissing());
        r.set("count_diff_missing", countDifferencesMissing());
        r.set("count_missing_1",    countMissing1());
        r.set("count_missing_2",    countMissing2());
        r.set("count_total",        countTotal());
        return r;
    }
    bool operator()(const double& v) {
        return statistics::ScalarpNormsFn< double >::operator()(v);
    }
    bool operator()(const double& v1, const double& v2) {
        return statistics::CountMissingValuesFn< double >::operator()(v1, v2)
            && statistics::ScalarpNormsFn < double >::operator()(std::abs(v1-v2));
    }
    using CountMissingValuesFn< double >::countDifferencesMissing;
};


}  // (anonymous namespace)


bool CompareDifferenceNorms::compare(
        const data::MIRField& field1, const param::MIRParametrisation&,
        const data::MIRField& field2, const param::MIRParametrisation& ) const {

    bool verbose = options_.get< bool >("compare-verbose");
    const double missingValue1 = field1.hasMissing()? field1.missingValue() : std::numeric_limits<double>::quiet_NaN();
    const double missingValue2 = field2.hasMissing()? field2.missingValue() : std::numeric_limits<double>::quiet_NaN();
    const size_t dim1 = field1.dimensions();
    const size_t dim2 = field2.dimensions();

    size_t w = 0;
    bool cmp = true;
    while (cmp && w<dim1 && w<dim2) {

        // access field values
        const MIRValuesVector& values1 = field1.values(w);
        const MIRValuesVector& values2 = field2.values(w);
        ++w;

        const size_t N = std::min(values1.size(), values2.size());
        eckit::Log::debug<LibMir>() << "\tN = min(#field A, #field B) = min(" << eckit::BigNum(values1.size()) << ", " << eckit::BigNum(values1.size()) << ")" << std::endl;

        // compare fields value count
        cmp = (values1.size() == values2.size());
        if (!cmp || verbose) {
            eckit::Log::info() << "\tfield A count: " << eckit::Plural(values1.size(), "value") << "\n"
                                  "\tfield B count: " << eckit::Plural(values2.size(), "value") << std::endl;
        }
        if (!cmp) {
            continue;
        }

        // perform value comparison
        ReportDifferencesOnFieldsNorms calc(missingValue1, missingValue2);
        for (size_t i=0; i<N; ++i) {
            calc(values1[i], values2[i]);
        }

        // set comparison results
        const double
                maxL1 = options_.get< double>("compare-max-L1"),
                maxL2 = options_.get< double>("compare-max-L2"),
                maxLi = options_.get< double>("compare-max-Li");
        cmp = (maxL1 != maxL1 || calc.normL1()   <= maxL1)
                && (maxL2 != maxL2 || calc.normL2()   <= maxL2)
                && (maxLi != maxLi || calc.normLinfinity() <= maxLi);
        if (!cmp || verbose) {
            eckit::Channel& out = eckit::Log::info();
            if (maxL1 == maxL1) { out << "\tL1|diff " << (calc.normL1()        < maxL1? "<":">") << " L1|max = " << maxL1 << "\n"; }
            if (maxL2 == maxL2) { out << "\tL2|diff " << (calc.normL2()        < maxL2? "<":">") << " L2|max = " << maxL2 << "\n"; }
            if (maxLi == maxLi) { out << "\tLi|diff " << (calc.normLinfinity() < maxLi? "<":">") << " Li|max = " << maxLi << "\n"; }
            out << "\tfield A - field B: " << calc.results() << std::endl;
        }

    }

    cmp = cmp && (dim1==dim2);
    if (dim1!=dim2 || verbose) {
        eckit::Log::info() << "\tfield A: " << eckit::Plural(dim1, "dimension") <<  "\n"
                              "\tfield B: " << eckit::Plural(dim2, "dimension") << std::endl;
    }
    return cmp;

}


namespace {
static ComparisonBuilder<CompareDifferenceNorms> compare("differenceNorms");
}


}  // namespace mir_cmp

