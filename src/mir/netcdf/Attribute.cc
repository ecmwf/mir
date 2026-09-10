// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/Attribute.h"

#include <ostream>

#include "mir/netcdf/Endowed.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Value.h"
#include "mir/util/Log.h"


namespace mir::netcdf {

Attribute::Attribute(Endowed& owner, const std::string& name, Value* value) :
    owner_(owner), name_(name), value_(value) {}

Attribute::~Attribute() {
    delete value_;
}

void Attribute::dump(std::ostream& out) const {
    out << "\t\t" << fullName() << " = ";
    value_->dump(out);
    out << " ;" << std::endl;
}

bool Attribute::sameAs(const Attribute& other) const {
    return name_ == other.name_;
}

std::string Attribute::fullName() const {
    return owner_.name() + ':' + name_;
}

std::string Attribute::asString() const {
    return value_->asString();
}

const std::string& Attribute::name() const {
    return name_;
}

const Value& Attribute::value() const {
    ASSERT(value_ != nullptr);
    return *value_;
}

void Attribute::invalidate() {
    Log::info() << __func__ << " " << *this << std::endl;
    NOTIMP;
}

void Attribute::merge(const Attribute& /*unused*/) {
    Log::info() << __func__ << " " << *this << std::endl;
    NOTIMP;
}

void Attribute::clone(Endowed& /*unused*/) const {
    Log::info() << __func__ << " " << *this << std::endl;
    NOTIMP;
}

void Attribute::create(int /*unused*/) const {
    Log::info() << __func__ << " " << *this << std::endl;
    NOTIMP;
}

}  // namespace mir::netcdf
