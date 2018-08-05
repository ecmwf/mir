/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/compare/FieldComparator.h"

#include <cmath>

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/StdFile.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/parser/StringTools.h"

#include "mir/caching/InMemoryCache.h"
#include "mir/compare/BufrField.h"
#include "mir/compare/Comparator.h"
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
#include "mir/util/Grib.h"
#include "eckit/parser/JSON.h"


using eckit::PathName;
using eckit::AutoStdFile;


namespace mir {
namespace compare {


static caching::InMemoryCache<AutoStdFile> cache_("files", 256, 0, "PGEN_COMPARE_FILE_CACHE");


const WhiteLister& DefaultWhiteLister::instance() {
    static DefaultWhiteLister i;
    return i;
}

//----------------------------------------------------------------------------------------------------------------------

void FieldComparator::addOptions(std::vector<eckit::option::Option*>& options) {
    using namespace eckit::option;

    options.push_back(new SimpleOption<size_t>("maximum-number-of-errors",
                      "Maximum number of errors per task"));

    options.push_back(new SimpleOption<bool>("save-fields",
                      "Save fields that do not compare"));

    options.push_back(new SimpleOption<bool>("save-all-fields",
                      "Save all fields into *.old and *.new files"));

    options.push_back(new SimpleOption<bool>("file-names-only",
                      "Only check that the list of files created are the same"));

    options.push_back(new SimpleOption<bool>("list-file-names",
                      "Create two files with extension '.list' containing the files names"));

    options.push_back(new SimpleOption<bool>("ignore-exceptions",
                      "Ignore exceptions"));

    options.push_back(new SimpleOption<bool>("ignore-count-mismatches",
                      "Ignore field count mismatches"));

    options.push_back(new SimpleOption<bool>("ignore-fields-not-found",
                      "Ignore fields not found"));

    options.push_back(new SimpleOption<bool>("ignore-duplicates",
                      "Ignore duplicate fields"));

    options.push_back(new SimpleOption<std::string>("ignore",
                      "Slash separated list of request keys to ignore when comparing fields"));

    options.push_back(new SimpleOption<bool>("ignore-wrapping-areas",
                      "Ignore fields with an area that wraps around the globe (e.g. 0-360)"));

    Field::addOptions(options);

    //==============================================
    options.push_back(new Separator("Field values"));

    options.push_back(new SimpleOption<bool>("compare-values", "Compare field values"));
    options.push_back(new SimpleOption<bool>("compare-statistics", "Compare field statistics"));

    options.push_back(new SimpleOption<double>("absolute-error", "Value comparison using absolute error, only values whose difference is more than tolerance are considered different"));

    options.push_back(new SimpleOption<bool>("white-list-entries",
                      "Output lines that can be used in white-list files"));

}



//----------------------------------------------------------------------------------------------------------------------

FieldComparator::FieldComparator(const eckit::option::CmdArgs &args, const WhiteLister& whiteLister):
    fatals_(0),
    warnings_(0),
    args_(args),
    normaliseLongitudes_(false),
    ignoreWrappingAreas_(false),
    roundDegrees_(false),
    maximumNumberOfErrors_(5),
    whiteLister_(whiteLister),
    whiteListEntries_(false) {

    Field::setOptions(args);

    args_.get("normalise-longitudes", normaliseLongitudes_);
    args_.get("maximum-number-of-errors", maximumNumberOfErrors_);
    args_.get("ignore-wrapping-areas", ignoreWrappingAreas_);
    args_.get("white-list-entries", whiteListEntries_);


    std::string ignore;
    args_.get("ignore", ignore);
    eckit::Tokenizer parse("/");
    parse(ignore, ignore_);

}

FieldComparator::~FieldComparator() = default;

void FieldComparator::compare(const std::string& name,
                              const MultiFile& multi1,
                              const MultiFile& multi2) {

    bool saveFields = false;
    args_.get("save-fields", saveFields);

    bool saveAllFields = false;
    args_.get("save-all-fields", saveAllFields);


    std::string requirements;
    args_.get("requirements", requirements);

    bool compareValues = false;
    args_.get("compare-values", compareValues);

    bool compareStatistics = false;
    args_.get("compare-statistics", compareStatistics);

    size_t save = fatals_;

    FieldSet fields1;
    FieldSet fields2;

    if (saveAllFields) {
        multi1.save();
        multi2.save();
    }


    compareCounts(name, multi1, multi2, fields1, fields2);

    compareFields(multi1, multi2, fields1, fields2, compareValues, compareStatistics);

    if (fatals_ == save) {
        compareFields(multi2, multi1, fields2, fields1, false,         compareStatistics);
    }


    if (fatals_ == save) {
        eckit::Log::info() << name << " OK." << std::endl;
    } else {
        if (!requirements.empty()) {
            /*
            std::string output = name + eckit::PathName(requirements).extension();

            std::ofstream out(output.c_str());
            std::ifstream in(requirements.c_str());

            eckit::Log::info() << "Save " << output << std::endl;

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

}


void FieldComparator::compare(const std::string& path1,
                              const std::string& path2) {
    MultiFile multi1(path1);
    MultiFile multi2(path2);

    compare("COMPARE", multi1, multi2);
}

void FieldComparator::error(const char* what) {
    bool ignore = false;
    args_.get(std::string("ignore-") + what, ignore);
    if (ignore) {
        warnings_++;
        eckit::Log::info() << "WARNING " << what << std::endl;
    }
    else {
        fatals_++;
        eckit::Log::info() << "ERROR " << what << std::endl;
        if (fatals_ > maximumNumberOfErrors_) {
            eckit::Log::info() << "Maximum number of errors reached (" << maximumNumberOfErrors_ << ")" << std::endl;
            throw eckit::SeriousBug("Maximum number of errors reached");
        }
    }
}

double FieldComparator::normalised(double longitude) const {
    if (normaliseLongitudes_) {
        while (longitude < 0 ) {
            longitude += 360;
        }

        while (longitude >= 360) {
            longitude -= 360;
        }
    }
    return longitude;
}



Field FieldComparator::getField(eckit::Buffer& buffer,
                                const std::string& path,
                                off_t offset,
                                size_t size) {
    const char *q = ((const char*)buffer);
    const char *p = ((const char*)buffer) + size - 4;

    if (p[0] != '7' || p[1] != '7' || p[2] != '7' || p[3] != '7')
        throw eckit::SeriousBug("No 7777 found");


    Field field;
    if (q[0] == 'G' && q[1] == 'R' && q[2] == 'I' && q[3] == 'B') {
        field  = GribField::field(buffer, size, path, offset, ignore_);
    }

    if (q[0] == 'B' && q[1] == 'U' && q[2] == 'F' && q[3] == 'R') {
        field  = BufrField::field(buffer, size, path, offset, ignore_);
    }

    ASSERT(field);
    return field;
}


void FieldComparator::getField(const MultiFile& multi,
                               eckit::Buffer& buffer,
                               FieldSet& fields,
                               const std::string& path,
                               off_t offset,
                               size_t size,
                               bool fail) {


    Field field = getField(buffer, path, offset, size);

    if (fields.duplicate(field) != fields.end()) {
        const auto& other = *fields.duplicate(field);
        eckit::Log::info() << "Duplicate field in "
                           << multi
                           << std::endl
                           << "  ==> "
                           << field << std::endl
                           << "  ==> ";
        other.printDifference(eckit::Log::info(), field);
        eckit::Log::info() << std::endl;
        // << "  ==> "
        // << field.compare(other)
        // << std::endl
        if (fail) {
            error("duplicates");
        }
    }

    if (whiteLister_.whiteListed(multi, field)) {
        eckit::Log::info() << "Field white listed " << field << std::endl;
        return;
    }

    fields.insert(field);


}


size_t FieldComparator::count(const MultiFile& multi, FieldSet& fields) {

    eckit::Buffer buffer(5L * 1024 * 1024 * 1024);

    fields.clear();

    for (auto p = multi.paths().begin(); p != multi.paths().end(); ++p) {

        int err;
        size_t size = buffer.size();

        eckit::AutoStdFile f(*p);
        while ( (err = wmo_read_any_from_file(f, buffer, &size)) != GRIB_END_OF_FILE ) {


            try {
                GRIB_CALL(err);
                getField(multi, buffer, fields, *p, ftello(f) - size, size, true);
            } catch (std::exception& e) {
                eckit::Log::info() << "Error in " << *p << " " << e.what() << std::endl;
                error("exceptions");
            }

            size = buffer.size();
        }
    }

    return fields.size();
}


size_t FieldComparator::list(const std::string& path) {

    MultiFile multi(path, path);
    eckit::Buffer buffer(5L * 1024 * 1024 * 1024);
    FieldSet fields;
    size_t result = 0;

    int err;
    size_t size = buffer.size();

    eckit::AutoStdFile f(path);
    while ( (err = wmo_read_any_from_file(f, buffer, &size)) != GRIB_END_OF_FILE ) {


        try {
            GRIB_CALL(err);
            getField(multi, buffer, fields, path, ftello(f) - size, size, false);
        } catch (std::exception& e) {
            eckit::Log::info() << "Error in " << path << " " << e.what() << std::endl;
        }

        result++;
        size = buffer.size();
    }


    for (auto f = fields.begin(); f != fields.end(); ++f) {
        eckit::Log::info() << *f << std::endl;
    }


    return result;
}

void FieldComparator::json(eckit::JSON& json, const std::string& path) {
    MultiFile multi(path, path);
    eckit::Buffer buffer(5L * 1024 * 1024 * 1024);

    int err;
    size_t size = buffer.size();

    eckit::AutoStdFile f(path);
    while ( (err = wmo_read_any_from_file(f, buffer, &size)) != GRIB_END_OF_FILE ) {

        GRIB_CALL(err);
        Field field = getField(buffer, path, ftello(f) - size, size);

        json << field;

        size = buffer.size();
    }

}




static AutoStdFile& open(const std::string& path) {
    auto j = cache_.find(path);
    if (j == cache_.end()) {
        cache_.insert(path, new AutoStdFile(path));
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

struct Statistics
{
    double min_;
    double max_;
    double average_;

    size_t missing_;
    size_t values_;
};


static void getStats(const Field& field, Statistics& stats) {

    eckit::Buffer buffer(5L * 1024 * 1024 * 1024);

    AutoStdFile& f = open(field.path());
    size_t size = buffer.size();
    fseek(f, field.offset(), SEEK_SET);
    GRIB_CALL(wmo_read_any_from_file(f, buffer, &size));
    ASSERT(size == field.length());

    grib_handle *h = grib_handle_new_from_message(0, buffer, size);
    ASSERT(h);
    HandleDeleter del(h);


    size_t count;
    GRIB_CALL(grib_get_size(h, "values", &count));

    long missingValuesPresent;
    GRIB_CALL(grib_get_long(h, "missingValuesPresent", &missingValuesPresent));

    double missingValue;
    GRIB_CALL(grib_get_double(h, "missingValue", &missingValue));

    double values[count];


    size = count;

    GRIB_CALL(grib_get_double_array(h, "values", values, &size));
    ASSERT(size == count);


    ASSERT(size);

    stats = {0,};

    size_t first = 0;
    for (size_t i = 0; i < size; i++) {
        if (missingValuesPresent && values[i] == missingValue )  {
            stats.missing_++;
        }
        else {
            first = i + 1;
            stats.min_ = values[0];
            stats.max_ = values[0];
            stats.average_ = values[0];
            break;
        }
    }

    if (stats.missing_ == size) {
        // All values missing
        return;
    }

    for (size_t i = first; i < size; i++) {
        if (missingValuesPresent && values[i] == missingValue) {
            stats.missing_++;
        }
        else {
            stats.min_ = std::min(stats.min_, values[i]);
            stats.max_ = std::max(stats.max_, values[i]);
            stats.average_ += values[i];
            stats.values_++;
        }
    }

    stats.average_ /= stats.values_;

}


void FieldComparator::compareFieldStatistics(
    const MultiFile & multi1,
    const MultiFile & multi2,
    const Field & field1,
    const Field & field2) {

    mir::caching::InMemoryCacheStatistics ignore;
    mir::caching::InMemoryCacheUser<AutoStdFile> lock(cache_, ignore);

    Statistics s1;
    getStats(field1, s1);

    Statistics s2;
    getStats(field2, s2);

    if (s1.values_ != s2.values_ ) {
        eckit::Log::info() << "Number of data values mismatch: "
                           << std::endl
                           << "  " << multi1 << ": " << s1.values_ << " " << field1 << std::endl
                           << "  " << multi2 << ": " << s2.values_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

    if (s1.missing_ != s2.missing_ ) {
        eckit::Log::info() << "Number of missing values mismatch: "
                           << std::endl
                           << "  " << multi1 << ": " << s1.missing_ << " " << field1 << std::endl
                           << "  " << multi2 << ": " << s2.missing_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

    if (relative_error(s1.min_, s2.min_) > 0.01) {
        eckit::Log::info() << "Minimum relative error too large: " << relative_error(s1.min_, s2.min_)
                           << std::endl
                           << "  " << multi1 << ": " << s1.min_ << " " << field1 << std::endl
                           << "  " << multi2 << ": " << s2.min_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

    if (relative_error(s1.max_, s2.max_) > 0.01) {
        eckit::Log::info() << "Maximum relative error too large: " << relative_error(s1.max_, s2.max_)
                           << std::endl
                           << "  " << multi1 << ": " << s1.max_ << " " << field1 << std::endl
                           << "  " << multi2 << ": " << s2.max_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }


    if (relative_error(s1.average_, s2.average_) > 0.01) {
        eckit::Log::info() << "Average relative error too large: " << relative_error(s1.average_, s2.average_)
                           << std::endl
                           << "  " << multi1 << ": " << s1.average_ << " " << field1 << std::endl
                           << "  " << multi2 << ": " << s2.average_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

}

void FieldComparator::compareFieldValues(
    const FieldComparator::MultiFile& multi1,
    const FieldComparator::MultiFile& multi2,
    const Field& field1,
    const Field& field2) {
    // using namespace param;

    // TODO

    input::GribFileInput grib1(field1.path(), field1.offset());
    input::GribFileInput grib2(field2.path(), field2.offset());

    grib1.next();
    grib2.next();


    input::MIRInput& input1 = grib1;
    input::MIRInput& input2 = grib2;


    const param::MIRParametrisation &metadata1 = input1.parametrisation();
    const param::MIRParametrisation &metadata2 = input2.parametrisation();


    std::string comparison1;
    std::string comparison2;
    ASSERT(metadata1.get("comparison", comparison1));
    ASSERT(metadata2.get("comparison", comparison2));

    repres::RepresentationHandle repres1 = input1.field().representation();
    repres::RepresentationHandle repres2 = input2.field().representation();
    repres1->comparison(comparison1);
    repres2->comparison(comparison2);

    ASSERT(comparison1 == comparison2);


    // get input and parameter-specific parametrisations
    const param::ConfigurationWrapper args_wrap(args_);
    static param::DefaultParametrisation defaults;
    param::CombinedParametrisation combined1(args_wrap, metadata1, defaults);
    param::CombinedParametrisation combined2(args_wrap, metadata2, defaults);

    std::vector<std::string> comparators = eckit::StringTools::split("/", comparison1);
    for (auto c = comparators.begin(); c != comparators.end(); ++c) {
        eckit::ScopedPtr<Comparator> comp(ComparatorFactory::build(*c, combined1, combined2));
        comp->execute(input1.field(), input2.field());
    }

}

void FieldComparator::whiteListEntries(const Field & field, const MultiFile & multi) const {
    multi.whiteListEntries(eckit::Log::info());
    eckit::Log::info() << ' ';
    field.whiteListEntries(eckit::Log::info());
    eckit::Log::info() << std::endl;
}


namespace {
struct Compare {
    const Field & field_;
    Compare(const Field & field) : field_(field) {}
    bool operator()(const Field &a, const Field & b) {
        size_t da = field_.differences(a);
        size_t db = field_.differences(b);
        return da < db;
    }
};

}

void FieldComparator::missingField(const MultiFile & multi1,
                                   const MultiFile & multi2,
                                   const Field & field,
                                   const FieldSet & fields,
                                   bool & show) {


    if (ignoreWrappingAreas_) {

        if (field.wrapped()) {
            eckit::Log::info()  << "Ignoring wrapped field " << field << std::endl;
            return;
        }

        std::vector<Field> matches = field.bestMatches(fields);
        std::sort(matches.begin(), matches.end(), Compare(field));
        if (matches.size() > 0) {
            for (auto m = matches.begin(); m != matches.end(); ++m) {
                const auto& other = (*m);
                if (other.wrapped()) {
                    eckit::Log::info()  << "Ignoring field " << field << " that matches wrapped " << other << std::endl;
                    return;
                }
            }
        }
    }

    if (whiteLister_.ignoreError(multi1, field)) {
        return;
    }

    if (show) {
        error("fields-not-found");
        eckit::Log::info() << "Fields in " << multi1 << " not in " << multi2 << std::endl;
        show = false;
    }

    eckit::Log::info() << "   " << field << std::endl;
    if (whiteListEntries_) {
        whiteListEntries(field, multi1);
    }
    // Find the best mismaches

    std::vector<Field> matches = field.bestMatches(fields);
    if (matches.size() == 0) {
        eckit::Log::info() << " ? " << "No match found in " << multi2 <<  std::endl;
        size_t cnt = 0;


        for (auto m = fields.begin(); m != fields.end(); ++m) {


            const auto& other = (*m);
            if (other.match(field)) {
                eckit::Log::info() << " @ ";
                other.printDifference(eckit::Log::info(), field);
                eckit::Log::info() << " (" ;
                other.compareExtra(eckit::Log::info(), field);
                eckit::Log::info() << ")" << std::endl;

                if (whiteListEntries_) {
                    whiteListEntries(other, multi2);
                }
                cnt++;
            }
        }
        if (!cnt) {
            for (auto m = fields.begin(); m != fields.end(); ++m) {
                const auto& other = (*m);
                eckit::Log::info() << " # ";
                other.printDifference(eckit::Log::info(), field);
                eckit::Log::info() << " (" ;
                other.compareExtra(eckit::Log::info(), field);
                eckit::Log::info() << ")" << std::endl;
                if (whiteListEntries_) {
                    whiteListEntries(other, multi2);
                }
                cnt++;
            }
        }
    } else {


        for (auto m = matches.begin(); m != matches.end(); ++m) {
            const auto& other = (*m);
            eckit::Log::info() << " ? ";
            other.printDifference(eckit::Log::info(), field);
            eckit::Log::info() << " (" ;
            other.compareExtra(eckit::Log::info(), field);
            eckit::Log::info() << ")" << std::endl;
            if (whiteListEntries_) {
                whiteListEntries(other, multi2);
            }
        }
    }
    eckit::Log::info() << std::endl;

}

void FieldComparator::compareFields(const MultiFile & multi1,
                                    const MultiFile & multi2,
                                    const FieldSet & fields1,
                                    const FieldSet & fields2,
                                    bool compareValues,
                                    bool compareStatistics) {

    bool show = true;

    for (auto j = fields1.begin(); j != fields1.end(); ++j) {
        auto other = fields2.same(*j);
        if (other != fields2.end()) {
            if (compareValues && compareStatistics) {
                compareFieldStatistics(multi1, multi2, *j, *other);
            }
            if (compareValues) {
                compareFieldValues(multi1, multi2, *j, *other);
            }
        } else {
            missingField(multi1, multi2, *j, fields2, show);
        }
    }
}


void FieldComparator::compareCounts(const std::string & name,
                                    const MultiFile & multi1,
                                    const MultiFile & multi2,
                                    FieldSet & fields1,
                                    FieldSet & fields2) {


    size_t n1 = count(multi1, fields1);
    size_t n2 = count(multi2, fields2);

    if (n1 != n2) {

        eckit::Log::info() << name
                           << " count mismatch"
                           << std::endl
                           << "    " << n1 << " " << multi1 << std::endl
                           << "    " << n2 << " " << multi2  << std::endl;

        error("count-mismatches");

    }

}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace compare
}  // namespace mir
