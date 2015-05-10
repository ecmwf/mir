/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/lsm/NoneLSM.h"
#include <iostream>
#include "mir/lsm/Mask.h"

namespace mir {
namespace lsm {

class NoLSM : public Mask {
    virtual bool active() const { return false; }
    virtual bool cacheable() const { return false; }
    virtual void hash(eckit::MD5&) const {}
    virtual void print(std::ostream & out) const { out << "none"; }
  public:
    NoLSM(): Mask("none") {}
};

NoneLSM::NoneLSM(const std::string &name):
    LSMChooser(name) {
}


NoneLSM::~NoneLSM() {
}

Mask& NoneLSM::instance() {
    static NoLSM none;
    return none;
}

void NoneLSM::print(std::ostream& out) const {
    out << "NoneLSM[" << name_ << "]";
}

Mask *NoneLSM::create(const std::string &name,
                      const param::MIRParametrisation &param,
                      const atlas::Grid &grid) const {
    return new NoLSM();
}

std::string NoneLSM::cacheKey(const std::string &name,
                      const param::MIRParametrisation &param,
                      const atlas::Grid &grid) const {
    return "none";
}

namespace {
static NoneLSM input("none.input");
static NoneLSM output("none.output");

}


}  // namespace logic
}  // namespace mir

