/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Apr 2015


//#include <cmath>
//#include "eckit/log/Plural.h"
//#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/runtime/Tool.h"
//#include "eckit/types/FloatCompare.h"
//#include "mir/action/context/Context.h"
//#include "mir/data/MIRField.h"
//#include "mir/input/GribFileInput.h"
//#include "mir/param/SimpleParametrisation.h"
//#include "mir/util/MIRStatistics.h"


using eckit::option::Option;
using eckit::option::SimpleOption;


class MIRCompare : public eckit::Tool {

    virtual void run();

    static void usage(const std::string &tool);

#if 0
    void compare(size_t n, mir::data::MIRField &field1, mir::data::MIRField &field2) const;
    void l2norm(size_t n, mir::data::MIRField &field1, mir::data::MIRField &field2) const;

    bool compare( double,  double) const;
    bool compare(const double *, const double *, size_t) const;
#endif

  public:
    MIRCompare(int argc, char **argv) :
        eckit::Tool(argc, argv) //,
#if 0
        user_absolute_(1e-9),
        user_relative_(1e-9),
        user_percent_(1e-9),
        user_ulps_(0),
        user_pack_factor_(0.)
#endif
        {
    }

#if 0
  private: // members

    double user_absolute_;
    double user_relative_;
    double user_percent_;
    long   user_ulps_;
    double user_pack_factor_;
    bool   l2norm_;

    eckit::ScopedPtr< eckit::FloatApproxCompare<double> > real_same_;
#endif
};


void MIRCompare::usage(const std::string &tool) {
    eckit::Log::info()
            << '\n'
            << "Usage: " << tool << " [options] file1.grib file2.grib" << '\n'
//            << "  [--absolute=a]: Maximum absolute error"                           << '\n'
//            << "  [--relative=r]: Maximum relative error"                           << '\n'
//            << "  [--percent=p]:  Maximum percentage of different values"           << '\n'
//            << "  [--packing=f]:  Comparing to packing error, with provided factor" << '\n'
//            << "  [--ulps=u]:     Comparing with ULPs"                              << '\n'
//            << "  [--l2norm]:     Compute L2 norm between 2 fields"                 << '\n'
            << std::endl;

    ::exit(1);
}


void MIRCompare::run() {
    std::vector<Option *> options;
    eckit::option::CmdArgs args(&usage, options, 2);
}


int main(int argc, char **argv) {
    MIRCompare tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    tool.start();
    return 0;
#else
    return tool.start();
#endif
}


















#if 0
static struct {
    size_t paramId_;
    double absolute_; // -1 means use default
    double relative_; // -1 means use default
} thresholds [] = {
    {0, 0, 0}
};


class MIRCompare : public eckit::Tool {

    virtual void run();

    static void usage(const std::string &tool);

    void compare(size_t n, mir::data::MIRField &field1, mir::data::MIRField &field2) const;
    void l2norm(size_t n, mir::data::MIRField &field1, mir::data::MIRField &field2) const;

    bool compare( double,  double) const;
    bool compare(const double *, const double *, size_t) const;

  public:
    MIRCompare(int argc, char **argv) :
        eckit::Tool(argc, argv),
        user_absolute_(1e-9),
        user_relative_(1e-9),
        user_percent_(1e-9),
        user_ulps_(0),
        user_pack_factor_(0.) {
    }

  private: // members

    double user_absolute_;
    double user_relative_;
    double user_percent_;
    long   user_ulps_;
    double user_pack_factor_;
    bool   l2norm_;

    eckit::ScopedPtr< eckit::FloatApproxCompare<double> > real_same_;

};

void MIRCompare::usage(const std::string &tool) {

    eckit::Log::info()
            << '\n'
            << "Usage: " << tool << " [options] file1.grib file2.grib" << '\n'
            << "  [--absolute=a]: Maximum absolute error"                           << '\n'
            << "  [--relative=r]: Maximum relative error"                           << '\n'
            << "  [--percent=p]:  Maximum percentage of different values"           << '\n'
            << "  [--packing=f]:  Comparing to packing error, with provided factor" << '\n'
            << "  [--ulps=u]:     Comparing with ULPs"                              << '\n'
            << "  [--l2norm]:     Compute L2 norm between 2 fields"                 << '\n'
            << std::endl;

    ::exit(1);
}



double maxAbsoluteError = 1e-9;
double maxRelativeError = 1e-9;
double packing_error1 = 0;
double packing_error2 = 0;

static double err(double A, double B) {
    double relativeError;

    if (fabs(A) <= maxAbsoluteError || fabs(B) <= maxAbsoluteError)
        relativeError = fabs(A - B);
    else if (fabs(B) > fabs(A))
        relativeError = fabs((A - B) / B);
    else
        relativeError = fabs((A - B) / A);

    return relativeError;
}

static bool same(double A, double B) {
    return err(A, B) < maxRelativeError;
}

bool MIRCompare::compare(double a, double b) const {

    if (real_same_)
        return (*real_same_)(a, b);
    else
        return same(a, b);
}

bool MIRCompare::compare(const double *a, const double *b, size_t size) const {

    size_t m = 0;
    size_t count = 0;
    double max = -std::numeric_limits<double>::max();
    for (size_t i = 0; i < size; i++  ) {
        if (!compare(a[i], b[i])) {
            count++;
            if (fabs(a[i] - b[i]) > max) {
                max = fabs(a[i] - b[i]);
                m = i;
            }

        }
    }
    if (count) {
        double p = double(count) / double(size) * 100;
        eckit::Log::info() << eckit::Plural(count, "value")
                           << " out of " << eckit::BigNum(size)
                           << (count > 1 ? " are " : " is ") << "different (" << p << "%)" << std::endl;

        eckit::Log::info() << "Max difference is element " << m + 1 << " v1=" << a[m] << " v2=" << b[m]
                           << " diff=" << fabs(a[m] - b[m]) << " err=" << err(a[m], b[m]) << std::endl;

        eckit::Log::info() << "maxAbsoluteError=" << maxAbsoluteError << " maxRelativeError=" << maxRelativeError << std::endl;
        eckit::Log::info() << "packing_error1=" << packing_error1 << " packing_error2=" << packing_error2 << std::endl;

        if (p <= user_percent_) {
            eckit::Log::info() << "Percent of different values smaller than " << user_percent_ << ", ignoring differences" << std::endl;
            count = 0;
        }

        // << "Value " << i + 1 << " are different: " << a[i]
        //                           << " and " << b[i] << " diff=" << fabs(a[i] - b[i]) << " err=" << err(a[i], b[i]) << std::endl;
        //       return false;
        //   }
    }
    return count == 0;
}

void MIRCompare::compare(size_t n, mir::data::MIRField &field1, mir::data::MIRField &field2) const {

    ASSERT(field1.dimensions() == 1);
    ASSERT(field2.dimensions() == 1);

    if (field1.hasMissing() != field2.hasMissing()) {
        eckit::Log::info() << "Field " << n << ": " << (field1.hasMissing() ? "file 1 has missing values" : "file 1 has not missing values") << " "
                           << (field2.hasMissing() ? "file 2 has missing values" : "file 2 has not missing values") << std::endl;
        ::exit(1);
    }


    if (field1.missingValue() != field2.missingValue()) {
        eckit::Log::info() << "Field " << n << ": missing value mismatch " <<  field1.missingValue()
                           << " and " << field2.missingValue() << std::endl;
        ::exit(1);
    }

    const std::vector<double> &v1 = field1.values(0);
    const std::vector<double> &v2 = field2.values(0);

    if (v1.size() != v2.size()) {
        eckit::Log::info() << "Field " << n << ": values count mismatch " <<  eckit::BigNum(v1.size())
                           << " and " << eckit::BigNum(v2.size()) << std::endl;
        ::exit(1);
    }
    if (!compare(&v1[0], &v2[0], v1.size())) {
        eckit::Log::info() << "Field " << n << " values comparaison failed" << std::endl;
        ::exit(1);
    }
}


void MIRCompare::l2norm(size_t n, mir::data::MIRField &field1, mir::data::MIRField &field2) const {

    ASSERT(field1.dimensions() == 1);
    ASSERT(field2.dimensions() == 1);

    if (field1.hasMissing() != field2.hasMissing()) {
        eckit::Log::info() << "Field " << n << ": " << (field1.hasMissing() ? "file 1 has missing values" : "file 1 has not missing values") << " "
                           << (field2.hasMissing() ? "file 2 has missing values" : "file 2 has not missing values") << std::endl;
        ::exit(1);
    }


    if (field1.missingValue() != field2.missingValue()) {
        eckit::Log::info() << "Field " << n << ": missing value mismatch " <<  field1.missingValue()
                           << " and " << field2.missingValue() << std::endl;
        ::exit(1);
    }

    const std::vector<double> &v1 = field1.values(0);
    const std::vector<double> &v2 = field2.values(0);

    if (v1.size() != v2.size()) {
        eckit::Log::info() << "Field " << n << ": values count mismatch " <<  eckit::BigNum(v1.size())
                           << " and " << eckit::BigNum(v2.size()) << std::endl;
        ::exit(1);
    }

    double norm = 0;
    for(size_t i = 0; i < v1.size(); i++) {
        double a = v1[i] - v2[i];
        norm += a * a;
    }

    std::cout << "L2-norm " << sqrt(norm) << " " << v1.size() << " " << sqrt(norm)/v1.size() << " " << sqrt(norm)/sqrt(v1.size()) << std::endl;
}

void MIRCompare::run() {

    using eckit::FloatApproxCompare;

    std::vector<Option *> options;


    //==============================================
    options.push_back(new SimpleOption< double >("absolute", "Maximum absolute error"));
    options.push_back(new SimpleOption< double >("relative", "Maximum relative error"));
    options.push_back(new SimpleOption< double >("percent",  "Maximum percentage of different values"));
    options.push_back(new SimpleOption< double >("packing",  "Comparing to packing error, with provided factor"));
    options.push_back(new SimpleOption< bool   >("ulps",     "Comparing with ULPS (?)"));
    options.push_back(new SimpleOption< bool   >("l2norm",   "Compute L2 norm between 2 fields"));

    eckit::option::CmdArgs args(&usage, options, 2);

    args.get("absolute", user_absolute_);
    args.get("relative", user_relative_);
    args.get("percent",  user_percent_);
    args.get("l2norm",   l2norm_);
    args.get("packing",  user_pack_factor_);
    const bool compare_with_packing_error(eckit::FloatCompare< double >::isStrictlyGreater(user_pack_factor_, 0.));


    /// TODO Test this code
    args.get("ulps",     user_ulps_);
    if (compare_with_packing_error) {
        eckit::Log::info() << "Comparing with packing error, factor " << user_pack_factor_ << std::endl;
    }
    else if (user_ulps_) {
        eckit::Log::info() << "Comparing with ULPS " << user_ulps_ << std::endl;
        real_same_.reset( new FloatApproxCompare<double>(0, user_ulps_) );
    }

#if 0
    const char* ops[] = { "metadata", "statistics", "differenceNorms", "values", 0 };
    size_t i = 0;
    while (ops[i]) {
        using namespace mir::action::compare;

        mir::param::SimpleParametrisation param;
        param.set("compare.file", args(1));
        if (!strncmp(ops[i], "values", 6)) {
            param.set("compare.mode", "absolute");
        }
        eckit::ScopedPtr< Compare > cmp(ComparisonFactory::build(ops[i++], param));

        mir::util::MIRStatistics statistics;
        mir::input::GribFileInput grib1(args(0));
        mir::context::Context ctx(grib1, statistics);
        cmp->execute(ctx);
    }
#else
    mir::input::GribFileInput file1(args(0));
    mir::input::GribFileInput file2(args(1));

    mir::input::MIRInput &input1 = file1;
    mir::input::MIRInput &input2 = file2;

    const mir::param::MIRParametrisation &metadata1 = input1.parametrisation();
    const mir::param::MIRParametrisation &metadata2 = input2.parametrisation();

    bool ok1 = file1.next();
    bool ok2 = file2.next();

    size_t n = 0;
    while ( ok1 && ok2 ) {

        ++n;

        mir::data::MIRField field1(input1.field());
        mir::data::MIRField field2(input2.field());

        if (l2norm_) {
            l2norm(n, field1, field2);
        } else {

            double absolute = user_absolute_;
            double relative = user_relative_;

            size_t paramId1 = 0;
            ASSERT(metadata1.get("paramId", paramId1));

            size_t paramId2 = 0;
            ASSERT(metadata2.get("paramId", paramId2));

            ASSERT(paramId1 == paramId2);

            std::string name;
            ASSERT(metadata1.get("shortName", name));
            eckit::Log::info() << "Field " << n << ": paramId is " << paramId1 << " (" << name << ")" << std::endl;

            size_t i = 0;
            while (thresholds[i].paramId_) {
                if (thresholds[i].paramId_ == paramId1) {
                    if (thresholds[i].absolute_ >= 0) {
                        absolute = thresholds[i].absolute_;
                    }
                    if (thresholds[i].relative_ >= 0) {
                        relative = thresholds[i].relative_;
                    }
                    eckit::Log::info() << "Field " << n << ": thresholds changed for paramId " << paramId1
                                       << " to absolute=" << absolute << ", relative=" << relative << std::endl;
                    break;
                }
                i++;
            }

            //================================

            packing_error1 = absolute;
            ASSERT(metadata1.get("packingError", packing_error1));

            packing_error2 = absolute;
            ASSERT(metadata2.get("packingError", packing_error2));

            double packing_error = std::min(packing_error1, packing_error2);

            maxAbsoluteError = std::max(absolute, packing_error);
            maxRelativeError = relative;

            if (compare_with_packing_error) {

                /* maxUlps=64 is a big enough number so ULPs comparisons don't matter */
                real_same_.reset( new FloatApproxCompare<double>(user_pack_factor_*packing_error, 64) );

            }
            else if (maxAbsoluteError != absolute) {
                eckit::Log::warning() << "Field " << n << ": packing error " << packing_error
                                      << " is more than requested absolute error " << absolute << std::endl;
                eckit::Log::warning() << "Field " << n << ": using packing error as absolute error" << std::endl;
            }

            compare(n, field1, field2);
        }

        ok1 = file1.next();
        ok2 = file2.next();

    }

    if (ok1 != ok2) {
        eckit::Log::info() << input1 << " has " << (ok1?"more":"less") << " fields than " << input2 << std::endl;
        ::exit(1);
    }
#endif
}


int main( int argc, char **argv ) {
    MIRCompare tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    tool.start();
    return 0;
#else
    return tool.start();
#endif
}
#endif

/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Aug 2016


#ifndef mir_action_compare_Compare_h
#define mir_action_compare_Compare_h

#include <iosfwd>
#include "eckit/exception/Exceptions.h"
#include "mir/action/plan/Action.h"
#include "mir/data/MIRField.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Compare.h"
#include "mir/util/MapKeyValue.h"


namespace mir {
namespace context {
class Context;
}
namespace input {
class MIRInput;
}
}


namespace mir_cmp {


// query of field represents a 2D cartesian vector (FIXME: find better solution)
bool field_is_vector_cartedian2d(const data::MIRField&);


// query of field represents an angle [°] (FIXME: find better solution)
bool field_is_angle_degrees(const data::MIRField&);


/**
 * @brief Compare action performs MIRField's comparisons
 */
class Compare : public Action {
public:

    // -- Types

    /// Compare options type
    typedef util::MapKeyValue CompareOptions;

    /// Compare results type
    typedef util::MapKeyValue CompareResults;

    // -- Exceptions
    // None

    // -- Constructors

    Compare(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~Compare() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    /// @return maximum number of allowed differences to still return comparison success
    static size_t getNMaxDifferences(size_t N, const CompareOptions&);

    /// @returns comparison of field reports
    static bool compareResults(const CompareResults&, const CompareResults&, const CompareOptions&);

    /// Comparison options get value
    /// @returns requested option value (if option is not set returns defaultValue)
    template< typename T >
    T optionGet(const std::string& optionName, const T& defaultValue=T()) const {
        return options_.get<T>(optionName, defaultValue);
    }

    /// Comparison options set value
    template< typename T >
    void optionSet(const std::string& optionName, const T& optionValue) {
        options_.set(optionName, optionValue);
    }

    // -- Overridden methods

    void execute(context::Context&) const;

    bool sameAs(const Action&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    /// Comparison options
    CompareOptions options_;

    // -- Methods

    /// @return if fields compare successfuly
    virtual bool compare(
            const data::MIRField& field1, const param::MIRParametrisation& param1,
            const data::MIRField& field2, const param::MIRParametrisation& param2 ) const = 0;

    /// Comparison options reset
    void optionsReset();

    /// Comparison options set from another parametrisation
    virtual void optionsSetFrom(const param::MIRParametrisation&);

    // -- Overridden methods

    void print(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


class ComparisonFactory {
private:
    std::string name_;
    virtual Compare* make(const param::MIRParametrisation&) = 0;
protected:
    ComparisonFactory(const std::string&);
    virtual ~ComparisonFactory();
public:
    static void list(std::ostream&);
    static Compare* build(const std::string&, const param::MIRParametrisation&);
};


template<class T>
class ComparisonBuilder : public ComparisonFactory {
private:
    Compare* make(const param::MIRParametrisation& param) {
        return new T(param);
    }
public:
    ComparisonBuilder(const std::string& name) : ComparisonFactory(name) {
        // register in the ActionFactory as well (Compare is an Action) under a more descriptive name
        static ActionBuilder<T> actionBuilder("compare." + name);
    }
};


}  // namespace mir_cmp


#endif

/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/compare/Compare.h"

#include <map>
#include <ostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/input/GribFileInput.h"  // TODO better
#include "mir/input/MIRInput.h"


namespace mir_cmp {


namespace {


static eckit::Mutex* local_mutex = 0;
static std::map< std::string, ComparisonFactory* > *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, ComparisonFactory* >();
}


}  // (anonymous namespace)


bool field_is_angle_degrees(const data::MIRField& field) {
    if (field.dimensions()==1) {
        const size_t id = field.paramId(0);
        return (id==140113) ||    // wefxd
               (id==140230);      // mwd
    }
    return false;
}


bool field_is_vector_cartedian2d(const data::MIRField& field) {
    if (field.dimensions()==2) {
        const size_t id1 = field.paramId(0), id2 = field.paramId(1);
        return (id1==131 && id2==132) ||    // u, v
               (id1==165 && id2==166);     // 10u, 10v
    }
    return false;
}


Compare::Compare(const param::MIRParametrisation& parametrisation) :
    action::Action(parametrisation) {
    optionsSetFrom(parametrisation);
}


size_t Compare::getNMaxDifferences(size_t N, const Compare::CompareOptions& options) {
    using util::compare::is_approx_one;
    using util::compare::is_approx_zero;
    const double compareCountFactor = std::max(0., std::min(1.,
        options.get< double >("compare.max_count_percent") * 0.01 ));
    return options.has("compare.max_count_diffs")? std::min(N, options.get< size_t >("compare.max_count_diffs"))
           : is_approx_one (compareCountFactor)? N
           : is_approx_zero(compareCountFactor)? 0
           : size_t(std::ceil(compareCountFactor * N));
}


bool Compare::compareResults(const CompareResults& a, const CompareResults& b, const CompareOptions& options) {
    bool cmp = (a==b);
    if (!cmp || options.get< bool >("compare.verbose")) {
        eckit::Log::info() << "\tfield A: " << a << "\n"
                              "\tfield B: " << b << std::endl;
    }
    return cmp;
}


void Compare::execute(context::Context& ctx) const {
    eckit::Log::info() << "Compare: options: " << options_ << std::endl;


    input::GribFileInput reference(options_.get< std::string >("compare.file"));

    input::MIRInput& input1 = ctx.input();
    input::MIRInput& input2(dynamic_cast< input::MIRInput& >( reference ));


    bool next1 = input1.next();
    bool next2 = input2.next();
    size_t count1 = next1? 1 : 0;
    size_t count2 = next2? 1 : 0;

    bool cmp = true;
    while (cmp && next1 && next2) {

        // perform comparison
        cmp = compare(
                    input1.field(), input1.parametrisation(),
                    input2.field(), input2.parametrisation() );

        next1 = input1.next();
        next2 = input2.next();
        if (next1) { ++count1; }
        if (next2) { ++count2; }
    }
    cmp = cmp && (count1==count2);

    const size_t countMax = options_.get<size_t>("compare.max_count_fields");
    while (input1.next() && (count1<countMax)) { ++count1; }
    while (input2.next() && (count2<countMax)) { ++count2; }


    if (count1!=count2 || options_.get< bool >("compare.verbose")) {
        eckit::Log::info() << "\tinput A: " <<  eckit::Plural(count1, "field") << (count1>=countMax? " (possibly more)":"") << "\n"
                              "\tinput B: " <<  eckit::Plural(count2, "field") << (count2>=countMax? " (possibly more)":"") << std::endl;
    }


    eckit::Log::info() << "Compare: input A " << (cmp? "==":"!=") << " input B." << std::endl;
}


bool Compare::sameAs(const action::Action& other) const {
    const Compare* o = dynamic_cast<const Compare*>(&other);
    return (o && options_ == o->options_);
}


void Compare::optionsReset() {
    param::SimpleParametrisation empty;
    optionsSetFrom(empty);
}


void Compare::optionsSetFrom(const param::MIRParametrisation& p) {
    options_.setFrom<std::string> (p, "compare.file",                    "");
    options_.setFrom<double>      (p, "compare.max_count_percent",       0);
    options_.setFrom<size_t>      (p, "compare.max_count_diffs",         0);
    options_.setFrom<size_t>      (p, "compare.max_count_fields",        1000);
    options_.setFrom<bool>        (p, "compare.compare_angle_in_polar",  true);
    options_.setFrom<bool>        (p, "compare.compare_vector_in_polar", true);
    options_.setFrom<bool>        (p, "compare.verbose",                 true);
}


void Compare::print(std::ostream& out) const {
    out << "Compare["
        <<  "options[" << options_ << "]"
        << "]";
}


ComparisonFactory::ComparisonFactory(const std::string& name) :
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if(m->find(name) != m->end()) {
        throw eckit::SeriousBug("ActionFactory: duplication action: " + name);
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


ComparisonFactory::~ComparisonFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}


void ComparisonFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map< std::string, ComparisonFactory* >::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


Compare* ComparisonFactory::build(const std::string& name, const param::MIRParametrisation& params) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    eckit::Log::debug<LibMir>() << "Looking for ActionFactory [" << name << "]" << std::endl;

    std::map< std::string, ComparisonFactory* >::const_iterator j = m->find(name);
    if (j == m->end()) {
        eckit::Log::error() << "No ComparisonFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "ComparisonFactories are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No ComparisonFactory called ") + name);
    }

    return (*j).second->make(params);
}


}  // namespace mir_cmp


/// Compare two values (possibly missing) with absolute tolerance
template< typename T >
struct CompareValuesAbsoluteToleranceFn : ACompareBinFn<T> {
    CompareValuesAbsoluteToleranceFn( T epsilon,
            const double& missingValue1=std::numeric_limits<double>::quiet_NaN(),
            const double& missingValue2=std::numeric_limits<double>::quiet_NaN() ) :
        miss1_(missingValue1),
        miss2_(missingValue2),
        eps_(epsilon) {
        ASSERT(eps_>=0);
    }
    bool operator()(const T& v1, const T& v2) {
        if (miss1_(v1) || miss2_(v2))
            return (miss1_(v1) && miss2_(v2));
        return eckit::FloatCompare<T>::isApproximatelyEqual(v1, v2, eps_, 64);  // 64 is chosen so ULPs comparisons don't matter
    }
    IsMissingFn miss1_, miss2_;
    const T eps_;
};


/// Compare two values (possibly missing) with relative tolerance
template< typename T >
struct CompareValuesRelativeToleranceFn : ACompareBinFn<T> {
    CompareValuesRelativeToleranceFn( T epsilon,
            const double& missingValue1=std::numeric_limits<double>::quiet_NaN(),
            const double& missingValue2=std::numeric_limits<double>::quiet_NaN() ) :
        miss1_(missingValue1),
        miss2_(missingValue2),
        eps_(epsilon) {
        ASSERT(eps_>=0.);
    }
    bool operator()(const T& v1, const T& v2) {
        if (miss1_(v1) || miss2_(v2))
            return (miss1_(v1) && miss2_(v2));
        const T dx = std::abs(v1 - v2);
        const T x = std::max(std::numeric_limits<T>::epsilon(), std::max(std::abs(v1), std::abs(v2)));
        return (dx/x <= eps_);
    }
    IsMissingFn miss1_, miss2_;
    const T eps_;
};


