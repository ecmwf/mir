/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/compare/FieldComparator.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>

#include "eckit/io/Buffer.h"
#include "eckit/io/StdFile.h"
#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/StringTools.h"

#include "mir/caching/InMemoryCache.h"
#include "mir/compare/BufrField.h"
#include "mir/compare/Field.h"
#include "mir/compare/FieldSet.h"
#include "mir/compare/GribField.h"
#include "mir/compare/MultiFile.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/stats/Comparator.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir::compare {


constexpr long bufferSize      = 5L * 1024 * 1024 * 1024;
constexpr size_t cacheCapacity = 256;

// TODO: change name of env. variable
static caching::InMemoryCache<eckit::AutoStdFile> cache_("files", cacheCapacity, 0, "PGEN_COMPARE_FILE_CACHE");


WhiteLister::~WhiteLister() = default;


DefaultWhiteLister::~DefaultWhiteLister() = default;


const WhiteLister& DefaultWhiteLister::instance() {
    static DefaultWhiteLister i;
    return i;
}


void FieldComparator::addOptions(std::vector<eckit::option::Option*>& options) {
    using eckit::option::FactoryOption;
    using eckit::option::Separator;
    using eckit::option::SimpleOption;

    options.push_back(new SimpleOption<size_t>("maximum-number-of-errors", "Maximum number of errors per task"));
    options.push_back(new SimpleOption<bool>("save-fields", "Save fields that do not compare"));
    options.push_back(new SimpleOption<bool>("save-all-fields", "Save all fields into *.old and *.new files"));

    options.push_back(
        new SimpleOption<bool>("file-names-only", "Only check that the list of files created are the same"));

    options.push_back(new SimpleOption<bool>("list-file-names",
                                             "Create two files with extension '.list' containing the files names"));

    options.push_back(new SimpleOption<bool>("ignore-exceptions", "Ignore exceptions"));
    options.push_back(new SimpleOption<bool>("ignore-count-mismatches", "Ignore field count mismatches"));
    options.push_back(new SimpleOption<bool>("ignore-values-mismatches", "Ignore field value comparison mismatches"));
    options.push_back(new SimpleOption<bool>("ignore-fields-not-found", "Ignore fields not found"));
    options.push_back(new SimpleOption<bool>("ignore-duplicates", "Ignore duplicate fields"));

    options.push_back(
        new SimpleOption<std::string>("ignore", "/-separated list of request keys to ignore when comparing fields"));

    options.push_back(new SimpleOption<bool>("ignore-wrapping-areas",
                                             "Ignore fields with an area that wraps around the globe (e.g. 0-360)"));

    Field::addOptions(options);

    options.push_back(new Separator("Field values"));

    options.push_back(new SimpleOption<bool>("compare-values", "Compare field values (GRIB only)"));
    options.push_back(new SimpleOption<bool>("compare-missing-values", "Compare field bitmap (GRIB only)"));
    options.push_back(new SimpleOption<bool>("compare-statistics", "Compare field statistics (GRIB only)"));
    options.push_back(new SimpleOption<bool>(
        "compare-headers-only", "Compare field headers-only, disables other compare-* options (GRIB only)"));

    options.push_back(
        new SimpleOption<bool>("white-list-entries", "Output lines that can be used in white-list files"));

    options.push_back(
        new SimpleOption<bool>("save-first-possible-match", "Save best match into a file for later analysis"));

    options.push_back(new SimpleOption<bool>("save-duplicates", "Save duplicates into a file for later analysis"));
    options.push_back(new SimpleOption<double>("counter-upper-limit", "Count values below lower limit"));
    options.push_back(new SimpleOption<double>("counter-lower-limit", "Count values above upper limit"));

    options.push_back(
        new SimpleOption<size_t>("ignore-different-missing-values", "Ignore counted different missing values"));

    options.push_back(
        new SimpleOption<double>("ignore-different-missing-values-factor",
                                 "Ignore counted different missing values factor (factor of total count)"));

    options.push_back(new SimpleOption<size_t>("ignore-above-upper-limit", "Ignore count above specified upper limit"));

    options.push_back(new SimpleOption<double>("ignore-above-upper-limit-factor",
                                               "Ignore count above specified upper limit (factor of total count)"));

    options.push_back(new SimpleOption<double>("ignore-above-latitude", "Ignore points values above latitude"));
    options.push_back(new SimpleOption<double>("ignore-below-latitude", "Ignore points values below latitude"));
    options.push_back(new SimpleOption<double>("absolute-error", "Absolute difference error"));
    options.push_back(new SimpleOption<double>("relative-error-factor", "Relative error to maximum difference"));

    options.push_back(
        new SimpleOption<double>("relative-error-min", "Relative difference error to minimum of both fields"));

    options.push_back(
        new SimpleOption<double>("relative-error-max", "Relative difference error to maximum of both fields"));

    options.push_back(
        new SimpleOption<double>("packing-error-factor", "Difference error factor to field packingError"));

    options.push_back(new SimpleOption<double>("spectral-mean-difference-max",
                                               "Maximum difference of spectral mean (spectral fields only)"));

    options.push_back(new SimpleOption<double>("spectral-energy-norm-difference-max",
                                               "Maximum difference of spectral energy norm (spectral fields only)"));

    options.push_back(
        new FactoryOption<stats::ComparatorFactory>("compare", "/-separated list of value comparison methods"));
}


FieldComparator::FieldComparator(const eckit::option::CmdArgs& args, const WhiteLister& whiteLister) :
    fatals_(0),
    warnings_(0),
    args_(args),
    maximumNumberOfErrors_(0),
    saved_(0),
    whiteLister_(whiteLister),
    normaliseLongitudes_(false),
    ignoreWrappingAreas_(false),
    roundDegrees_(false),
    whiteListEntries_(false),
    saveFirstPossibleMatch_(false),
    saveDuplicates_(false) {

    Field::setOptions(args);

    args_.get("normalise-longitudes", normaliseLongitudes_);
    args_.get("maximum-number-of-errors", maximumNumberOfErrors_);
    args_.get("ignore-wrapping-areas", ignoreWrappingAreas_);
    args_.get("white-list-entries", whiteListEntries_);
    args_.get("save-first-possible-match", saveFirstPossibleMatch_);
    args_.get("save-duplicates", saveDuplicates_);


    std::string ignore;
    args_.get("ignore", ignore);
    eckit::Tokenizer parse("/");
    parse(ignore, ignore_);
}


FieldComparator::~FieldComparator() = default;


void FieldComparator::compare(const std::string& name, const MultiFile& multi1, const MultiFile& multi2) {

    bool saveFields = false;
    args_.get("save-fields", saveFields);

    bool saveAllFields = false;
    args_.get("save-all-fields", saveAllFields);

    std::string requirements;
    args_.get("requirements", requirements);

    bool compareValues = true;
    args_.get("compare-values", compareValues);

    bool compareMissingValues = false;
    args_.get("compare-missing-values", compareMissingValues);

    bool compareStatistics = false;
    args_.get("compare-statistics", compareStatistics);

    bool compareHeadersOnly = false;
    args_.get("compare-headers-only", compareHeadersOnly);

    if (compareHeadersOnly) {
        compareValues        = false;
        compareMissingValues = false;
        compareStatistics    = false;
    }

    size_t save = fatals_;

    FieldSet fields1;
    FieldSet fields2;

    if (saveAllFields) {
        multi1.save();
        multi2.save();
    }


    compareCounts(name, multi1, multi2, fields1, fields2);

    compareFields(multi1, multi2, fields1, fields2, compareValues, compareMissingValues, compareStatistics);

    if (fatals_ == save) {
        compareFields(multi2, multi1, fields2, fields1, false, false, compareStatistics);
    }

    if (fatals_ == save) {
        Log::info() << name << " OK." << std::endl;
        return;
    }

    if (!requirements.empty()) {
        /*
        std::string output = name + eckit::PathName(requirements).extension();

        std::ofstream out(output.c_str());
        std::ifstream in(requirements.c_str());

        Log::info() << "Save " << output << std::endl;

        std::string dstream = name.substr(0, 2);
        std::string destination = name.substr(24, 3);

        char line[1024];
        while (in.getline(line, sizeof(line))) {
            if (dstream == std::string(line + 99, line + 101) &&
                    destination == std::string(line + 102, line + 105)) {
                out << line << std::endl;
            }
        }
        */
    }

    if (saveFields) {
        multi1.save();
        multi2.save();
    }
}


void FieldComparator::compare(const std::string& path1, const std::string& path2) {
    MultiFile multi1(path1);
    MultiFile multi2(path2);

    compare("COMPARE", multi1, multi2);
}


void FieldComparator::error(const char* what) {
    bool ignore = false;
    args_.get(std::string("ignore-") + what, ignore);
    if (ignore) {
        warnings_++;
        Log::warning() << "WARNING " << what << std::endl;
    }
    else {
        fatals_++;
        Log::info() << "ERROR " << what << std::endl;
        if (fatals_ > maximumNumberOfErrors_) {
            Log::info() << "Maximum number of errors reached (" << maximumNumberOfErrors_ << ")" << std::endl;
            throw exception::SeriousBug("Maximum number of errors reached");
        }
    }
}


double FieldComparator::normalised(double longitude) const {
    constexpr double globe     = 360.;
    constexpr double reference = 0.;

    if (normaliseLongitudes_) {
        while (longitude < reference) {
            longitude += globe;
        }

        while (longitude >= globe) {
            longitude -= globe;
        }
    }
    return longitude;
}


Field FieldComparator::getField(eckit::Buffer& buffer, const std::string& path, off_t offset, size_t size) {
    if (CODES_SUCCESS == codes_check_message_header(buffer.data(), size, PRODUCT_GRIB) &&
        CODES_SUCCESS == codes_check_message_footer(buffer.data(), size, PRODUCT_GRIB)) {
        return GribField::field(buffer, size, path, offset, ignore_);
    }

    if (CODES_SUCCESS == codes_check_message_header(buffer.data(), size, PRODUCT_BUFR) &&
        CODES_SUCCESS == codes_check_message_footer(buffer.data(), size, PRODUCT_BUFR)) {
        return BufrField::field(buffer, size, path, offset, ignore_);
    }

    throw exception::SeriousBug("No message found (codes_check_message_header|footer(PRODUCT_GRIB|PRODUCT_BUFR))");
}


void FieldComparator::getField(const MultiFile& multi, eckit::Buffer& buffer, FieldSet& fields, const std::string& path,
                               off_t offset, size_t size, bool fail, size_t& duplicates) {


    Field field = getField(buffer, path, offset, size);

    if (fields.duplicate(field) != fields.end()) {
        const auto& other = *fields.duplicate(field);
        Log::info() << "Duplicate field in " << multi << std::endl << "  ==> " << field << std::endl << "  ==> ";
        other.printDifference(Log::info(), field);
        Log::info() << std::endl;
        Log::info() << "This: " << field.path() << ", offset=" << field.offset() << ", length=" << field.length()
                    << std::endl;
        Log::info() << "Prev: " << other.path() << ", offset=" << other.offset() << ", length=" << other.length()
                    << std::endl;

        if (saveDuplicates_) {
            multi.save(field.path(), field.offset(), field.length(), size_t(field.offset()));
            multi.save(other.path(), other.offset(), other.length(), size_t(other.offset()));
        }
        // << "  ==> "
        // << field.compare(other)
        // << std::endl
        if (fail) {
            error("duplicates");
        }
        duplicates++;
    }

    if (whiteLister_.whiteListed(multi, field)) {
        Log::info() << "Field white listed " << field << std::endl;
        return;
    }

    fields.insert(field);
}


size_t FieldComparator::count(const MultiFile& multi, FieldSet& fields) {
    eckit::Buffer buffer(bufferSize);

    fields.clear();
    size_t duplicates = 0;

    for (auto p = multi.paths().begin(); p != multi.paths().end(); ++p) {

        int err;
        off_t pos;
        size_t size = buffer.size();

        eckit::AutoStdFile f(*p);
        while ((err = wmo_read_any_from_file(f, buffer, &size)) != CODES_END_OF_FILE) {


            try {
                GRIB_CALL(err);
                SYSCALL(pos = ::ftello(f));
                getField(multi, buffer, fields, *p, off_t(pos) - off_t(size), size, true, duplicates);
            }
            catch (std::exception& e) {
                Log::info() << "Error in " << *p << " " << e.what() << std::endl;
                error("exceptions");
            }

            size = buffer.size();
        }
    }

    return fields.size() - duplicates;
}


size_t FieldComparator::list(const std::string& path) {
    eckit::Buffer buffer(bufferSize);

    MultiFile multi(path, path);
    FieldSet fields;
    size_t result = 0;

    int err;
    off_t pos;
    size_t size       = buffer.size();
    size_t duplicates = 0;

    eckit::AutoStdFile f(path);
    while ((err = wmo_read_any_from_file(f, buffer, &size)) != CODES_END_OF_FILE) {


        try {
            GRIB_CALL(err);
            SYSCALL(pos = ::ftello(f));
            getField(multi, buffer, fields, path, off_t(pos) - off_t(size), size, false, duplicates);
        }
        catch (std::exception& e) {
            Log::info() << "Error in " << path << " " << e.what() << std::endl;
        }

        result++;
        size = buffer.size();
    }


    for (const auto& field : fields) {
        Log::info() << field << std::endl;
    }

    return result;
}


void FieldComparator::json(eckit::JSON& json, const std::string& path) {
    eckit::Buffer buffer(bufferSize);

    MultiFile multi(path, path);

    int err;
    off_t pos;
    size_t size = buffer.size();

    eckit::AutoStdFile f(path);
    while ((err = wmo_read_any_from_file(f, buffer, &size)) != CODES_END_OF_FILE) {

        GRIB_CALL(err);
        SYSCALL(pos = ::ftello(f));
        Field field = getField(buffer, path, off_t(pos) - off_t(size), size);

        json << field;

        size = buffer.size();
    }
}


static eckit::AutoStdFile& open(const std::string& path) {
    auto j = cache_.find(path);
    if (j == cache_.end()) {
        cache_.insert(path, new eckit::AutoStdFile(path));
        j = cache_.find(path);
    }
    return *j;
}


double relative_error(double a, double b) {
    double mx = std::max(std::abs(a), std::abs(b));

    if (mx == 0) {
        mx = 1;
    }

    return std::abs(a - b) / mx;
}


struct Statistics {
    double min_     = std::numeric_limits<double>::quiet_NaN();
    double max_     = std::numeric_limits<double>::quiet_NaN();
    double average_ = std::numeric_limits<double>::quiet_NaN();

    size_t missing_ = 0;
    size_t values_  = 0;
};


static void getStats(const Field& field, Statistics& stats) {
    eckit::Buffer buffer(bufferSize);

    eckit::AutoStdFile& f = open(field.path());
    size_t size           = buffer.size();
    SYSCALL(fseek(f, field.offset(), SEEK_SET));
    GRIB_CALL(wmo_read_any_from_file(f, buffer, &size));
    ASSERT(size == field.length());

    auto h = codes_handle_new_from_message(nullptr, buffer, size);
    HandleDeleter del(h);

    size_t count;
    GRIB_CALL(codes_get_size(h, "values", &count));

    long missingValuesPresent;
    GRIB_CALL(codes_get_long(h, "missingValuesPresent", &missingValuesPresent));

    double missingValue;
    GRIB_CALL(codes_get_double(h, "missingValue", &missingValue));

    std::vector<double> values(count);

    size = count;
    GRIB_CALL(codes_get_double_array(h, "values", values.data(), &size));
    ASSERT(size == count);
    ASSERT(size);

    stats = Statistics();

    size_t first = 0;
    for (size_t i = 0; i < size; i++) {
        if ((missingValuesPresent != 0) && (values[i] == missingValue)) {
            stats.missing_++;
        }
        else {
            first          = i + 1;
            stats.min_     = values[0];
            stats.max_     = values[0];
            stats.average_ = values[0];
            break;
        }
    }

    if (stats.missing_ == size) {
        // All values missing
        return;
    }

    for (size_t i = first; i < size; i++) {
        if ((missingValuesPresent != 0) && (values[i] == missingValue)) {
            stats.missing_++;
        }
        else {
            stats.min_ = std::min(stats.min_, values[i]);
            stats.max_ = std::max(stats.max_, values[i]);
            stats.average_ += values[i];
            stats.values_++;
        }
    }

    stats.average_ /= double(stats.values_);
}


void FieldComparator::compareFieldStatistics(const MultiFile& multi1, const MultiFile& multi2, const Field& field1,
                                             const Field& field2) {

    caching::InMemoryCacheStatistics ignore;
    caching::InMemoryCacheUser<eckit::AutoStdFile> lock(cache_, ignore);

    Statistics s1;
    getStats(field1, s1);

    Statistics s2;
    getStats(field2, s2);

    constexpr double relativeErrorMax = 0.01;

    if (s1.values_ != s2.values_) {
        Log::info() << "Number of data values mismatch:" << "\n  " << multi1 << ": " << s1.values_ << " " << field1
                    << "\n  " << multi2 << ": " << s2.values_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

    if (s1.missing_ != s2.missing_) {
        Log::info() << "Number of missing values mismatch:" << "\n  " << multi1 << ": " << s1.missing_ << " " << field1
                    << "\n  " << multi2 << ": " << s2.missing_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

    if (relative_error(s1.min_, s2.min_) > relativeErrorMax) {
        Log::info() << "Minimum relative error too large: " << relative_error(s1.min_, s2.min_) << "\n  " << multi1
                    << ": " << s1.min_ << " " << field1 << "\n  " << multi2 << ": " << s2.min_ << " " << field2
                    << std::endl;
        error("statistics-mismatches");
    }

    if (relative_error(s1.max_, s2.max_) > relativeErrorMax) {
        Log::info() << "Maximum relative error too large: " << relative_error(s1.max_, s2.max_) << "\n  " << multi1
                    << ": " << s1.max_ << " " << field1 << "\n  " << multi2 << ": " << s2.max_ << " " << field2
                    << std::endl;
        error("statistics-mismatches");
    }


    if (relative_error(s1.average_, s2.average_) > relativeErrorMax) {
        Log::info() << "Average relative error too large: " << relative_error(s1.average_, s2.average_) << "\n  "
                    << multi1 << ": " << s1.average_ << " " << field1 << "\n  " << multi2 << ": " << s2.average_ << " "
                    << field2 << std::endl;
        error("statistics-mismatches");
    }
}


void FieldComparator::compareFieldValues(const FieldComparator::MultiFile& multi1,
                                         const FieldComparator::MultiFile& multi2, const Field& field1,
                                         const Field& field2) {

    std::unique_ptr<input::MIRInput> input1(new input::GribFileInput(field1.path(), field1.offset()));
    std::unique_ptr<input::MIRInput> input2(new input::GribFileInput(field2.path(), field2.offset()));
    input1->next();
    input2->next();


    // comparison method can be set by classification and/or representation
    const param::ConfigurationWrapper args_wrap(args_);
    static param::DefaultParametrisation defaults;

    param::CombinedParametrisation param1(args_wrap, input1->parametrisation(), defaults);
    param::CombinedParametrisation param2(args_wrap, input2->parametrisation(), defaults);

    auto get_comparison = [](const param::MIRParametrisation& param, const data::MIRField& field) {
        std::string s;
        param.get("compare", s);
        repres::RepresentationHandle repres = field.representation();
        repres->comparison(s);
        return s;
    };

    std::string comparison1 = get_comparison(param1, input1->field());
    std::string comparison2 = get_comparison(param2, input2->field());
    ASSERT(comparison1 == comparison2);
    ASSERT(!comparison1.empty());


    // compare
    for (auto& comparator : eckit::StringTools::split("/", comparison1)) {
        std::unique_ptr<stats::Comparator> comp(stats::ComparatorFactory::build(comparator, param1, param2));
        auto problems = comp->execute(input1->field(), input2->field());

        if (!problems.empty()) {
            Log::info() << "Value compare failed between:" << "\n  " << multi1 << ": " << field1 << "\n  " << multi2
                        << ": " << field2 << "\n  reporting " << *comp << "\n  failed because" << problems << std::endl;
            error("values-mismatches");
        }
    }
}


void FieldComparator::compareFieldMissingValues(const FieldComparator::MultiFile& multi1,
                                                const FieldComparator::MultiFile& multi2, const Field& field1,
                                                const Field& field2) {

    std::unique_ptr<input::MIRInput> input1(new input::GribFileInput(field1.path(), field1.offset()));
    std::unique_ptr<input::MIRInput> input2(new input::GribFileInput(field2.path(), field2.offset()));
    input1->next();
    input2->next();


    const param::ConfigurationWrapper args_wrap(args_);
    static param::DefaultParametrisation defaults;

    param::CombinedParametrisation param1(args_wrap, input1->parametrisation(), defaults);
    param::CombinedParametrisation param2(args_wrap, input2->parametrisation(), defaults);

    std::unique_ptr<stats::Comparator> comp(stats::ComparatorFactory::build("missing-values", param1, param2));
    auto problems = comp->execute(input1->field(), input2->field());

    if (!problems.empty()) {
        Log::info() << "Value compare failed between:" << "\n  " << multi1 << ": " << field1 << "\n  " << multi2 << ": "
                    << field2 << "\n  reporting " << *comp << "\n  failed because" << problems << std::endl;
        error("values-mismatches");
    }
}


void FieldComparator::whiteListEntries(const Field& field, const MultiFile& multi) const {
    multi.whiteListEntries(Log::info());
    Log::info() << ' ';
    field.whiteListEntries(Log::info());
    Log::info() << std::endl;
}


void FieldComparator::missingField(const MultiFile& multi1, const MultiFile& multi2, const Field& field,
                                   const FieldSet& fields, bool& show) {
    constexpr size_t maxWarnings = 5;

    struct Compare {
        const Field& field_;
        Compare(const Field& field) : field_(field) {}
        bool operator()(const Field& a, const Field& b) {
            size_t da = field_.differences(a);
            size_t db = field_.differences(b);
            return da < db;
        }
    };

    if (ignoreWrappingAreas_) {

        if (field.wrapped()) {
            Log::info() << "Ignoring wrapped field " << field << std::endl;
            return;
        }

        std::vector<Field> matches = field.bestMatches(fields);
        std::sort(matches.begin(), matches.end(), Compare(field));
        for (auto& other : matches) {
            if (other.wrapped()) {
                Log::info() << "Ignoring field " << field << " that matches wrapped " << other << std::endl;
                return;
            }
        }
    }

    if (whiteLister_.ignoreError(multi1, field)) {
        return;
    }

    if (show) {
        error("fields-not-found");
        Log::info() << "Fields in " << multi1 << " not in " << multi2 << std::endl;
        show = false;
    }

    Log::info() << "   " << field << std::endl;
    if (whiteListEntries_) {
        whiteListEntries(field, multi1);
    }
    // Find the best mismatches

    std::vector<Field> matches = field.bestMatches(fields);
    if (!matches.empty()) {
        Log::info() << " ? " << "No match found in " << multi2 << std::endl;
        size_t cnt = 0;

        auto flds = field.sortByDifference(fields);


        for (const auto& other : flds) {
            if (other.match(field)) {

                if (cnt >= maxWarnings) {
                    Log::info() << " # ..." << std::endl;
                    break;
                }

                Log::info() << " @ ";
                other.printDifference(Log::info(), field);
                Log::info() << " (";
                other.compareExtra(Log::info(), field);
                Log::info() << ")" << std::endl;

                if (whiteListEntries_) {
                    whiteListEntries(other, multi2);
                }
                cnt++;
            }
        }
        if (cnt == 0) {
            for (const auto& other : flds) {

                if (cnt >= maxWarnings) {
                    Log::info() << " # ..." << std::endl;
                    break;
                }

                Log::info() << " # ";
                other.printDifference(Log::info(), field);
                Log::info() << " (";
                other.compareExtra(Log::info(), field);
                Log::info() << ")" << std::endl;
                if (whiteListEntries_) {
                    whiteListEntries(other, multi2);
                }
                cnt++;
            }
        }
    }
    else {

        Log::info() << " + " << "Possible matched in " << multi2 << std::endl;

        size_t cnt = 0;
        for (const auto& other : matches) {

            if (saveFirstPossibleMatch_ && cnt == 0) {
                multi1.save(field.path(), field.offset(), field.length(), saved_);
                multi2.save(other.path(), other.offset(), other.length(), saved_);
                saved_++;
            }

            if (cnt++ >= maxWarnings) {
                Log::info() << " # ..." << std::endl;
                break;
            }


            Log::info() << " ? ";
            other.printDifference(Log::info(), field);
            Log::info() << " (";
            other.compareExtra(Log::info(), field);
            Log::info() << ")" << std::endl;
            if (whiteListEntries_) {
                whiteListEntries(other, multi2);
            }
        }
    }
    Log::info() << std::endl;
}


void FieldComparator::compareFields(const MultiFile& multi1, const MultiFile& multi2, const FieldSet& fields1,
                                    const FieldSet& fields2, bool compareValues, bool compareMissingValues,
                                    bool compareStatistics) {

    bool show = true;

    for (const auto& j : fields1) {
        auto other = fields2.same(j);
        if (other == fields2.end()) {
            missingField(multi1, multi2, j, fields2, show);
        }
        else if (j.canCompareFieldValues()) {
            if (compareValues) {
                compareFieldValues(multi1, multi2, j, *other);
            }
            if (compareMissingValues) {
                compareFieldMissingValues(multi1, multi2, j, *other);
            }
            if (compareStatistics) {
                compareFieldStatistics(multi1, multi2, j, *other);
            }
        }
    }
}


void FieldComparator::compareCounts(const std::string& name, const MultiFile& multi1, const MultiFile& multi2,
                                    FieldSet& fields1, FieldSet& fields2) {

    size_t n1 = count(multi1, fields1);
    size_t n2 = count(multi2, fields2);

    if (n1 != n2) {
        Log::info() << name << " count mismatch" << "\n  " << n1 << " " << multi1 << "\n  " << n2 << " " << multi2
                    << std::endl;
        error("count-mismatches");
    }
}


}  // namespace mir::compare
