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


namespace mir {
namespace compare {

void BufrField::addOptions(std::vector<eckit::option::Option*>& options) {
    using namespace eckit::option;
}

void BufrField::setOptions(const eckit::option::CmdArgs &args) {

}

BufrEntry::BufrEntry(const std::string& name, const eckit::Value& value, int type):
    name_(name),
    value_(value),
    type_(type) {

}

void BufrEntry::print(std::ostream &out) const {
    out << name_ << '=' << value_;
}

bool BufrEntry::operator==(const BufrEntry &other) const {
    return name_ == other.name_ && type_ == other.type_ && value_ == other.value_;
}

bool BufrEntry::operator!=(const BufrEntry &other) const {
    return !(*this == other);
}

bool BufrEntry::operator<(const BufrEntry &other) const {

    if (name_ == other.name_) {
        return name_ < other.name_;
    }

    if (type_ == other.type_) {
        return type_ < other.type_;
    }

    return  value_ < other.value_;
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
            entries_.push_back(BufrEntry(name, l, t));
            break;

        case GRIB_TYPE_DOUBLE:
            GRIB_CALL(grib_get_double(h, name, &d));
            entries_.push_back(BufrEntry(name, d, t));

            break;

        case GRIB_TYPE_STRING:
            GRIB_CALL(grib_get_string(h, name, s, &len));
            entries_.push_back(BufrEntry(name, d, t));
            break;

        default:
            throw eckit::SeriousBug(std::string("Unsupported BUFR type: ") + grib_get_type_name(t));
        }

        entriesByName_[name] = &entries_.back();

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

    size_t n = std::min(entries_.size(), other.entries_.size());
    for (size_t i = 0; i < n; ++i) {
        if (entries_[i] < other.entries_[i]) {
            return true;
        }
    }

    if (n > entries_.size()) {
        return false;
    }

    if (n > other.entries_.size()) {
        return true;
    }

    return false;
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

std::ostream& BufrField::printDifference(std::ostream& out, const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);

    const char* sep = "";

    size_t count = 0;
    size_t n = std::min(entries_.size(), other.entries_.size());
    for (size_t i = 0; i < n; ++i) {
        if (entries_[i] != other.entries_[i]) {
            out << sep;
            if (++count > 5) {
                out << "...";
                break;
            }
            out << entries_[i];
            out << " - ";
            out << other.entries_[i];
            sep = "; ";

        }
    }

    return out;
}

void BufrField::compareAreas(std::ostream& out, const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    out << "bufr(area)";
}

bool BufrField::same(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);

    size_t n = std::min(entries_.size(), other.entries_.size());
    for (size_t i = 0; i < n; ++i) {
        if (entries_[i] != other.entries_[i]) {
            return false;
        }
    }

    return entries_.size() == other.entries_.size();

}

bool BufrField::match(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    return descriptors_ == other.descriptors_;
}

std::ostream& BufrField::printGrid(std::ostream& out) const {
    out << "bufr(grid)";
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
