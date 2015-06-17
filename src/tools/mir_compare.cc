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

#include <cmath>

#include "eckit/log/Plural.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/runtime/Tool.h"
#include "eckit/types/FloatCompare.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/MIRArgs.h"
#include "mir/param/option/SimpleOption.h"

using mir::param::option::Option;
using mir::param::option::SimpleOption;

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

    bool compare( double,  double) const;
    bool compare(const double *, const double *, size_t) const;

  public:
    MIRCompare(int argc, char **argv) :
        eckit::Tool(argc, argv),
        user_absolute_(1e-9),
        user_relative_(1e-9),
        user_percent_(1e-9),
        user_ulps_(0) {
    }

  private: // members

    double user_absolute_;
    double user_relative_;
    double user_percent_;
    long   user_ulps_;

    eckit::ScopedPtr< eckit::FloatApproxCompare<double> > real_same_;

};

void MIRCompare::usage(const std::string &tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " [--absolute=a] [--relative=r] [--ulps=u] [--percent=p] file1.grib file2.grib" << std::endl
            ;

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

    if(real_same_)
        return (*real_same_)(a,b);
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

        if(p <= user_percent_) {
            eckit::Log::info() << "Percent of different valus smaller than " << user_percent_ << ", ignoring differences" << std::endl;
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

void MIRCompare::run() {

    using eckit::FloatApproxCompare;

    std::vector<const Option *> options;


    //==============================================
    options.push_back(new SimpleOption<double>("absolute", "Maximum absolute error"));
    options.push_back(new SimpleOption<double>("relative", "Maximum relative error"));
    options.push_back(new SimpleOption<double>("percent", "Maximum percentage of different values"));
    options.push_back(new SimpleOption<bool>("ulps", "Comparing with ULPS (?)"));

    mir::param::MIRArgs args(&usage, 2, options);

    args.get("absolute", user_absolute_);
    args.get("relative", user_relative_);
    args.get("percent", user_percent_);


    /// TODO Test this code
    args.get("ulps",     user_ulps_);
    if(user_ulps_) {
        eckit::Log::info() << "Comparing with ULPS " << user_ulps_ << std::endl;
        real_same_.reset( new FloatApproxCompare<double>(0,user_ulps_) );
    }

    mir::input::GribFileInput file1(args.args(0));
    mir::input::GribFileInput file2(args.args(1));

    mir::input::MIRInput &input1 = file1;
    mir::input::MIRInput &input2 = file2;

    const mir::param::MIRParametrisation &metadata1 = input1.parametrisation();
    const mir::param::MIRParametrisation &metadata2 = input2.parametrisation();

    bool ok1 = file1.next();
    bool ok2 = file2.next();

    size_t n = 0;
    while ( ok1 && ok2 ) {

        ++n;

        eckit::ScopedPtr<mir::data::MIRField> field1(input1.field());
        eckit::ScopedPtr<mir::data::MIRField> field2(input2.field());

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

        if (maxAbsoluteError != absolute) {
            eckit::Log::warning() << "Field " << n << ": packing error " << packing_error
                                  << " is more than requested absolute error " << absolute << std::endl;
            eckit::Log::warning() << "Field " << n << ": using packing error as absolute error" << std::endl;
        }


        compare(n, *field1, *field2);

        ok1 = file1.next();
        ok2 = file2.next();

    }

    if (ok1 != ok2) {
        if (ok1)  {
            eckit::Log::info() << input1 << " has more fields than " << input2 << std::endl;
            ::exit(1);
        }
        if (ok2)  {
            eckit::Log::info() << input2 << " has more fields than " << input1 << std::endl;
            ::exit(1);
        }
    }

}


int main( int argc, char **argv ) {
    MIRCompare tool(argc, argv);
    tool.start();
    return 0;
}

