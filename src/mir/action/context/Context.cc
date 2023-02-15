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


#include "mir/action/context/Context.h"

#include <ostream>
#include <sstream>

#include "eckit/thread/AutoLock.h"

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/util/Exceptions.h"
#include "mir/util/MIRStatistics.h"


namespace mir::context {


namespace {
class MissingInput final : public input::MIRInput {
    const param::MIRParametrisation& parametrisation(size_t /*which*/ = 0) const override { NOTIMP; }

    bool sameAs(const MIRInput& /*unused*/) const override { NOTIMP; }

    void print(std::ostream& out) const override { out << "MissingInput[]"; }

    data::MIRField field() const override { NOTIMP; }

public:
    MissingInput() = default;
};
}  // namespace


static MissingInput missing;
static util::MIRStatistics stats;


class Content {

    virtual void print(std::ostream&) const = 0;

public:
    Content()          = default;
    virtual ~Content() = default;

    Content(const Content&)            = delete;
    Content(Content&&)                 = delete;
    Content& operator=(const Content&) = delete;
    Content& operator=(Content&&)      = delete;

    virtual data::MIRField& field() {
        std::ostringstream oss;
        oss << "Cannot get field from " << *this;
        throw exception::SeriousBug(oss.str());
    }

    virtual double scalar() const {
        std::ostringstream oss;
        oss << "Cannot get field from " << *this;
        throw exception::SeriousBug(oss.str());
    }

    virtual Extension& extension() const {
        std::ostringstream oss;
        oss << "Cannot get extension from " << *this;
        throw exception::SeriousBug(oss.str());
    }

    virtual bool isField() const { return false; }

    virtual bool isScalar() const { return false; }

    virtual bool isExtension() const { return false; }

    virtual Content* clone() const = 0;

    friend std::ostream& operator<<(std::ostream& s, const Content& p) {
        p.print(s);
        return s;
    }
};


class ScalarContent : public Content {

    double value_;

    void print(std::ostream& out) const override { out << "ScalarContent[value=" << value_ << "]"; }

    double scalar() const override { return value_; }

    bool isScalar() const override { return true; }

    Content* clone() const override { return new ScalarContent(value_); }

public:
    ScalarContent(double value) : value_(value) {}
};


class FieldContent : public Content {
    data::MIRField field_;

    data::MIRField& field() override { return field_; }

    void print(std::ostream& out) const override { out << "FieldContent[field=" << field_ << "]"; }

    bool isField() const override { return true; }

    Content* clone() const override { return new FieldContent(field_); }

public:
    FieldContent(const data::MIRField& field) : field_(field) {}
};

class ExtensionContent : public Content {

    std::unique_ptr<Extension> extension_;

    void print(std::ostream& out) const override { out << "ExtensionContent[" << *extension_ << "]"; }

    bool isExtension() const override { return true; }

    Extension& extension() const override { return *extension_; }

    Content* clone() const override { return new ExtensionContent(extension_->clone()); }

public:
    ExtensionContent(Extension* extension) : extension_(extension) { ASSERT(extension_); }
};


Context::Context() : input_(missing), statistics_(stats), content_(nullptr) {}


Context::Context(const Context& other) : input_(other.input_), statistics_(other.statistics_), content_(nullptr) {
    eckit::AutoLock<const Context> lock(other);
    if (other.content_) {
        content_.reset(other.content_->clone());
    }
}


Context::Context(data::MIRField& field, util::MIRStatistics& statistics) :
    input_(missing), statistics_(statistics), content_(new FieldContent(field)) {}


Context::Context(input::MIRInput& input, util::MIRStatistics& statistics) :
    input_(input), statistics_(statistics), content_(nullptr) {}


Context::~Context() = default;


bool Context::isField() const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);
    return content_ ? content_->isField() : false;
}


bool Context::isScalar() const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);
    return content_ ? content_->isScalar() : false;
}


bool Context::isExtension() const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);
    return content_ ? content_->isExtension() : false;
}


input::MIRInput& Context::input() {
    return input_;
}


void Context::field(data::MIRField& other) {
    content_ = std::make_unique<FieldContent>(other);
}


util::MIRStatistics& Context::statistics() {
    return statistics_;
}


data::MIRField& Context::field() {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    if (!content_) {
        auto timer(statistics().gribDecodingTimer());
        content_ = std::make_unique<FieldContent>(input_.field());
    }
    return content_->field();
}

Extension& Context::extension() {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(isExtension());
    return content_->extension();
}


void Context::extension(Extension* e) {
    content_ = std::make_unique<ExtensionContent>(e);
}


void Context::select(size_t which) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    field().select(which);
}


void Context::scalar(double value) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    content_ = std::make_unique<ScalarContent>(value);
}


double Context::scalar() const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(content_);
    return content_->scalar();
}


void Context::print(std::ostream& out) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    out << "Context[content=";
    if (content_) {
        out << *content_;
    }
    else {
        out << "null";
    }
    out << "]";
}


Context& Context::push() {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    stack_.emplace_back(Context(*this));
    return stack_.back();
}


Context Context::pop() {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(!stack_.empty());
    Context ctx = stack_.back();
    stack_.pop_back();
    return ctx;
}


void Context::lock() const {
    mutex_.lock();
}


void Context::unlock() const {
    mutex_.unlock();
}


}  // namespace mir::context
