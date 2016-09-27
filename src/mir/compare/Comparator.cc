/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/compare/Comparator.h"
#include "mir/compare/MultiFile.h"
#include "mir/compare/FieldSet.h"
#include "mir/compare/MultiFile.h"

#include "eckit/filesystem/PathName.h"

#include "eckit/io/StdFile.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/log/Plural.h"
#include "eckit/io/Buffer.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"
#include "mir/caching/InMemoryCache.h"
#include "eckit/io/StdFile.h"
#include "mir/util/Grib.h"

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "mir/compare/Comparator.h"

#include <cmath>

namespace mir {
namespace compare {

static mir::InMemoryCache<eckit::StdFile> cache_("files", 256, "PGEN_COMPARE_FILE_CACHE");

//----------------------------------------------------------------------------------------------------------------------

void Comparator::addOptions(std::vector<eckit::option::Option*>& options) {
    using namespace eckit::option;

    options.push_back(new SimpleOption<size_t>("maximum-number-of-errors", "Maximum number of errors per task"));

    options.push_back(new SimpleOption<bool>("save-fields",             "Save fields that do not compare"));

    options.push_back(new SimpleOption<bool>("normalise-longitudes",    "(Not yet used) Compare normalised values of east/west longitude (e.g. -1 == 359)"));
    options.push_back(new SimpleOption<long>("round-degrees",           "(Not yet used) Number of decimal digits to round degrees to (away from zero)"));

    options.push_back(new SimpleOption<bool>("file-names-only",         "Only check that the list of files created are the same"));
    options.push_back(new SimpleOption<bool>("list-file-names",         "Create two files with extension '.list' containing the files names"));

    options.push_back(new SimpleOption<bool>("ignore-exceptions",       "Ignore exceptions"));
    options.push_back(new SimpleOption<bool>("ignore-count-mismatches", "Ignore field count mismatches"));
    options.push_back(new SimpleOption<bool>("ignore-fields-not-found", "Ignore fields not found"));

    options.push_back(new SimpleOption<bool>("ignore-duplicates",       "Ignore duplicate fields"));
    options.push_back(new SimpleOption<bool>("compare-statistics",      "Compare field statistics"));

    options.push_back(new SimpleOption<std::string>("ignore",           "Slash separated list of request keys to ignore when comparing fields"));
    options.push_back(new SimpleOption<std::string>("parameters-white-list",       "Slash separated list of parameters to ignore"));

}



//----------------------------------------------------------------------------------------------------------------------

Comparator::Comparator(const eckit::option::CmdArgs &args):
    fatals_(0),
    warnings_(0),
    args_(args),
    normaliseLongitudes_(false),
    roundDegrees_(false),
    rounding_(1),
    compareStatistics_(false),
    saveFields_(false),
    maximumNumberOfErrors_(5) {

    args.get("maximum-number-of-errors", maximumNumberOfErrors_);
    args.get("normalise-longitudes", normaliseLongitudes_);
    args.get("compare-statistics", compareStatistics_);
    args.get("requirements", requirements_);
    args.get("save-fields", saveFields_);

    long digits = 0;
    roundDegrees_ = args.get("round-degrees", digits);

    while (digits > 0) {
        rounding_ *= 10.0;
        digits--;
    }

    while (digits < 0) {
        rounding_ /= 10.0;
        digits++;
    }

    std::string ignore;
    args.get("ignore", ignore);
    eckit::Tokenizer parse("/");
    parse(ignore, ignore_);


    eckit::Translator<std::string, long> s2l;
    std::string params;
    args.get("parameters-white-list", params);
    std::vector<std::string> v;
    parse(params, v);
    for (auto j = v.begin(); j != v.end(); ++j) {
        parametersWhiteList_.insert(s2l(*j));
    }

}

Comparator::~Comparator() {
}

void Comparator::compare(const std::string& name,
                         const MultiFile& multi1,
                         const MultiFile& multi2) {


    size_t save = fatals_;

    FieldSet fields1;
    FieldSet fields2;

    compareCounts(name, multi1, multi2, fields1, fields2);
    compareFields(multi1, multi2, fields1, fields2, true);
    compareFields(multi2, multi1, fields2, fields1, false);


    if (fatals_ == save) {
        std::cout << name << " OK." << std::endl;
    }
    else {
        if (!requirements_.empty()) {
            std::string ext = eckit::PathName(requirements_).extension();

            std::ofstream out(name + ext);
            std::ifstream in(requirements_);

            std::cout << "Save " << name << ext << std::endl;

            std::string dstream = name.substr(0, 2);
            std::string destination = name.substr(24, 3);

            char line[1024];
            while (in.getline(line, sizeof(line))) {
                if (dstream == std::string(line + 99, line + 101) &&
                        destination == std::string(line + 102, line + 105)) {
                    out << line << std::endl;
                }
            }

        }
        if (saveFields_) {
            multi1.save();
            multi2.save();
        }
    }

}


void Comparator::compare(const std::string& path1,
                         const std::string& path2) {
    MultiFile multi1(path1);
    MultiFile multi2(path2);

    compare("COMPARE", multi1, multi2);
}

void Comparator::error(const char* what) {
    bool ignore = false;
    args_.get(std::string("ignore-") + what, ignore);
    if (ignore) {
        warnings_++;
    }
    else {
        fatals_++;
        std::cout << "ERROR " << what << std::endl;
        if (fatals_ > maximumNumberOfErrors_) {
            std::cout << "Maximum number of errors reached (" << maximumNumberOfErrors_ << ")" << std::endl;
            ::exit(1);
        }
    }
}

double Comparator::normalised(double longitude) const {
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

void Comparator::getField(const MultiFile& multi,
                          eckit::Buffer& buffer,
                          FieldSet& fields,
                          const std::string& path,
                          off_t offset,
                          size_t size) {

    Field field(path, offset, size);

    const char *p = buffer + size - 4;

    if (p[0] != '7' || p[1] != '7' || p[2] != '7' || p[3] != '7')
        throw eckit::SeriousBug("No 7777 found");

    grib_handle *h = grib_handle_new_from_message(0, buffer, size);
    ASSERT(h);
    HandleDeleter del(h);

    char mars_str [] = "mars";
    grib_keys_iterator *ks = grib_keys_iterator_new(h, GRIB_KEYS_ITERATOR_ALL_KEYS, mars_str);
    ASSERT(ks);

    while (grib_keys_iterator_next(ks)) {
        const char *name = grib_keys_iterator_get_name(ks);
        ASSERT(name);

        if (name[0] == '_') continue;
        if (::strcmp(name, "param") == 0) continue;

        char val[1024];
        size_t len = sizeof(val);

        GRIB_CALL( grib_keys_iterator_get_string(ks, val, &len) );

        field.insert(name, val);
    }

    grib_keys_iterator_delete(ks);


    long paramId;
    GRIB_CALL (grib_get_long(h, "paramId", &paramId));
    field.param(paramId);

    if (parametersWhiteList_.find(paramId) != parametersWhiteList_.end()) {
        eckit::Log::warning() << "Ignoring white-listed parameter " << paramId << " in " << multi << std::endl;
        return;
    }


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
                        oss << "N";
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
        std::cout << "Duplicate field in "
                  << multi
                  << std::endl
                  << "  ==> "
                  << field << std::endl
                  << "  ==> ";
        other.printDifference(std::cout, field);
        std::cout << std::endl;
        // << "  ==> "
        // << field.compare(other)
        // << std::endl
        ;
        error("duplicates");
    }


    fields.insert(field);

}

size_t Comparator::count(const MultiFile& multi, FieldSet& fields) {

    eckit::Buffer buffer(5L * 1024 * 1024 * 1024);

    size_t result = 0;

    for (auto p = multi.paths().begin(); p != multi.paths().end(); ++p) {

        int err;
        size_t size = buffer.size();

        eckit::StdFile f(*p);
        while ( (err = wmo_read_any_from_file(f, buffer, &size)) != GRIB_END_OF_FILE ) {


            try {
                GRIB_CALL(err);
                getField(multi, buffer, fields, *p, ftello(f) - size, size);
            } catch (std::exception& e) {
                std::cout << "Error in " << *p << " " << e.what() << std::endl;
                error("exceptions");
            }

            result++;
            size = buffer.size();
        }

    }

    return result;
}


size_t Comparator::list(const std::string& path) {

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
            getField(multi, buffer, fields, path, ftello(f) - size, size);
        } catch (std::exception& e) {
            std::cout << "Error in " << path << " " << e.what() << std::endl;
        }

        result++;
        size = buffer.size();
    }


    for (auto f = fields.begin(); f != fields.end(); ++f) {
        std::cout << *f << std::endl;
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


void Comparator::compareField(const MultiFile & multi1,
                              const MultiFile & multi2,
                              const Field & field1,
                              const Field & field2) {


    if (!compareStatistics_) {
        return;
    }


    mir::InMemoryCacheStatistics ignore;
    mir::InMemoryCacheUser<eckit::StdFile> lock(cache_, ignore);

    Statistics s1;
    getStats(field1, s1);

    Statistics s2;
    getStats(field2, s2);
//====================

    if (s1.values_ != s2.values_ ) {
        std::cout << "Number of data values mismatch: "
                  << std::endl
                  << "  " << multi1 << ": " << s1.values_ << " " << field1 << std::endl
                  << "  " << multi2 << ": " << s2.values_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

    if (s1.missing_ != s2.missing_ ) {
        std::cout << "Number of missing values mismatch: "
                  << std::endl
                  << "  " << multi1 << ": " << s1.missing_ << " " << field1 << std::endl
                  << "  " << multi2 << ": " << s2.missing_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

    if (relative_error(s1.min_, s2.min_) > 0.01) {
        std::cout << "Minimum relative error too large: " << relative_error(s1.min_, s2.min_)
                  << std::endl
                  << "  " << multi1 << ": " << s1.min_ << " " << field1 << std::endl
                  << "  " << multi2 << ": " << s2.min_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

    if (relative_error(s1.max_, s2.max_) > 0.01) {
        std::cout << "Maximum relative error too large: " << relative_error(s1.max_, s2.max_)
                  << std::endl
                  << "  " << multi1 << ": " << s1.max_ << " " << field1 << std::endl
                  << "  " << multi2 << ": " << s2.max_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }


    if (relative_error(s1.average_, s2.average_) > 0.01) {
        std::cout << "Average relative error too large: " << relative_error(s1.average_, s2.average_)
                  << std::endl
                  << "  " << multi1 << ": " << s1.average_ << " " << field1 << std::endl
                  << "  " << multi2 << ": " << s2.average_ << " " << field2 << std::endl;
        error("statistics-mismatches");
    }

}


void Comparator::missingField(const MultiFile & multi1,
                              const MultiFile & multi2,
                              const Field & field,
                              const FieldSet & fields,
                              bool & show) {

    if (show) {
        error("fields-not-found");
        std::cout << "Fields in " << multi1 << " not in " << multi2 << std::endl;
        show = false;
    }

    std::cout << "   " << field << std::endl;

    // Find the best mismaches

    std::vector<Field> matches = field.bestMatches(fields);
    if (matches.size() == 0) {
        std::cout << " ? " << "No match found in " << multi2 <<  std::endl;
        size_t cnt = 0;
        for (auto m = fields.begin(); m != fields.end(); ++m) {
            const auto& other = (*m);
            if (other.match(field)) {
                std::cout << " @ ";
                other.printDifference(std::cout, field);
                std::cout << " (" << other.compare(field) << ")" << std::endl;
                cnt++;
            }
        }
        if (!cnt) {
            for (auto m = fields.begin(); m != fields.end(); ++m) {
                const auto& other = (*m);
                std::cout << " # ";
                other.printDifference(std::cout, field);
                std::cout << " (" << other.compare(field) << ")" << std::endl;
                cnt++;
            }
        }
    } else {


        for (auto m = matches.begin(); m != matches.end(); ++m) {
            const auto& other = (*m);
            std::cout << " ? ";
            other.printDifference(std::cout, field);
            std::cout << " (" << other.compare(field) << ")" << std::endl;
        }
    }
    std::cout << std::endl;

}

void Comparator::compareFields(const MultiFile & multi1,
                               const MultiFile & multi2,
                               const FieldSet & fields1,
                               const FieldSet & fields2,
                               bool compareData) {

    bool show = true;

    for (auto j = fields1.begin(); j != fields1.end(); ++j) {
        auto other = fields2.same(*j);
        if (other != fields2.end()) {
            if (compareData) {
                compareField(multi1,
                             multi2,
                             *j,
                             *other);
            }
        } else {
            missingField(multi1,
                         multi2,
                         *j,
                         fields2,
                         show);
        }
    }
}


void Comparator::compareCounts(const std::string & name,
                               const MultiFile & multi1,
                               const MultiFile & multi2,
                               FieldSet & fields1,
                               FieldSet & fields2) {


    size_t n1 = count(multi1, fields1);
    size_t n2 = count(multi2, fields2);

    if (n1 != n2) {

        std::cout << name
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
