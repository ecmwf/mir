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


#include "mir/key/grid/NamedORCA.h"

#include <memory>
#include <ostream>

#include "eckit/geo/Grid.h"
#include "eckit/geo/spec/Custom.h"

#include "mir/repres/ORCA.h"
#include "mir/util/Exceptions.h"


namespace mir::key::grid {


NamedORCA::NamedORCA(const std::string& name) : NamedGrid(name) {}


void NamedORCA::print(std::ostream& out) const {
    out << "NamedORCA[key=" << key_ << "]";
}


const repres::Representation* NamedORCA::representation() const {
    auto user = std::make_unique<eckit::geo::spec::Custom>();
    user->set("grid", key_);

    std::unique_ptr<eckit::geo::Spec> spec(eckit::geo::GridFactory::make_spec(*user));
    return new repres::ORCA(spec->get_string("uid"));
}


const repres::Representation* NamedORCA::representation(const util::Rotation& /*unused*/) const {
    NOTIMP;
}


size_t NamedORCA::gaussianNumber() const {
    return default_gaussian_number();
}


}  // namespace mir::key::grid
