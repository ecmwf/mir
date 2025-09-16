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


#include "mir/method/FailMethod.h"

#include <string>

#include "eckit/log/JSON.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::method {


static const MethodBuilder<FailMethod> __method_1("fail");
static const MethodBuilder<FailMethod> __method_2("none");


bool FailMethod::canCrop() const {
    return false;
}


bool FailMethod::hasCropping() const {
    return false;
}


bool FailMethod::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const FailMethod*>(&other);
    return (o != nullptr);
}


const util::BoundingBox& FailMethod::getCropping() const {
    NOTIMP;
}


void FailMethod::execute(context::Context& /*unused*/, const repres::Representation& /*in*/,
                         const repres::Representation& /*out*/) const {
    std::string msg = "FailMethod: this interpolation method fails intentionally";
    Log::error() << msg << std::endl;
    throw exception::UserError(msg);
}


const char* FailMethod::type() const {
    return "fail";
}


void FailMethod::hash(eckit::MD5& /*unused*/) const {}


int FailMethod::version() const {
    NOTIMP;
}


void FailMethod::print(std::ostream& out) const {
    out << "FailMethod[]";
}


void FailMethod::json(eckit::JSON& out) const {
    out << type();
}


void FailMethod::setCropping(const util::BoundingBox& /*unused*/) {
    NOTIMP;
}


}  // namespace mir::method
