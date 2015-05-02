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


#include "eckit/runtime/Tool.h"
#include "eckit/runtime/Context.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/log/BigNum.h"
#include "eckit/log/Plural.h"

#include <cmath>

#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/data/MIRField.h"


class MIRCompare : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool) const;
    void compare(size_t n, mir::data::MIRField &field1, mir::data::MIRField &field2) const;

    static bool compare( double,  double);
    static bool compare(const double *, const double *, size_t);

  public:
    MIRCompare(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRCompare::usage(const std::string &tool) const {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " file1.grib file2.grib" << std::endl
            // << std::endl << "Examples: " << std::endl
            // << "% " << tool << " grid=2/2 area=90/-8/12/80 input.grib output.grib" << std::endl
            // << "% " << tool << " reduced=80 input.grib output.grib" << std::endl << std::endl
            // << "% " << tool << " regular=80 input.grib output.grib" << std::endl << std::endl
            // << "% " << tool << " truncation=63 input.grib output.grib" << std::endl << std::endl
            // << "Option are:" << std::endl
            // << "===========" << std::endl << std::endl

            // << "   accuracy=n" << std::endl
            // << "   area=north/west/south/east" << std::endl
            // << "   autoresol=0/1" << std::endl
            // << "   bitmap=path" << std::endl
            // << "   epsilon=e" << std::endl
            // << "   frame=n" << std::endl
            // << "   grid=west_east/north_south" << std::endl
            // << "   intermediate_gaussian=N" << std::endl
            // << "   interpolation=method (e.g. bilinear)" << std::endl
            // << "   nclosest=n (e.g. for k-nearest)" << std::endl
            // << "   octahedral=N" << std::endl
            // << "   reduced=N" << std::endl
            // << "   regular=N" << std::endl
            // << "   truncation=T" << std::endl

            ;

    ::exit(1);
}



// double maxAbsoluteError = 1e-6;
// double maxRelativeError = 1e-6;
double maxAbsoluteError = 1e-3;
double maxRelativeError = 1e-3;
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

bool MIRCompare::compare(double a, double b) {
    return same(a, b);
}

bool MIRCompare::compare(const double *a, const double *b, size_t size) {

    size_t m = 0;
    size_t count = 0;
    double max = std::numeric_limits<double>::max();
    for (size_t i = 0; i < size; i++  ) {
        if (!compare(a[i], b[i])) {
            count++;
            if (fabs(a[i] - b[i]) < max) {
                max = fabs(a[i] - b[i]);
                m = i;
            }

        }
    }
    if (count) {
        double p = double(count) / double(size) * 100;
        eckit::Log::error() << eckit::Plural(count, "value")
                            << " out of " << eckit::BigNum(size)
                            << (count > 1 ? " are " : " is ") << "different (" << p << "%)" << std::endl;

        eckit::Log::error() << "Max difference is element " << m + 1 << " v1=" << a[m] << " v2=" << b[m]
                            << " diff=" << fabs(a[m] - b[m]) << " err=" << err(a[m], b[m]) << std::endl;

        eckit::Log::error() << "maxAbsoluteError=" << maxAbsoluteError << " maxRelativeError=" << maxRelativeError << std::endl;
        eckit::Log::error() << "packing_error1=" << packing_error1 << " packing_error2=" << packing_error2 << std::endl;

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
        eckit::Log::error() << "Field " << n << ": " << (field1.hasMissing() ? "file 1 has missing values" : "file 1 has not missing values") << " "
                            << (field2.hasMissing() ? "file 2 has missing values" : "file 2 has not missing values") << std::endl;
        ::exit(1);
    }


    if (field1.missingValue() != field2.missingValue()) {
        eckit::Log::error() << "Field " << n << ": missing value mismatch" <<  field1.missingValue() << " and " << field2.missingValue() << std::endl;
        ::exit(1);
    }

    const std::vector<double> &v1 = field1.values(0);
    const std::vector<double> &v2 = field2.values(0);

    if (v1.size() != v2.size()) {
        eckit::Log::error() << "Field " << n << ": values count mismatch" <<  v1.size() << " and " << v2.size() << std::endl;
        ::exit(1);
    }
    if (!compare(&v1[0], &v2[0], v1.size())) {
        eckit::Log::error() << "Field " << n << " values comparaison failed" << std::endl;
        ::exit(1);
    }
}

void MIRCompare::run() {

    eckit::Context &ctx = eckit::Context::instance();
    const std::string &tool = ctx.runName();
    size_t argc = ctx.argc();

    if (argc != 3) {
        usage(tool);
    }

    // std::cout << std::numeric_limits<float>::min() << std::endl;

    mir::api::MIRJob job;
    mir::input::GribFileInput file1(ctx.argv(argc - 2));
    mir::input::GribFileInput file2(ctx.argv(argc - 1));

    mir::input::MIRInput &input1 = file1;
    mir::input::MIRInput &input2 = file2;

    const mir::param::MIRParametrisation &metadata1 = input1.parametrisation();
    const mir::param::MIRParametrisation &metadata2 = input2.parametrisation();

    bool ok1 = file1.next();
    bool ok2 = file2.next();



    size_t n = 0;
    while ( ok1 && ok2 ) {

        std::auto_ptr<mir::data::MIRField> field1(input1.field());
        std::auto_ptr<mir::data::MIRField> field2(input2.field());

        // ASSERT(metadata1.get("packingError", packing_error1));
        // ASSERT(metadata2.get("packingError", packing_error2));

        // ASSERT(packing_error1 == packing_error2);



        compare(++n, *field1, *field2);

        ok1 = file1.next();
        ok2 = file2.next();

    }

    if (ok1 != ok2) {
        if (ok1)  {
            eckit::Log::error() << input1 << " has more fields than " << input2 << std::endl;
            ::exit(1);
        }
        if (ok2)  {
            eckit::Log::error() << input2 << " has more fields than " << input1 << std::endl;
            ::exit(1);
        }
    }

}


int main( int argc, char **argv ) {
    MIRCompare tool(argc, argv);
    tool.start();
    return 0;
}

