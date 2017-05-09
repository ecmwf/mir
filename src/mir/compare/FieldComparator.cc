/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/compare/FieldComparator.h"

#include <cmath>

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/StdFile.h"
#include "eckit/log/Plural.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/parser/StringTools.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/utils/Translator.h"

#include "mir/caching/InMemoryCache.h"
#include "mir/compare/Comparator.h"
#include "mir/compare/FieldSet.h"
#include "mir/compare/MultiFile.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"


namespace mir {
namespace compare {

static mir::InMemoryCache<eckit::StdFile> cache_("files", 256, "PGEN_COMPARE_FILE_CACHE");


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

    options.push_back(new SimpleOption<bool>("compare-statistics",
                      "Compare field statistics"));

    options.push_back(new SimpleOption<bool>("compare-values",
                      "Compare field values"));

    options.push_back(new SimpleOption<std::string>("ignore",
                      "Slash separated list of request keys to ignore when comparing fields"));

    options.push_back(new SimpleOption<bool>("ignore-wrapping-areas",
                      "Ignore fields with an area that wraps around the globe (e.g. 0-360)"));

    Field::addOptions(options);
}



//----------------------------------------------------------------------------------------------------------------------

FieldComparator::FieldComparator(const eckit::option::CmdArgs &args, const WhiteLister& whiteLister):
    fatals_(0),
    warnings_(0),
    args_(args),
    normaliseLongitudes_(false),
    ignoreWrappingAreas_(false),
    roundDegrees_(false),
    whiteLister_(whiteLister),
    maximumNumberOfErrors_(5) {

    Field::setOptions(args);

    args_.get("normalise-longitudes", normaliseLongitudes_);
    args_.get("maximum-number-of-errors", maximumNumberOfErrors_);
    args_.get("ignore-wrapping-areas", ignoreWrappingAreas_);


    std::string ignore;
    args_.get("ignore", ignore);
    eckit::Tokenizer parse("/");
    parse(ignore, ignore_);

}

FieldComparator::~FieldComparator() {
}

void FieldComparator::compare(const std::string& name,
                              const MultiFile& multi1,
                              const MultiFile& multi2) {

    bool saveFields = false;
    args_.get("save-fields", saveFields);

    std::string requirements;
    args_.get("requirements", requirements);

    bool compareValues = false;
    args_.get("compare-values", compareValues);

    bool compareStatistics = false;
    args_.get("compare-statistics", compareStatistics);

    size_t save = fatals_;

    FieldSet fields1;
    FieldSet fields2;


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

class HandleDeleter {
    grib_handle *h_;
public:
    HandleDeleter(grib_handle *h) : h_(h) {}
    ~HandleDeleter() {
        grib_handle_delete(h_);
    }
};


static void setArea(Field& field, grib_handle *h) {
    double n = -99999, w = -99999, s = -99999, e = -99999;
    GRIB_CALL(grib_get_double(h, "latitudeOfFirstGridPointInDegrees", &n));
    GRIB_CALL(grib_get_double(h, "longitudeOfFirstGridPointInDegrees", &w));
    GRIB_CALL(grib_get_double(h, "latitudeOfLastGridPointInDegrees", &s));
    GRIB_CALL(grib_get_double(h, "longitudeOfLastGridPointInDegrees", &e));

    long scanningMode = 0;
    GRIB_CALL(grib_get_long(h, "scanningMode", &scanningMode));

    switch (scanningMode) {


    case 0:
        break;

    case 64:
        std::swap(n, s);
        break;

    default: {
        std::ostringstream oss;
        oss << "Invalid scanning mode " << scanningMode;
        throw eckit::SeriousBug(oss.str());
    }
    break;
    }


    field.area(n, w, s, e);
}

static void setGrid(Field& field, grib_handle *h) {


    double we = -99999, ns = -99999;
    GRIB_CALL(grib_get_double(h, "jDirectionIncrementInDegrees", &ns));
    GRIB_CALL(grib_get_double(h, "iDirectionIncrementInDegrees", &we));
    field.grid(ns, we);
}

void FieldComparator::getField(const MultiFile& multi,
                               eckit::Buffer& buffer,
                               FieldSet& fields,
                               const std::string& path,
                               off_t offset,
                               size_t size,
                               bool fail) {

    Field field(path, offset, size);

    const char *p = buffer + size - 4;

    if (p[0] != '7' || p[1] != '7' || p[2] != '7' || p[3] != '7')
        throw eckit::SeriousBug("No 7777 found");

    grib_handle *h = grib_handle_new_from_message(0, buffer, size);
    ASSERT(h);
    HandleDeleter del(h);

    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    grib_keys_iterator *ks = grib_keys_iterator_new(h, GRIB_KEYS_ITERATOR_ALL_KEYS, gribToRequestNamespace.c_str());
    ASSERT(ks);

    /// @todo this code should be factored out into metkit

    // bool sfc = false;

    std::map<std::string, std::string> req;

    while (grib_keys_iterator_next(ks)) {
        const char *name = grib_keys_iterator_get_name(ks);
        ASSERT(name);

        if (name[0] == '_') continue;
        if (::strcmp(name, "param") == 0) continue;

        char val[1024];
        size_t len = sizeof(val);

        GRIB_CALL( grib_keys_iterator_get_string(ks, val, &len) );

        field.insert(name, val);

        // if (::strcmp(val, "sfc") == 0) {
        //     sfc = true;
        // }

        req[name] = val;
    }

    grib_keys_iterator_delete(ks);


    long paramId;
    GRIB_CALL (grib_get_long(h, "paramId", &paramId));

    field.param(paramId);

    long numberOfDataPoints;
    GRIB_CALL (grib_get_long(h, "numberOfDataPoints", &numberOfDataPoints));
    field.numberOfPoints(numberOfDataPoints);

    // Look for request embbeded in GRIB message
    long local;

    if (grib_get_long(h, "localDefinitionNumber", &local) ==  0 && local == 191) {
        size_t size;
        /* TODO: Not grib2 compatible, but speed-up process */
        if (grib_get_size(h, "freeFormData", &size) ==  0 && size != 0) {
            unsigned char buffer[size];
            GRIB_CALL(grib_get_bytes(h, "freeFormData", buffer, &size) );

            eckit::MemoryStream s(buffer, size);

            int count;
            s >> count; // Number of requests
            ASSERT(count == 1);
            std::string tmp;
            s >> tmp; // verb
            s >> count;
            for (int i = 0; i < count; i++) {
                std::string keyword, value;
                int n;
                s >> keyword;
                std::transform(keyword.begin(), keyword.end(), keyword.begin(), tolower);
                s >> n; // Number of values
                ASSERT(n == 1);
                s >> value;
                std::transform(value.begin(), value.end(), value.begin(), tolower);
                field.insert(keyword, value);
                req[keyword] = value;
            }
        }
    }

    static eckit::Translator<long, std::string> l2s;


    {
        char value[1024];
        size_t len = sizeof(value);
        if (grib_get_string(h, "gridType", value, &len) == 0) {
            field.gridtype(value);


            if (strcmp(value, "regular_ll") == 0) {
                setGrid(field, h);
                setArea(field, h);
            } else  if (strcmp(value, "rotated_ll") == 0) {
                setGrid(field, h);
                setArea(field, h);
                {
                    double lat, lon;
                    GRIB_CALL(grib_get_double(h, "latitudeOfSouthernPoleInDegrees", &lat));
                    GRIB_CALL(grib_get_double(h, "longitudeOfSouthernPoleInDegrees", &lon) );
                    field.rotation(lat, lon);
                }
            }
            else if (strcmp(value, "sh") == 0) {

                // double d;
                {
                    long n = -1;
                    GRIB_CALL(grib_get_long(h, "pentagonalResolutionParameterJ", &n) );
                    field.resol(n);
                }
            }
            else if (strcmp(value, "reduced_gg") == 0) {
                {
                    long n = 0;
                    std::ostringstream oss;


                    GRIB_CALL(grib_get_long(h, "isOctahedral", &n) );

                    if (n) {
                        oss << "O";
                    }
                    else {

                        // Don't trust eccodes
                        size_t pl_size = 0;
                        GRIB_CALL(grib_get_size(h, "pl", &pl_size) );
                        long pl[pl_size];

                        bool isOctahedral = true;
                        for(size_t i = 1 ; i < pl_size; i++) {
                            long diff = std::abs(pl[i] - pl[i-1]);
                            std::cout << diff << std::endl;
                            if(diff != 4 && diff != 0) {
                                isOctahedral = false;
                                break;
                            }
                        }

                        if (isOctahedral) {
                            oss << "O";
                        }
                        else {
                            oss << "N";
                        }
                    }

                    GRIB_CALL(grib_get_long(h, "N", &n) );
                    oss << n;



                    // ASSERT(grib_get_double(h, "iDirectionIncrementInDegrees", &d) == 0);
                    // oss << '/' << rounded(d);
                    field.gridname(oss.str());
                }

                setArea(field, h);
            } else if (strcmp(value, "regular_gg") == 0) {
                long n;
                {
                    std::ostringstream oss;


                    GRIB_CALL(grib_get_long(h, "N", &n) );
                    oss << "F" << n;

                    // ASSERT(grib_get_double(h, "iDirectionIncrementInDegrees", &d) == 0);
                    // oss << '/' << rounded(d);
                    field.gridname(oss.str());
                }
                setArea(field, h);
            }
            else if (strcmp(value, "polar_stereographic") == 0) {
                eckit::Log::warning() << "Ignoring polar_stereographic in " << multi << std::endl;
                return;
            }
            else {
                std::ostringstream oss;
                oss << multi << ": Unknown grid [" << value << "]";
                throw eckit::SeriousBug(oss.str());
            }
        }
    }



    // long scanningMode = 0;
    // if (grib_get_long(h, "scanningMode", &scanningMode) == 0) {
    //     field.insert("scanningMode", scanningMode);
    // }

    // long decimalScaleFactor = 0;
    // if (grib_get_long(h, "decimalScaleFactor", &decimalScaleFactor) == 0) {
    //     field.insert("decimalScaleFactor", decimalScaleFactor);
    // }


    long edition;
    if (grib_get_long(h, "edition", &edition) == 0) {
        field.format("grib" + l2s(edition));
    }

    long bitmap;
    if (grib_get_long(h, "bitmapPresent", &bitmap) == 0) {
        if (bitmap) {
            field.bitmap(true);
        }
    }

    long bitsPerValue;
    if (grib_get_long(h, "bitsPerValue", &bitsPerValue) == 0) {
        field.accuracy(bitsPerValue);
    }

    {
        char value[1024];
        size_t len = sizeof(value);
        if (grib_get_string(h, "packingType", value, &len) == 0) {
            field.packing(value);
        }
    }

    {
        char value[1024];
        size_t len = sizeof(value);
        if (grib_get_string(h, "packing", value, &len) == 0) {
            field.packing(value);
        }
    }

    for (auto j = ignore_.begin(); j != ignore_.end(); ++j) {
        field.erase(*j);
    }

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

    if (whiteLister_.whiteListed(multi.name(), field)) {
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

        eckit::StdFile f(*p);
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

    eckit::StdFile f(path);
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



static eckit::StdFile& open(const std::string& path) {
    auto j = cache_.find(path);
    if (j == cache_.end()) {
        cache_.insert(path, new eckit::StdFile(path));
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

//====================
    eckit::StdFile& f = open(field.path());
    size_t size = buffer.size();
    fseek(f, field.offset(), SEEK_SET);
    GRIB_CALL(wmo_read_any_from_file(f, buffer, &size));
    ASSERT(size == field.length());

    grib_handle *h = grib_handle_new_from_message(0, buffer, size);
    ASSERT(h);
    HandleDeleter del(h);


    size_t count;
    GRIB_CALL(grib_get_size(h, "values", &count));

    long bitmap;
    GRIB_CALL(grib_get_long(h, "bitmapPresent", &bitmap));

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
        if (bitmap && values[i] == missingValue )  {
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
        if (bitmap && values[i] == missingValue) {
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

    mir::InMemoryCacheStatistics ignore;
    mir::InMemoryCacheUser<eckit::StdFile> lock(cache_, ignore);

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

    // TODO

    input::GribFileInput grib1(field1.path(), field1.offset());
    input::GribFileInput grib2(field2.path(), field2.offset());

    grib1.next();
    grib2.next();


    input::MIRInput& input1 = grib1;
    input::MIRInput& input2 = grib2;


    const param::MIRParametrisation &metadata1 = input1.parametrisation(0);
    const param::MIRParametrisation &metadata2 = input2.parametrisation(0);


    std::string comparison1;
    std::string comparison2;
    ASSERT(metadata1.get("comparison", comparison1));
    ASSERT(metadata2.get("comparison", comparison2));

    repres::RepresentationHandle repres1 = input1.field().representation();
    repres::RepresentationHandle repres2 = input2.field().representation();
    repres1->comparison(comparison1);
    repres2->comparison(comparison2);

    ASSERT(comparison1 == comparison2);

    std::vector<std::string> comparators = eckit::StringTools::split("/", comparison1);
    for (auto c = comparators.begin(); c != comparators.end(); ++c) {
        eckit::ScopedPtr<Comparator> comp(ComparatorFactory::build(*c, metadata1, metadata2));
        comp->execute(input1.field(), input2.field());
    }

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

    if (show) {
        error("fields-not-found");
        eckit::Log::info() << "Fields in " << multi1 << " not in " << multi2 << std::endl;
        show = false;
    }

    eckit::Log::info() << "   " << field << std::endl;

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
                other.compareAreas(eckit::Log::info(), field);
                eckit::Log::info() << ")" << std::endl;
                cnt++;
            }
        }
        if (!cnt) {
            for (auto m = fields.begin(); m != fields.end(); ++m) {
                const auto& other = (*m);
                eckit::Log::info() << " # ";
                other.printDifference(eckit::Log::info(), field);
                eckit::Log::info() << " (" ;
                other.compareAreas(eckit::Log::info(), field);
                eckit::Log::info() << ")" << std::endl;
                cnt++;
            }
        }
    } else {


        for (auto m = matches.begin(); m != matches.end(); ++m) {
            const auto& other = (*m);
            eckit::Log::info() << " ? ";
            other.printDifference(eckit::Log::info(), field);
            eckit::Log::info() << " (" ;
            other.compareAreas(eckit::Log::info(), field);
            eckit::Log::info() << ")" << std::endl;
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
