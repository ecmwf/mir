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


#include <iostream>

#include "mir/action/plan/ActionPlan.h"
#include "mir/action/io/Save.h"
#include "mir/action/io/Copy.h"

#include "mir/action/context/Context.h"
#include "mir/input/MIRInput.h"
#include "mir/style/MIRStyle.h"
#include "mir/output/MIROutput.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRDefaults.h"
#include "mir/log/MIR.h"

#include "mir/action/context/Context.h"
#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"


namespace mir {
namespace context {

namespace {
class MissingInput : public input::MIRInput
{
    virtual const param::MIRParametrisation& parametrisation(size_t which = 0) const {
        NOTIMP;
    }

    virtual bool sameAs(const MIRInput& other) const {
        NOTIMP;
    }

    virtual void print(std::ostream& out) const {
        out << "MissingInput[]";
    }

    virtual data::MIRField* field() const {
        NOTIMP;
    }

public:
    MissingInput() {}
    ~MissingInput() {}

};

static MissingInput missing;
}

static Context& c(Context* ctx) {
    ASSERT(ctx);
    return *ctx;
}

Context::Context(Context* parent):
    input_(c(parent).input()),
    statistics_(c(parent).statistics())  {

    field_.reset(new data::MIRField(&c(parent).field()));
}

Context::Context(mir::data::MIRField& field, mir::util::MIRStatistics& statistics):
    input_(missing),
    statistics_(statistics) {
    field_.reset(new data::MIRField(&field));
}


Context::Context(input::MIRInput &input,
                 util::MIRStatistics& statistics):
    input_(input),
    statistics_(statistics)  {

}

Context::~Context() {
}

input::MIRInput &Context::input() {
    return input_;
}

util::MIRStatistics& Context::statistics() {
    return statistics_;
}

data::MIRField& Context::field() {
    // TODO: Add a mutex
    if(!field_) {
        field_.reset(input_.field());
    }
    return *field_;
}

}  // namespace action
}  // namespace mir

