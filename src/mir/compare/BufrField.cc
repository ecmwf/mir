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


namespace mir {
namespace compare {

void BufrField::addOptions(std::vector<eckit::option::Option*>& options) {
    using namespace eckit::option;
}

void BufrField::setOptions(const eckit::option::CmdArgs &args) {

}


BufrField::BufrField(const std::string& path, off_t offset, size_t length):
    FieldBase(path, offset, length) {

    data_ = new char[length];


}

BufrField::~BufrField() {
    delete[] data_;
}


Field BufrField::field(const char* buffer, size_t size,
                       const std::string& path, off_t offset,
                       const std::vector<std::string>& ignore) {


    BufrField* field = new BufrField(path, offset, size);
    memcpy(field->data_, buffer, size);

    Field result(field);
    return result;
}

void BufrField::print(std::ostream &out) const {
    out << "BufrField[]";
}


bool BufrField::wrapped() const {
    return false;
}

bool BufrField::less_than(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    if (info_.length() != other.info_.length()) {
        return info_.length() < other.info_.length();
    }
    return memcmp(data_, other.data_, info_.length()) < 0;
}

void BufrField::whiteListEntries(std::ostream& out) const {
    out << "bufr(white)";
}

size_t BufrField::differences(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    size_t n = 0;
    for (size_t i = 0; i < std::min(info_.length() , other.info_.length()); ++i) {
        if (data_[i] != other.data_[i]) {
            n++;
        }
    }
    return n;
}

std::ostream& BufrField::printDifference(std::ostream& out, const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    out << "bufr(diff)";
    return out;
}

void BufrField::compareAreas(std::ostream& out, const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    out << "bufr(area)";
}

bool BufrField::same(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    if (info_.length() != other.info_.length()) {
        return false;
    }
    return memcmp(data_, other.data_, info_.length()) == 0;
}

bool BufrField::match(const FieldBase& o) const {
    const BufrField& other = dynamic_cast<const BufrField&>(o);
    if (info_.length() != other.info_.length()) {
        return false;
    }
    return memcmp(data_, other.data_, info_.length()) == 0;
}

std::ostream& BufrField::printGrid(std::ostream& out) const {
    out << "bufr(grid)";
    return out;
}

bool BufrField::match(const std::string&, const std::string&) const {
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
