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
#include "eckit/log/BigNum.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/MIRArgs.h"
#include "mir/param/option/SimpleOption.h"

using mir::param::option::Option;
using mir::param::option::SimpleOption;



class MIRStats : public eckit::Tool {

    virtual void run();

    static void usage(const std::string &tool);


  public:
    MIRStats(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }



};

void MIRStats::usage(const std::string &tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " file.grib" << std::endl
            ;

    ::exit(1);
}



void MIRStats::run() {

    using eckit::FloatApproxCompare;

    std::vector<Option *> options;


    //==============================================
    options.push_back(new SimpleOption<size_t>("buckets", "Bucket count for computing entropy (default 65536)"));
    options.push_back(new SimpleOption<size_t>("bits", "Bucket count (as 2^bits) for computing entropy (default 16)"));

    // options.push_back(new SimpleOption<double>("relative", "Maximum relative error"));
    // options.push_back(new SimpleOption<double>("percent", "Maximum percentage of different values"));
    // options.push_back(new SimpleOption<bool>("ulps", "Comparing with ULPS (?)"));

    mir::param::MIRArgs args(&usage, 1, options);



    mir::input::GribFileInput file(args.args(0));

    mir::input::MIRInput &input = file;

    const mir::param::MIRParametrisation &metadata = input.parametrisation();
    long bucket_count = 65536;
    args.get("buckets", bucket_count);

    long bits = 0;
    if (args.get("bits", bits)) {
        bucket_count = 1L << bits;
    }

    eckit::Log::info() << "Number of buckets: " << bucket_count << std::endl;

    size_t n = 0;
    while ( file.next() ) {
        ++n;

        eckit::ScopedPtr<mir::data::MIRField> field(input.field());

        const std::vector<double>& values = field->values(0);
        bool hasMissing = field->hasMissing();
        double missingValue = field->missingValue();
        double minvalue = 0;
        double maxvalue = 0;

        size_t missing = 0;

        size_t first   = 0;
        for (; first < values.size(); ++first) {
            if (!hasMissing || values[first] != missingValue) {
                minvalue = values[first];
                maxvalue = values[first];
                break;
            } else {
                missing++;
            }
        }

        if (first == values.size()) {
            eckit::Log::info() << "Field " << n << " has only missing values" << std::endl;
            continue;
        }

        size_t count = 0;
        for (size_t i = first; i < values.size(); ++i) {
            if (!hasMissing || values[i] != missingValue) {
                minvalue = std::min(minvalue, values[i]);
                maxvalue = std::max(maxvalue, values[i]);
                count++;
            } else {
                missing++;
            }
        }

        eckit::Log::info() << "Field " << eckit::BigNum(n)
                           << ", min=" << minvalue
                           << ", max=" << maxvalue
                           << ", count=" << eckit::BigNum(count)
                           << ", missing=" << eckit::BigNum(missing)
                           << std::endl;


        // Compute entropy
        std::vector<size_t> buckets(bucket_count, 0);
        double scale = (bucket_count - 1)  / (maxvalue - minvalue);
        for (size_t i = 0; i < values.size(); ++i) {
            if (!hasMissing || values[i] != missingValue) {
                size_t b = (values[i] - minvalue) * scale;
                ASSERT(b < bucket_count);
                buckets[b]++;
            }
        }

        double e = 0;
        double one_over_log2 = 1 / log(2);
        for (size_t i = 0; i < buckets.size(); ++i) {
            double p = double(buckets[i]) / double(count);
            if (p) {
                e += -p * log(p) * one_over_log2;
            }
        }

        std::cout << "Entropy " << e << std::endl;

    }



}


int main( int argc, char **argv ) {
    MIRStats tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    return 0;
#else
    return tool.start();
#endif
}

