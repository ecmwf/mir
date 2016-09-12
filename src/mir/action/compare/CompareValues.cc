/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/compare/CompareValues.h"

#include <limits>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/util/Statistics.h"


namespace mir {
namespace action {
namespace compare {


namespace  {


namespace statistics = util::statistics;


/// Calculate values differences, for a pair of fields (scalar quantities)
struct ReportDifferencesOnFieldsScalar :
        statistics::CountMissingValuesFn< double >,
        statistics::ScalarValueMinMaxFn< double > {
private:
    CompareValues::CompareValuesFn& compare_fn_;
    size_t countDifferencesValue_;
public:
    ReportDifferencesOnFieldsScalar(
            CompareValues::CompareValuesFn& compare_fn,
            const double& missingValue1,
            const double& missingValue2 ) :
        statistics::CountMissingValuesFn< double >(missingValue1, missingValue2),
        compare_fn_(compare_fn) {
        reset();
    }
    void reset() {
        statistics::CountMissingValuesFn< double >::reset();
        statistics::ScalarValueMinMaxFn< double >::reset();
        countDifferencesValue_ = 0;
    }
    Compare::CompareOptions results() const {
        Compare::CompareOptions r;
        r.set("min",       min());
        r.set("max",       max());
        r.set("min_index", minIndex());
        r.set("max_index", maxIndex());
        r.set("count",              countNonMissing());
        r.set("count_diff_missing", countDifferencesMissing());
        r.set("count_diff_value",   countDifferencesValue());
        r.set("count_diff_total",   countDifferences());
        r.set("count_missing_1",    countMissing1());
        r.set("count_missing_2",    countMissing2());
        r.set("count_total",        countTotal());
        return r;
    }
    bool operator()(const double& v) { return statistics::ScalarValueMinMaxFn< double >::operator()(v); }
    bool operator()(const double& v1, const double& v2) {
        if (statistics::CountMissingValuesFn< double >::operator()(v1, v2)) {
            statistics::ScalarValueMinMaxFn< double >::operator()(std::abs(v1-v2));
            if (compare_fn_(v1, v2)) {
                return true;
            }
            ++countDifferencesValue_;
        }
        return false;
    }
    using ScalarValueMinMaxFn< double >::max;
    using ScalarValueMinMaxFn< double >::maxIndex;
    size_t countDifferencesValue()   const { return countDifferencesValue_; }
    size_t countDifferencesMissing() const { return CountMissingValuesFn< double >::countDifferencesMissing(); }
    size_t countDifferences()        const { return (countDifferencesValue() + countDifferencesMissing()); }
};


/// Calculate vector (polar 2D representation) angle differences, for a pair of fields (angular quantities)
struct ReportDifferencesOnFieldsAngleDegrees :
        statistics::CountMissingValuesFn< double >,
        statistics::ScalarValueMinMaxFn< double > {
private:
    CompareValues::CompareValuesFn& compare_fn_;
    size_t countDifferencesValue_;
public:
    ReportDifferencesOnFieldsAngleDegrees(
            CompareValues::CompareValuesFn& compare_fn,
            const double& missingValue1,
            const double& missingValue2 ) :
        statistics::CountMissingValuesFn< double >(missingValue1, missingValue2),
        compare_fn_(compare_fn) {
        reset();
    }
    void reset() {
        statistics::CountMissingValuesFn< double >::reset();
        statistics::ScalarValueMinMaxFn< double >::reset();
        countDifferencesValue_ = 0;
    }
    Compare::CompareOptions results() const {
        Compare::CompareOptions r;
        r.set("min",       min());
        r.set("max",       max());
        r.set("min_index", minIndex());
        r.set("max_index", maxIndex());
        r.set("count",              countNonMissing());
        r.set("count_diff_missing", countDifferencesMissing());
        r.set("count_diff_value",   countDifferencesValue());
        r.set("count_diff_total",   countDifferences());
        r.set("count_missing_1",    countMissing1());
        r.set("count_missing_2",    countMissing2());
        r.set("count_total",        countTotal());
        return r;
    }
    bool operator()(const double& v) { return statistics::ScalarValueMinMaxFn< double >::operator()(v); }
    bool operator()(const double& v1, const double& v2) {
        if (statistics::CountMissingValuesFn< double >::operator()(v1, v2)) {
            // the angles are expected in degrees, map their difference to [-180., 180] and compare
            double a = util::angles::between_m180_and_p180(v1 - v2);
            statistics::ScalarValueMinMaxFn< double >::operator()(std::abs(a));
            if (compare_fn_(a, 0.)) {
                return true;
            }
            ++countDifferencesValue_;
        }
        return false;
    }
    using ScalarValueMinMaxFn< double >::max;
    using ScalarValueMinMaxFn< double >::maxIndex;
    size_t countDifferencesValue()   const { return countDifferencesValue_; }
    size_t countDifferencesMissing() const { return CountMissingValuesFn< double >::countDifferencesMissing(); }
    size_t countDifferences()        const { return (countDifferencesValue() + countDifferencesMissing()); }
};


}  // (anonymous namespace)


CompareValues::CompareValuesFn* CompareValues::getCompareFunctor(double missingValue1, double missingValue2, double packingError) const {
    const std::string compareMode = options_.get< std::string >("compare.mode");
    if (compareMode=="packing") {

        const double packing_factor = options_.get< double >("compare.eps_packingfactor");
        const double eps = std::max(1., packing_factor) * packingError;
        eckit::Log::debug() << "Compare: |ε| = max(1, " << packing_factor << ")*ε|packing = " << eps << std::endl;
        return new util::compare::CompareValuesAbsoluteToleranceFn< double >(eps, missingValue1, missingValue2);

    }
    else if (compareMode=="absolute") {

        const double eps = std::max(packingError, options_.get< double >("compare.eps_absolute"));
        eckit::Log::debug() << "Compare: |ε| =  max(ε|absolute, ε|packing) = " << eps << std::endl;
        return new util::compare::CompareValuesAbsoluteToleranceFn< double >(eps, missingValue1, missingValue2);

    }
    else if (compareMode=="relative") {

        const double eps = options_.get< double >("compare.eps_relative");
        eckit::Log::debug() << "Compare: |ε| =  ε|relative = " << eps << std::endl;
        return new util::compare::CompareValuesRelativeToleranceFn< double >(eps, missingValue1, missingValue2);

    }
    throw eckit::SeriousBug("Compare::getCompareFunctor: mode is not one of (\"packing\"|\"absolute\"|\"relative\")");
    return NULL;
}


CompareValues::CompareValuesFn* CompareValues::getCompareAnglesFunctor(double missingValue1, double missingValue2, double packingError) const {
    const double eps = std::max(packingError, options_.get< double >("compare.eps_angle"));
    eckit::Log::debug() << "Compare: |ε| =  max(ε|angle, ε|packing) = " << eps << "°" << std::endl;
    return new util::compare::CompareValuesAbsoluteToleranceFn< double >(eps, missingValue1, missingValue2);
}


bool CompareValues::compareFieldsValuesScalar(
        const data::MIRField& field1, const data::MIRField& field2,
        const CompareOptions& options, const double& packingError ) const {

    bool verbose = options.get< bool >("compare.verbose");
    const double missingValue1 = field1.hasMissing()? field1.missingValue() : std::numeric_limits<double>::quiet_NaN();
    const double missingValue2 = field2.hasMissing()? field2.missingValue() : std::numeric_limits<double>::quiet_NaN();
    const size_t dim1 = field1.dimensions();
    const size_t dim2 = field2.dimensions();

    // set comparison functor
    eckit::ScopedPtr< CompareValuesFn > compare_fn(getCompareFunctor(missingValue1, missingValue2, packingError));
    ASSERT(compare_fn);

    size_t w = 0;
    bool cmp = true;
    while (cmp && w<dim1 && w<dim2) {

        // access field values
        const std::vector<double>& values1 = field1.values(w);
        const std::vector<double>& values2 = field2.values(w);
        ++w;

        // set maximum allowable difference count (Ndiffmax)
        const size_t N = std::min(values1.size(), values2.size());
        const size_t Ndiffmax = getNMaxDifferences(N, options);
        eckit::Log::debug() << "\tN = min(#field A, #field B) = min(" << eckit::BigNum(values1.size()) << ", " << eckit::BigNum(values1.size()) << ")\n"
                               "\tNdiffmax = " << Ndiffmax << std::endl;

        cmp = (values1.size() == values2.size());
        if (!cmp || verbose) {
            eckit::Log::info() << "\tfield A count: " << eckit::Plural(values1.size(), "value") << "\n"
                                  "\tfield B count: " << eckit::Plural(values2.size(), "value") << std::endl;
        }
        else if (cmp) {

            // perform value comparison
            ReportDifferencesOnFieldsScalar calc(*compare_fn, missingValue1, missingValue2);
            for (size_t i=0; i<N; ++i) {
                calc(values1[i], values2[i]);
            }

            // set comparison results
            size_t Ndiff = calc.countDifferences();
            cmp = (Ndiff <= Ndiffmax);
            if (!cmp || verbose) {
                double p = double(Ndiff) / double(N) * 100;
                eckit::Log::info() << "\t" << Ndiff << " out of " << eckit::Plural(N, "value") << (Ndiff!=1 ? " are " : " is ") << "different (" << p << "%)\n"
                                      "\tε|max = |field A - field B|[" << calc.maxIndex() << "] = " << calc.max() << "\n"
                                      "\t" << calc.results() << std::endl;
            }

        }
    }

    cmp = cmp && (dim1==dim2);
    if (dim1!=dim2 || verbose) {
        eckit::Log::info() << "\tfield A: " << eckit::Plural(dim1, "dimension") <<  "\n"
                              "\tfield B: " << eckit::Plural(dim2, "dimension") << std::endl;
    }
    return cmp;
}


bool CompareValues::compareFieldsValuesAngleDegrees(
        const data::MIRField& field1, const data::MIRField& field2,
        const CompareValues::CompareOptions& options, const double& packingError ) const {

    bool verbose = options.get< bool >("compare.verbose");
    const double missingValue1 = field1.hasMissing()? field1.missingValue() : std::numeric_limits<double>::quiet_NaN();
    const double missingValue2 = field2.hasMissing()? field2.missingValue() : std::numeric_limits<double>::quiet_NaN();
    const size_t dim1 = field1.dimensions();
    const size_t dim2 = field2.dimensions();

    // set comparison functor
    eckit::ScopedPtr< CompareValuesFn > compare_fn(getCompareAnglesFunctor(missingValue1, missingValue2, packingError));
    ASSERT(compare_fn);

    size_t w = 0;
    bool cmp = true;
    while (cmp && w<dim1 && w<dim2) {

        // access field values
        const std::vector<double>& values1 = field1.values(w);
        const std::vector<double>& values2 = field2.values(w);
        ++w;

        // set maximum allowable difference count (Ndiffmax)
        const size_t N = std::min(values1.size(), values2.size());
        const size_t Ndiffmax = getNMaxDifferences(N, options);
        eckit::Log::debug() << "\tN = min(#field A, #field B) = min(" << eckit::BigNum(values1.size()) << ", " << eckit::BigNum(values1.size()) << ")\n"
                               "\tNdiffmax = " << Ndiffmax << std::endl;

        cmp = (values1.size() == values2.size());
        if (!cmp || verbose) {
            eckit::Log::info() << "\tfield A count: " << eckit::Plural(values1.size(), "value") << "\n"
                                  "\tfield B count: " << eckit::Plural(values2.size(), "value") << std::endl;
        }
        else if (cmp) {

            // perform value comparison
            ReportDifferencesOnFieldsAngleDegrees calc(*compare_fn, missingValue1, missingValue2);
            for (size_t i=0; i<N; ++i) {
                calc(values1[i], values2[i]);
            }

            // set comparison results
            size_t Ndiff = calc.countDifferences();
            cmp = (Ndiff <= Ndiffmax);
            if (!cmp || verbose) {
                double p = double(Ndiff) / double(N) * 100;
                eckit::Log::info() << "\t" << Ndiff << " out of " << eckit::Plural(N, "value") << (Ndiff!=1 ? " are " : " is ") << "different (" << p << "%)\n"
                                      "\tpolar Θ: ε|max = |field A - field B|[" << calc.maxIndex() << "] = " << calc.max() << "°\n"
                                      "\tpolar Θ: " << calc.results() << std::endl;
            }

        }
    }

    cmp = cmp && (dim1==dim2);
    if (dim1!=dim2 || verbose) {
        eckit::Log::info() << "\tfield A: " << eckit::Plural(dim1, "dimension") <<  "\n"
                              "\tfield B: " << eckit::Plural(dim2, "dimension") << std::endl;
    }
    return cmp;
}


bool CompareValues::compareFieldsValuesVectorCartesian2D(
        const data::MIRField& field1, const data::MIRField& field2,
        const CompareOptions& options, const double& packingError ) const {

    const double missingValue1 = field1.hasMissing()? field1.missingValue() : std::numeric_limits<double>::quiet_NaN();
    const double missingValue2 = field2.hasMissing()? field2.missingValue() : std::numeric_limits<double>::quiet_NaN();

    // set comparison functors
    eckit::ScopedPtr< CompareValuesFn > compare_mag_fn(getCompareFunctor      (missingValue1, missingValue2, packingError));
    eckit::ScopedPtr< CompareValuesFn > compare_ang_fn(getCompareAnglesFunctor(missingValue1, missingValue2, packingError));
    ASSERT(compare_mag_fn);
    ASSERT(compare_ang_fn);


    // access field values
    ASSERT(field1.dimensions()==2);
    ASSERT(field2.dimensions()==2);
    const std::vector<double>
            &values1x = field1.values(0), &values1y = field1.values(1),
            &values2x = field2.values(0), &values2y = field2.values(1);
    ASSERT(values1x.size() == values1y.size());
    ASSERT(values2x.size() == values2y.size());


    // set maximum allowable difference count (Ndiffmax)
    const size_t N = std::min(values1x.size(), values2x.size());
    const size_t Ndiffmax = getNMaxDifferences(N, options);
    eckit::Log::debug() << "\tN = min(#field A, #field B) = min(" << eckit::BigNum(values1x.size()) << ", " << eckit::BigNum(values1x.size()) << ")\n"
                           "\tNdiffmax = " << Ndiffmax << std::endl;


    // perform value comparison
    ReportDifferencesOnFieldsScalar       compareMag(*compare_mag_fn, missingValue1, missingValue2);
    ReportDifferencesOnFieldsAngleDegrees compareAng(*compare_ang_fn, missingValue1, missingValue2);
    util::angles::ConvertVectorCartesian2dToPolarDegrees convert1(missingValue1);
    util::angles::ConvertVectorCartesian2dToPolarDegrees convert2(missingValue2);
    for (size_t i=0; i<N; ++i) {
        compareMag(convert1.r    (values1x[i], values1y[i]), convert2.r    (values2x[i], values2y[i]));
        compareAng(convert1.theta(values1x[i], values1y[i]), convert2.theta(values2x[i], values2y[i]));
    }


    // set comparison results
    size_t Ndiff = std::max(compareMag.countDifferences(), compareAng.countDifferences());
    const bool cmp = (Ndiff <= Ndiffmax);
    if (!cmp || options.get< bool >("compare.verbose")) {
        double p = double(Ndiff) / double(N) * 100;
        eckit::Log::info() << "\t" << Ndiff << " out of " << eckit::Plural(N, "value") << (Ndiff!=1 ? " are " : " is ") << "different (" << p << "%)\n"
                              "\tpolar r: ε|max = |field A - field B|[" << compareMag.maxIndex() << "] = " << compareMag.max() << "\n"
                              "\tpolar Θ: ε|max = |field A - field B|[" << compareAng.maxIndex() << "] = " << compareAng.max() << "°" << std::endl;
    }
    return cmp;
}


bool CompareValues::compare(
        const data::MIRField& field1, const param::MIRParametrisation& param1,
        const data::MIRField& field2, const param::MIRParametrisation& param2 ) const {

    double d;  // dummy
    const double packingError = std::min(
                (param1.get("packingError", d)? d : 0.),
                (param2.get("packingError", d)? d : 0.) );

    if ( options_.get<bool>("compare.compare_vector_in_polar") &&
              field_is_vector_cartedian2d(field1) &&
              field_is_vector_cartedian2d(field2) ) {

        return compareFieldsValuesVectorCartesian2D(field1, field2, options_, packingError);

    }
    else if ( options_.get<bool>("compare.compare_angle_in_polar") &&
              field_is_angle_degrees(field1) &&
              field_is_angle_degrees(field2) ) {

        return compareFieldsValuesAngleDegrees(field1, field2, options_, packingError);

    }

    return compareFieldsValuesScalar(field1, field2, options_, packingError);
}


namespace {
static ComparisonBuilder<CompareValues> compare("values");
}


}  // namespace compare
}  // namespace action
}  // namespace mir

