/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */



#include "mir/compare/BufrField.h"
#include "mir/util/Grib.h"
#include "eckit/types/Types.h"


#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

namespace mir {
namespace compare {


static double bufrRelativeError_ = 0.;


void BufrField::addOptions(std::vector<eckit::option::Option*>& options) {
    using namespace eckit::option;
    options.push_back(new SimpleOption<double>("bufr-relative-error",
                      "Relative when comparing BUFR floating pooint values"));
}

void BufrField::setOptions(const eckit::option::CmdArgs &args) {
    args.get("bufr-relative-error", bufrRelativeError_);
}

BufrEntry::BufrEntry(const std::string& name,
                     long l,
                     double d,
                     const std::string& s,
                     int type):
    name_(name),
    l_(l),
    d_(d),
    s_(s),
    type_(type) {

    switch (type_) {

    case GRIB_TYPE_LONG:
        s_.clear();
        d_ = 0;
        break;

    case GRIB_TYPE_DOUBLE:
        s_.clear();
        l_ = 0;

        break;

    case GRIB_TYPE_STRING:
        d_ = 0;
        l_ = 0;
        break;

    default:
        NOTIMP;
        break;
    }

}

void BufrEntry::print(std::ostream &out) const {
    out << name_ << '=';
    printValue(out);
}



void BufrEntry::printValue(std::ostream &out) const {

    switch (type_) {

    case GRIB_TYPE_LONG:
        out << l_;
        break;

    case GRIB_TYPE_DOUBLE:
        out << d_;
        break;

    case GRIB_TYPE_STRING:
        out << s_;
        break;
    }

}


inline bool sameValue(double a, double b, double e) {
    double m = std::max(::fabs(a), ::fabs(b));
    if (m > 0) {
        return ::fabs(a - b) / m <= e;
    }
    else {
        return ::fabs(a - b) <= e;
    }
}


bool BufrEntry::operator==(const BufrEntry &other) const {
    return name_ == other.name_
           && type_ == other.type_
           && l_ == other.l_
           && sameValue(d_, other.d_, bufrRelativeError_)
           && s_ == other.s_;
}

bool BufrEntry::operator!=(const BufrEntry &other) const {
    return !(*this == other);
}

bool BufrEntry::operator<(const BufrEntry &other) const {

    if (name_ != other.name_) {
        return name_ < other.name_;
    }

    if (type_ != other.type_) {
        return type_ < other.type_;
    }

    switch (type_) {

    case GRIB_TYPE_LONG:
        return  l_ < other.l_;

    case GRIB_TYPE_DOUBLE:
        return  d_ < other.d_ && !sameValue(d_, other.d_, bufrRelativeError_);

    case GRIB_TYPE_STRING:
        return  s_ < other.s_;
    }

    NOTIMP;
}

BufrField::BufrField(const char* buffer, size_t size,
                     const std::string& path, off_t offset,
                     const std::vector<std::string>& ignore):
    FieldBase(path, offset, size) {

    grib_handle *h = grib_handle_new_from_message(0, buffer, size);
    ASSERT(h);
    HandleDeleter delh(h);

    size_t count;
    GRIB_CALL(grib_get_size(h, "unexpandedDescriptors", &count));
    ASSERT(count > 0);

    descriptors_.resize(count);

    size_t n = count;
    GRIB_CALL(grib_get_long_array(h, "unexpandedDescriptors", &descriptors_[0], &n));
    ASSERT(n == count);



    bufr_keys_iterator *ks = codes_bufr_keys_iterator_new(h, 0);
    ASSERT(ks);
    BKeyIteratorDeleter delk(ks);

    /// @todo this code should be factored out into mir

    // bool sfc = false;

    // std::map<std::string, std::string> req;

    codes_set_long(h, "unpack", 1);

    while (codes_bufr_keys_iterator_next(ks)) {
        const char *name = codes_bufr_keys_iterator_get_name(ks);


        ASSERT(name);
        if (strcmp(name, "unexpandedDescriptors") == 0) {
            continue;
        }

        double d = 0;
        long l = 0;
        char s[1024];
        size_t len = sizeof(s);

        int t;
        GRIB_CALL(grib_get_native_type(h, name, &t));

        switch (t) {

        case GRIB_TYPE_LONG:
            GRIB_CALL(grib_get_long(h, name, &l));
            break;

        case GRIB_TYPE_DOUBLE:
            GRIB_CALL(grib_get_double(h, name, &d));

            break;

        case GRIB_TYPE_STRING:
            GRIB_CALL(grib_get_string(h, name, s, &len));
            break;

        default:
            throw eckit::SeriousBug(std::string("Unsupported BUFR type: ") + grib_get_type_name(t));
        }

        ASSERT(entriesByName_.find(name) == entriesByName_.end());
        entriesByName_[name] = entries_.size();
        entries_.push_back(BufrEntry(name, l, d, s, t));

    }

}

BufrField::~BufrField() {
}


Field BufrField::field(const char* buffer, size_t size,
                       const std::string& path, off_t offset,
                       const std::vector<std::string>& ignore) {


    BufrField* field = new BufrField(buffer, size, path, offset, ignore);

    Field result(field);
    return result;
}

void BufrField::print(std::ostream &out) const {

    out << '[';
    const char* sep = "";
    for (auto j : entries_) {
        out << sep;

        if (j.name()[0] == '#' && j.name()[1] == '2') {
            out << "...";
            break;
        }
        out << j;
        sep = ",";
    }
    out << ']';

}


bool BufrField::wrapped() const {
    return false;
}

bool BufrField::less_than(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    return entries_ < other.entries_;
}

void BufrField::whiteListEntries(std::ostream& out) const {
    out << "bufr(white)";
}

size_t BufrField::differences(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    size_t count = 0;

    size_t n = std::min(entries_.size(), other.entries_.size());
    for (size_t i = 0; i < n; ++i) {
        if (entries_[i] != other.entries_[i]) {
            count++;
        }
    }

    count += std::max(n, entries_.size());
    count += std::max(n, other.entries_.size());

    return count;
}


// template<class T>
// static void pdiff(std::ostream & out, const T& v1, const T& v2) {
//     if (v1 != v2) {
//         // out << eckit::Colour::red << eckit::Colour::bold << v1 << eckit::Colour::reset;
//         out << "**" << v1 << "**";
//     }
//     else {
//         out << v1;
//     }
// }


std::ostream& BufrField::printDifference(std::ostream& out, const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);

    const std::vector<BufrEntry>& ei = entries_;
    const std::vector<BufrEntry>& ej = other.entries_;

    const std::map<std::string, size_t>& ni = entriesByName_;
    const std::map<std::string, size_t>& nj = other.entriesByName_;

    size_t count = 0;
    size_t n = std::min(ei.size(), ej.size());
    size_t j = 0;
    size_t i = 0;

    for (; i < n  && j < n;) {
        if (ei[i] == ej[j]) {
            ++i;
            ++j;
            continue;
        }

        if (++count > 5) {
            out << "...";
            break;
        }

        if (ei[i].name() == ej[j].name()) {
            out << '(' << ei[i].name() << '=';
            ei[i].printValue(out);
            out << '|';
            ej[j].printValue(out);
            out << ')';

            ++i;
            ++j;

            continue;
        }

        auto ki = ni.find(ej[j].name());
        auto kj = nj.find(ei[i].name());

        if (kj == nj.end()) {

            if (++count > 5) {
                out << "...";
                break;
            }

            out << '(' << ei[i].name() << '=';
            ei[i].printValue(out);
            out << "|?)";

            ++i;
        }

        if (ki == ni.end()) {

            if (++count > 5) {
                out << "...";
                break;
            }

            out << '(' << ej[j].name() << "=?|";
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
        out << '(' << ei[i].name() << '=';
        ei[i].printValue(out);
        out << "|?)";
    }

    for (; j < ej.size(); ++j) {

        if (++count > 5) {
            out << "...";
            break;
        }
        out << '(' << ej[j].name() << "=?|";
        ej[j].printValue(out);
        out << ')';
    }

    return out;
}

void BufrField::compareAreas(std::ostream& out, const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    // out << "bufr(area)";
}

bool BufrField::same(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    return entries_ == other.entries_;
}

bool BufrField::match(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    return descriptors_ == other.descriptors_;
}

std::ostream& BufrField::printGrid(std::ostream& out) const {
    out << "-";
    return out;
}

bool BufrField::match(const std::string&, const std::string&) const {
    NOTIMP;
    return false;
}

size_t BufrField::numberOfPoints() const {
    return 0;
}

const std::string& BufrField::format() const {
    static std::string bufr = "bufr";
    return bufr;
}

//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
}  // namespace compare

}  // namespace mir
