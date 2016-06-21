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
#include "eckit/exception/Exceptions.h"

#include "mir/action/context/Context.h"
#include "mir/input/MIRInput.h"
#include "mir/style/MIRStyle.h"
#include "mir/output/MIROutput.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRDefaults.h"
#include "mir/log/MIR.h"
#include "mir/util/MIRStatistics.h"

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
static util::MIRStatistics stats;

}

class Content {
    virtual void print(std::ostream &) const = 0; // Change to virtual if base class

public:
    Content() {}
    virtual ~Content() {}

    virtual data::MIRField& field() {
        std::ostringstream oss;
        oss << "Cannot get field from " << *this;
        throw eckit::SeriousBug(oss.str());
    }

    virtual double scalar() const {
        std::ostringstream oss;
        oss << "Cannot get field from " << *this;
        throw eckit::SeriousBug(oss.str());
    }

    friend std::ostream &operator<<(std::ostream &s, const Content &p) {
        p.print(s);
        return s;
    }
};

class ScalarContent : public Content {

    double value_;

    virtual void print(std::ostream& out) const {
        out << "ScalarContent[value=" << value_ << "]";
    }

    virtual double scalar() const {
        return value_;
    }

public:

    ScalarContent(double value): value_(value) {}

};

class FieldContent : public Content {
    eckit::ScopedPtr<data::MIRField> field_;

    data::MIRField& field() {
        return *field_;
    }

    virtual void print(std::ostream& out) const {
        out << "FieldContent[field=" << *field_ << "]";
    }

public:
    FieldContent(data::MIRField* field):
        field_(field) { ASSERT(field); }


};

static Context& c(Context* ctx) {
    ASSERT(ctx);
    return *ctx;
}

Context::Context():
    input_(missing),
    statistics_(stats) {

}

Context::Context(Context* parent):
    input_(c(parent).input()),
    statistics_(c(parent).statistics())  {

    content_.reset(new FieldContent(new data::MIRField(&c(parent).field())));

}

Context::Context(mir::data::MIRField& field, mir::util::MIRStatistics& statistics):
    input_(missing),
    statistics_(statistics) {
    content_.reset(new FieldContent(new data::MIRField(&field)));
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
    if (!content_) {
        std::cout << "Context -> allocate field from " << input_ << std::endl;
        content_.reset(new FieldContent(input_.field()));
    }
    return content_->field();
}

void Context::scalar(double value) {
    content_.reset(new ScalarContent(value));
}

double Context::scalar() const {
    ASSERT(content_);
    return content_->scalar();
}

}  // namespace action
}  // namespace mir

