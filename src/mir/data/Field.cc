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


#include "mir/data/Field.h"

#include <algorithm>
#include <ostream>

#include "eckit/thread/AutoLock.h"
#include "eckit/types/Types.h"

#include "mir/data/MIRFieldStats.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace data {


Field::Field(const param::MIRParametrisation& param, bool hasMissing, double missingValue) :
    Field(repres::RepresentationFactory::build(param), hasMissing, missingValue) {}


Field::Field(const repres::Representation* repres, bool hasMissing, double missingValue) :
    missingValue_(missingValue), representation_(repres), recomputeHasMissing_(false), hasMissing_(hasMissing) {

    if (representation_ != nullptr) {
        representation_->attach();
    }
}


Field::Field(const Field& other) :
    values_(other.values_),
    metadata_(other.metadata_),
    handles_(other.handles_),
    missingValue_(other.missingValue_),
    representation_(other.representation_),
    recomputeHasMissing_(other.recomputeHasMissing_),
    hasMissing_(other.hasMissing_) {

    if (representation_ != nullptr) {
        representation_->attach();
    }
}


Field* Field::clone() const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    return new Field(*this);
}


// Warning: take ownership of values
void Field::update(MIRValuesVector& values, size_t which, bool recomputeHasMissing) {
    eckit::AutoLock<const eckit::Counted> lock(this);

    recomputeHasMissing_ = recomputeHasMissing;

    if (values_.size() <= which) {
        values_.resize(which + 1);
    }
    std::swap(values_[which], values);
}


size_t Field::dimensions() const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    return values_.size();
}


void Field::dimensions(size_t size) {
    eckit::AutoLock<const eckit::Counted> lock(this);
    metadata_.resize(size);
    values_.resize(size);
    handles_.clear();
}


void Field::select(size_t which) {
    eckit::AutoLock<const eckit::Counted> lock(this);
    ASSERT(which < values_.size());

    metadata_.resize(values_.size());

    if (which != 0) {
        std::swap(metadata_[0], metadata_[which]);
        std::swap(values_[0], values_[which]);
    }

    metadata_.resize(1);
    values_.resize(1);

    auto hit = handles_.find(which);
    if (hit != handles_.end()) {
        auto h = hit->second;
        handles_.clear();
        handles_[0] = h;
    }
    else {
        handles_.clear();
    }
}


Field::~Field() {
    if (representation_ != nullptr) {
        representation_->detach();
    }
}


void Field::print(std::ostream& out) const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    out << "Field[count=" << count() << ",";
    out << "dimensions=" << values_.size();
    if (hasMissing()) {
        out << ",missingValue=" << missingValue_;
    }

    if (representation_ != nullptr) {
        out << ",representation=" << *representation_;
    }

    if (!metadata_.empty()) {
        out << ",params=";
        char sep = '(';
        for (const auto& m : metadata_) {
            out << sep << m;
            sep = ',';
        }
        out << ')';
    }

    out << ",handles=";
    char sep = '(';
    for (auto h : handles_) {
        out << sep << h.first << "=>" << h.second;
        sep = ',';
    }
    out << ')';

    out << "]";
}


const repres::Representation* Field::representation() const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    ASSERT(representation_);
    return representation_;
}


void Field::validate() const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    if (representation_ != nullptr) {
        for (size_t i = 0; i < values_.size(); i++) {
            representation_->validate(values(i));
        }
    }
}


void Field::handle(size_t which, size_t handle) {
    eckit::AutoLock<const eckit::Counted> lock(this);

    ASSERT(which < dimensions());
    handles_[which] = handle;
}


size_t Field::handle(size_t which) const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    ASSERT(which < dimensions());
    auto hit = handles_.find(which);
    return hit != handles_.end() ? hit->second : which;
}


MIRFieldStats Field::statistics(size_t i) const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    if (hasMissing()) {
        const MIRValuesVector& vals = values(i);
        MIRValuesVector tmp;
        tmp.reserve(vals.size());
        size_t missing = 0;

        for (auto& value : vals) {
            if (value != missingValue_) {
                tmp.push_back(value);
            }
            else {
                missing++;
            }
        }
        return {tmp, missing};
    }
    return {values(i), 0};
}


void Field::representation(const repres::Representation* representation) {
    eckit::AutoLock<const eckit::Counted> lock(this);

    if (representation != nullptr) {
        representation->attach();
    }
    if (representation_ != nullptr) {
        representation_->detach();
    }
    representation_ = representation;
}


const MIRValuesVector& Field::values(size_t which) const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    ASSERT(which < values_.size());
    return values_[which];
}


MIRValuesVector& Field::direct(size_t which) {
    eckit::AutoLock<const eckit::Counted> lock(this);

    ASSERT(which < values_.size());
    return values_[which];
}


void Field::metadata(size_t which, const std::map<std::string, long>& md) {
    eckit::AutoLock<const eckit::Counted> lock(this);

    while (metadata_.size() <= which) {
        metadata_.emplace_back(std::map<std::string, long>());
    }
    metadata_[which] = md;
}


void Field::metadata(size_t which, const std::string& name, long value) {
    eckit::AutoLock<const eckit::Counted> lock(this);

    while (metadata_.size() <= which) {
        metadata_.emplace_back(std::map<std::string, long>());
    }
    metadata_[which][name] = value;
}


const std::map<std::string, long>& Field::metadata(size_t which) const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    if (metadata_.size() <= which) {
        static std::map<std::string, long> empty;
        return empty;
    }

    return metadata_[which];
}


bool Field::hasMissing() const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    // re-check for missing values if required
    if (recomputeHasMissing_) {
        recomputeHasMissing_ = false;
        hasMissing_          = false;
        for (const auto& v : values_) {
            if (v.front() == missingValue_ || v.back() == missingValue_ ||
                std::find(v.begin(), v.end(), missingValue_) != v.end()) {
                hasMissing_ = true;
                break;
            }
        }
    }

    return hasMissing_;
}


double Field::missingValue() const {
    eckit::AutoLock<const eckit::Counted> lock(this);

    return missingValue_;
}


void Field::hasMissing(bool on) {
    eckit::AutoLock<const eckit::Counted> lock(this);

    recomputeHasMissing_ = false;
    hasMissing_          = on;
}


void Field::missingValue(double value) {
    eckit::AutoLock<const eckit::Counted> lock(this);

    missingValue_ = value;
}


}  // namespace data
}  // namespace mir
