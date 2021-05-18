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


#include "mir/compare/BufrField.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <ostream>
#include <sstream>

#include "eckit/log/Colour.h"
#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/Tokenizer.h"

#include "mir/util/Grib.h"


namespace mir {
namespace compare {


static double bufrRelativeError_ = 0.;
static bool bufrFullLists        = false;
static std::set<std::string> ignoreBufrKeys;

static std::set<std::string> headerKeys = {
    "masterTableNumber",
    "bufrHeaderSubCentre",
    "bufrHeaderCentre",
    "updateSequenceNumber",
    "dataCategory",
    "dataSubCategory",
    "masterTablesVersionNumber",
    "localTablesVersionNumber",
    "typicalYearOfCentury",
    "typicalMonth",
    "typicalDay",
    "typicalHour",
    "typicalMinute",
    "typicalDate",
    "typicalTime",
    "numberOfSubsets",
    "observedData",
    "compressedData",
    "subsetNumber",
    "dataType",
    "ensembleMemberNumber",
    "year",
    "month",
    "day",
    "hour",
    "latitude",
    "longitude",
    "pressure",
};


void BufrField::addOptions(std::vector<eckit::option::Option*>& options) {
    using eckit::option::SimpleOption;
    options.push_back(new SimpleOption<bool>("bufr-print-all-values", "Print all BUFR values"));
    options.push_back(
        new SimpleOption<double>("bufr-relative-error", "Relative when comparing BUFR floating pooint values"));

    options.push_back(new SimpleOption<std::string>("ignore-bufr-keys", "Keys to ignore when comparing"));
}

void BufrField::setOptions(const eckit::option::CmdArgs& args) {
    args.get("bufr-relative-error", bufrRelativeError_);
    args.get("bufr-print-all-values", bufrFullLists);

    std::string s;
    args.get("ignore-bufr-keys", s);


    eckit::Tokenizer parse("/");
    std::vector<std::string> v;
    parse(s, v);

    ignoreBufrKeys = std::set<std::string>(v.begin(), v.end());
}

BufrEntry::BufrEntry(const std::string& full, const std::vector<long>& l, const std::vector<double>& d,
                     const std::string& s, int type) :
    full_(full), l_(l), d_(d), s_(s), type_(type) {

    static eckit::Tokenizer parse("#");
    std::vector<std::string> v;
    parse(full_, v);

    ASSERT(v.size());
    name_ = v.back();

    ignore_ = ignoreBufrKeys.find(name_) != ignoreBufrKeys.end();


    switch (type_) {

        case CODES_TYPE_LONG:
            s_.clear();
            d_.clear();
            break;

        case CODES_TYPE_DOUBLE:
            s_.clear();
            l_.clear();

            break;

        case CODES_TYPE_STRING:
            d_.clear();
            l_.clear();
            break;

        default:
            NOTIMP;
            // break;
    }
}

void BufrEntry::print(std::ostream& out) const {
    out << full_ << '=';
    printValue(out);
}


void BufrEntry::printValue(std::ostream& out) const {

    switch (type_) {

        case CODES_TYPE_LONG:
            out << l_;
            break;

        case CODES_TYPE_DOUBLE:
            out << std::setprecision(9) << d_;
            break;

        case CODES_TYPE_STRING:
            out << s_;
            break;
    }
}


void BufrEntry::json(eckit::JSON& json) const {

    json << full_;

    switch (type_) {

        case CODES_TYPE_LONG:
            if (l_.size() == 1) {
                json << l_[0];
            }
            else {
                json << l_;
            }
            break;

        case CODES_TYPE_DOUBLE:
            if (d_.size() == 1) {
                json << d_[0];
            }
            else {
                json << d_;
            }
            break;

        case CODES_TYPE_STRING:
            json << s_;
            break;
    }
}


static bool sameValue(const std::string& name, double a, double b, double e) {
    // TODO: configure me

    if (name == "longitude") {
        a = FieldBase::normaliseLongitude(a);
        b = FieldBase::normaliseLongitude(b);
    }

    if (name == "latitude" || name == "longitude") {
        return std::abs(a - b) <= 0.0001 || sameValue("-", std::abs(a - b), 0.0001, 1e-7);
    }

    if (headerKeys.find(name) != headerKeys.end()) {
        return a == b;
    }


    double m = std::max(std::abs(a), std::abs(b));
    if (m > 0) {
        return std::abs(a - b) / m <= e;
    }

    return std::abs(a - b) <= e;
}

template <class T>
static bool sameValue(const std::string& name, const std::vector<T>& a, const std::vector<T>& b, double e) {

    if (a.size() != b.size()) {


        if (a.size() == 1) {
            std::vector<T> tmp(b.size(), a[0]);
            return sameValue(name, tmp, b, e);
        }

        if (b.size() == 1) {
            std::vector<T> tmp(a.size(), b[0]);
            return sameValue(name, a, tmp, e);
        }


        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        if (!sameValue(name, double(a[i]), double(b[i]), e)) {
            return false;
        }
    }
    return true;
}


bool BufrEntry::operator==(const BufrEntry& other) const {

    if (full_ != other.full_) {
        return false;
    }

    if (type_ != other.type_) {
        return false;
    }


    switch (type_) {

        case CODES_TYPE_LONG:
            return sameValue(name_, l_, other.l_, bufrRelativeError_);

        case CODES_TYPE_DOUBLE:
            return sameValue(name_, d_, other.d_, bufrRelativeError_);

        case CODES_TYPE_STRING:
            return s_ == other.s_;
    }

    NOTIMP;
}

bool BufrEntry::operator!=(const BufrEntry& other) const {
    return !(*this == other);
}

bool BufrEntry::operator<(const BufrEntry& other) const {

    if (full_ != other.full_) {
        return full_ < other.full_;
    }

    if (type_ != other.type_) {
        return type_ < other.type_;
    }

    switch (type_) {

        case CODES_TYPE_LONG:
            return l_ < other.l_;

        case CODES_TYPE_DOUBLE:
            return d_ < other.d_ && !sameValue(name_, d_, other.d_, bufrRelativeError_);

        case CODES_TYPE_STRING:
            return s_ < other.s_;
    }

    NOTIMP;
}

BufrField::BufrField(const char* buffer, size_t size, const std::string& path, off_t offset,
                     const std::vector<std::string>& /*ignore*/) :
    FieldBase(path, offset, size) {
    // NOTE: should this be using BUFR functions, rather than GRIB?

    auto h = codes_handle_new_from_message(nullptr, buffer, size);
    HandleDeleter delh(h);

    size_t nDescriptors;
    GRIB_CALL(codes_get_size(h, "unexpandedDescriptors", &nDescriptors));
    ASSERT(nDescriptors > 0);

    descriptors_.resize(nDescriptors);

    size_t n = nDescriptors;
    GRIB_CALL(codes_get_long_array(h, "unexpandedDescriptors", &descriptors_[0], &n));
    ASSERT(n == nDescriptors);


    bufr_keys_iterator* ks = codes_bufr_keys_iterator_new(h, 0);
    ASSERT(ks);
    BKeyIteratorDeleter delk(ks);

    /// @todo this code should be factored out into mir

    // bool sfc = false;

    // std::map<std::string, std::string> req;

    codes_set_long(h, "unpack", 1);

    while (codes_bufr_keys_iterator_next(ks) != 0) {
        const char* name = codes_bufr_keys_iterator_get_name(ks);


        ASSERT(name);
        if (strcmp(name, "unexpandedDescriptors") == 0) {
            continue;
        }

        if (strcmp(name, "subsetNumber") == 0) {
            continue;
        }

        std::vector<double> d;
        std::vector<long> l;
        char s[1024];
        size_t len = sizeof(s);

        int t;
        GRIB_CALL(codes_get_native_type(h, name, &t));

        size_t count = 0;
        GRIB_CALL(codes_get_size(h, name, &count));
        // ASSERT(count == 1);


        switch (t) {

            case CODES_TYPE_LONG:
                l.resize(count);
                GRIB_CALL(codes_get_long_array(h, name, &l[0], &count));
                ASSERT(l.size() == count);
                break;

            case CODES_TYPE_DOUBLE:
                d.resize(count);
                GRIB_CALL(codes_get_double_array(h, name, &d[0], &count));
                ASSERT(d.size() == count);
                break;

            case CODES_TYPE_STRING:
                ASSERT(count == 1);
                GRIB_CALL(codes_get_string(h, name, s, &len));
                break;

            default:
                throw exception::SeriousBug(std::string("Unsupported BUFR type: ") + codes_get_type_name(t));
        }


        if (entriesByName_.find(name) != entriesByName_.end()) {
            std::ostringstream oss;
            oss << "BufrEntry duplicate name [" << name << "]";
            throw exception::SeriousBug(oss.str());
        }

        ASSERT(entriesByName_.find(name) == entriesByName_.end());
        entriesByName_[name] = allEntries_.size();

        allEntries_.emplace_back(BufrEntry(name, l, d, s, t));
        if (allEntries_.back().ignore()) {
            ignored_.insert(allEntries_.back().name());
        }
        else {
            activeEntries_.push_back(allEntries_.back());
        }
    }
}


void BufrField::json(eckit::JSON& json) const {
    json.startObject();
    FieldBase::json(json);


    for (auto& j : activeEntries_) {
        json << j;
    }

    json << "descriptors";
    json.startList();
    for (auto& j : descriptors_) {
        json << j;
    }
    json.endList();

    if (!ignored_.empty()) {
        json << "ignored" << ignored_;
    }


    json.endObject();
}


Field BufrField::field(const char* buffer, size_t size, const std::string& path, off_t offset,
                       const std::vector<std::string>& ignore) {
    Field result(new BufrField(buffer, size, path, offset, ignore));
    return result;
}


void BufrField::print(std::ostream& out) const {

    out << '[';
    const char* sep = "";
    for (auto& j : activeEntries_) {
        out << sep;

        if (!bufrFullLists) {
            if (j.full()[0] == '#' && j.full()[1] == '2') {
                out << "...";
                break;
            }
        }
        out << j;
        sep = ",";
    }
    out << ';';
    out << descriptors_;
    out << ";ignored=";
    out << ignored_;
    out << ']';
}


bool BufrField::wrapped() const {
    return false;
}

bool BufrField::less_than(const FieldBase& o) const {
    auto& other = dynamic_cast<const BufrField&>(o);
    if (ignored_ == other.ignored_) {
        return activeEntries_ < other.activeEntries_;
    }
    return ignored_ < other.ignored_;
}

void BufrField::whiteListEntries(std::ostream& out) const {
    out << "bufr(white)";
}

size_t BufrField::differences(const FieldBase& o) const {
    auto& other  = dynamic_cast<const BufrField&>(o);
    size_t count = 0;


    size_t n = std::min(activeEntries_.size(), other.activeEntries_.size());

    for (size_t i = 0; i < n; ++i) {
        if (activeEntries_[i] != other.activeEntries_[i]) {
            count += n;  // More weight at the begining
        }
    }

    count += std::max(n, activeEntries_.size());
    count += std::max(n, other.activeEntries_.size());

    return count;
}


std::ostream& BufrField::printDifference(std::ostream& out, const FieldBase& o) const {
    auto& other = dynamic_cast<const BufrField&>(o);

    const std::vector<BufrEntry>& ei = activeEntries_;
    const std::vector<BufrEntry>& ej = other.activeEntries_;

    const std::map<std::string, size_t>& ni = entriesByName_;
    const std::map<std::string, size_t>& nj = other.entriesByName_;

    size_t count = 0;
    size_t n     = std::min(ei.size(), ej.size());
    size_t j     = 0;
    size_t i     = 0;

    for (; i < n && j < n;) {
        if (ei[i] == ej[j]) {
            ++i;
            ++j;
            continue;
        }

        if (++count > 5) {
            out << "...";
            break;
        }

        if (ei[i].full() == ej[j].full()) {
            out << '(' << ei[i].full() << '=';
            out << eckit::Colour::red << eckit::Colour::bold;
            ei[i].printValue(out);
            out << eckit::Colour::reset;
            out << '|';
            ej[j].printValue(out);
            out << ')';

            ++i;
            ++j;

            continue;
        }

        auto ki = ni.find(ej[j].full());
        auto kj = nj.find(ei[i].full());

        if (kj == nj.end()) {

            if (++count > 5) {
                out << "...";
                break;
            }

            out << '(' << ei[i].full() << '=';
            ei[i].printValue(out);
            out << "|?)";

            ++i;
        }

        if (ki == ni.end()) {

            if (++count > 5) {
                out << "...";
                break;
            }

            out << '(' << ej[j].full() << "=?|";
            ej[j].printValue(out);
            out << ')';

            ++j;
        }
    }

    for (; i < ei.size(); ++i) {
        if (++count > 5) {
            out << "...";
            break;
        }
        out << '(' << ei[i].full() << '=';
        ei[i].printValue(out);
        out << "|?)";
    }

    for (; j < ej.size(); ++j) {

        if (++count > 5) {
            out << "...";
            break;
        }
        out << '(' << ej[j].full() << "=?|";
        ej[j].printValue(out);
        out << ')';
    }

    return out;
}

void BufrField::compareExtra(std::ostream& out, const FieldBase& o) const {
    auto& other = dynamic_cast<const BufrField&>(o);
    // out << "bufr(area)";
    size_t n        = std::min(descriptors_.size(), other.descriptors_.size());
    const char* sep = "";
    for (size_t i = 0; i < n; ++i) {
        out << sep;
        if (descriptors_[i] == other.descriptors_[i]) {
            out << descriptors_[i];
        }
        else {
            out << eckit::Colour::red << eckit::Colour::bold;
            out << descriptors_[i];
            out << eckit::Colour::reset;
        }
        sep = ",";
    }
}

bool BufrField::same(const FieldBase& o) const {
    auto& other = dynamic_cast<const BufrField&>(o);
    return (activeEntries_ == other.activeEntries_) && (ignored_ == other.ignored_);
}

bool BufrField::match(const FieldBase& o) const {
    auto& other = dynamic_cast<const BufrField&>(o);
    size_t n    = std::min(descriptors_.size(), other.descriptors_.size());
    for (size_t i = 0; i < n; ++i) {
        if (descriptors_[i] != other.descriptors_[i]) {
            bool loop1 = (descriptors_[i] > 100000);
            bool loop2 = (other.descriptors_[i] > 100000);
            if (!loop1 || !loop2) {
                return false;
            }
        }
    }
    return descriptors_.size() == other.descriptors_.size();
}

std::ostream& BufrField::printGrid(std::ostream& out) const {
    out << descriptors_;
    return out;
}

bool BufrField::match(const std::string&, const std::string&) const {
    NOTIMP;
}

size_t BufrField::numberOfPoints() const {
    return 0;
}

const std::string& BufrField::format() const {
    static std::string bufr = "bufr";
    return bufr;
}

bool BufrField::canCompareFieldValues() const {
    return false;
}


}  // namespace compare

}  // namespace mir
