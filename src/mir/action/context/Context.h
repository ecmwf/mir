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


#pragma once

#include <memory>
#include <vector>

#include "mir/util/Mutex.h"


namespace mir {
namespace input {
class MIRInput;
}
namespace util {
class MIRStatistics;
}
namespace data {
class MIRField;
}
namespace context {
class Content;
}
}  // namespace mir


namespace mir::context {

class Extension {
public:
    virtual ~Extension()                    = default;
    virtual Extension* clone() const        = 0;
    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const Extension& p) {
        p.print(s);
        return s;
    }
};

class Context {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Context();
    Context(const Context&);
    Context& operator=(const Context&);

    Context(input::MIRInput&, util::MIRStatistics&);
    Context(data::MIRField&, util::MIRStatistics&);

    // -- Destructor

    ~Context();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    Context& push();
    Context pop();

    util::MIRStatistics& statistics();
    data::MIRField& field();
    input::MIRInput& input();
    Extension& extension();

    // Substitute context's field
    void field(data::MIRField&);

    // Extension
    void extension(Extension*);

    // Select only one field
    void select(size_t which);

    void scalar(double);
    double scalar() const;

    bool isField() const;
    bool isScalar() const;
    bool isExtension() const;

    void lock() const;
    void unlock() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    mutable util::recursive_mutex mutex_;
    std::vector<Context> stack_;

    input::MIRInput& input_;
    util::MIRStatistics& statistics_;
    std::unique_ptr<Content> content_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Context& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::context
