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


#include "mir/data/MIRField.h"

#include <ostream>

#include "mir/data/Field.h"
#include "mir/data/MIRFieldStats.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace data {


MIRField::MIRField(const param::MIRParametrisation& param, bool hasMissing, double missingValue) :
    field_(new Field(param, hasMissing, missingValue)) {
    field_->attach();
}


MIRField::MIRField(const repres::Representation* repres, bool hasMissing, double missingValue) :
    field_(new Field(repres, hasMissing, missingValue)) {
    field_->attach();
}


MIRField::MIRField(const MIRField& other) : field_(other.field_) {
    field_->attach();
}


void MIRField::copyOnWrite() {
    if (field_->count() > 1) {
        // Log::info() << "XXXX copyOnWrite " << *field_ << std::endl;
        Field* f = field_->clone();
        field_->detach();
        field_ = f;
        field_->attach();
    }
}


// Warning: take ownership of values
void MIRField::update(MIRValuesVector& values, size_t which, bool recomputeHasMissing) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    // Log::info() << "MIRField::update " << *field_ << std::endl;

    copyOnWrite();
    field_->update(values, which, recomputeHasMissing);
}


size_t MIRField::dimensions() const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    return field_->dimensions();
}


void MIRField::dimensions(size_t size) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    copyOnWrite();
    field_->dimensions(size);
}


void MIRField::select(size_t which) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);
    // TODO: Check the if we can select() without copying everything first
    copyOnWrite();
    field_->select(which);
}


MIRField::~MIRField() {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    field_->detach();
}


void MIRField::print(std::ostream& out) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    out << *field_;
}


const repres::Representation* MIRField::representation() const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    return field_->representation();
}


void MIRField::validate() const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    field_->validate();
}


void MIRField::handle(size_t which, size_t handle) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    field_->handle(which, handle);
}


size_t MIRField::handle(size_t which) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    return field_->handle(which);
}


MIRFieldStats MIRField::statistics(size_t i) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    return field_->statistics(i);
}


void MIRField::representation(const repres::Representation* representation) {
    // Log::info() << "MIRField::representation " << *field_ << " => " << *representation << std::endl;
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    copyOnWrite();
    field_->representation(representation);
}


const MIRValuesVector& MIRField::values(size_t which) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    return field_->values(which);
}


MIRValuesVector& MIRField::direct(size_t which) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    // Log::info() << "MIRField::direct " << *field_ << std::endl;
    copyOnWrite();
    return field_->direct(which);
}


void MIRField::metadata(size_t which, const std::map<std::string, long>& md) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    // Log::info() << "MIRField::paramId " << *field_ << std::endl;

    copyOnWrite();
    field_->metadata(which, md);
}

void MIRField::metadata(size_t which, const std::string& name, long value) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    // Log::info() << "MIRField::paramId " << *field_ << std::endl;

    copyOnWrite();
    field_->metadata(which, name, value);
}

const std::map<std::string, long>& MIRField::metadata(size_t which) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    return field_->metadata(which);
}


bool MIRField::hasMissing() const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    return field_->hasMissing();
}


double MIRField::missingValue() const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    return field_->missingValue();
}


void MIRField::hasMissing(bool on) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    if (on != field_->hasMissing()) {
        copyOnWrite();
        field_->hasMissing(on);
    }
}


void MIRField::missingValue(double value) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    if (value != missingValue()) {
        copyOnWrite();
        field_->missingValue(value);
    }
}


static util::once_flag once;
static util::recursive_mutex* local_mutex      = nullptr;
static std::map<std::string, FieldFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, FieldFactory*>();
}


FieldFactory::FieldFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


FieldFactory::~FieldFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


void FieldFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


MIRField* FieldFactory::build(const std::string& name, const param::MIRParametrisation& params, bool hasMissing,
                              double missingValue) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "FieldFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "FieldFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("FieldFactory: unknown '" + name + "'");
    }

    return j->second->make(params, hasMissing, missingValue);
}


}  // namespace data
}  // namespace mir
