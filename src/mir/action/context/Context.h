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


#ifndef Context_H
#define Context_H

#include <string>

#include "eckit/memory/ScopedPtr.h"
#include "eckit/memory/NonCopyable.h"

#include "mir/param/SimpleParametrisation.h"
#include "eckit/thread/Mutex.h"

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

class Context  {


public:

    // -- Exceptions
    // None

    // -- Contructors

    Context();
    Context(const Context& other);
    Context& operator=(const Context& other);

    // Context(Context*);

    Context(input::MIRInput &input,
            util::MIRStatistics& statistics);

    Context(data::MIRField &field,
            util::MIRStatistics& statistics);
    // -- Destructor

    ~Context();

    // -- Convertors
    // None

    // -- Operators


    // -- Methods

    Context& push();
    Context pop();

    util::MIRStatistics& statistics();
    data::MIRField& field();
    input::MIRInput& input();

    // Select only one field
    void select(size_t which);

    void scalar(double);
    double scalar() const;

    bool isField() const;
    bool isScalar() const;

    std::vector<Context> stack_;

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

    void print(std::ostream& s) const;


    // -- Overridden methods


    // -- Class members
    // None

    // -- Class methods
    // None

private:


    // -- Members

    mutable eckit::Mutex mutex_;

    input::MIRInput &input_;
    util::MIRStatistics& statistics_;
    eckit::ScopedPtr<Content> content_;

    // -- Methods


    // -- Overridden methods

    // From MIRParametrisation


    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends


    friend std::ostream &operator<<(std::ostream &s, const Context &p) {
        p.print(s);
        return s;
    }

};

}  // namespace action
}  // namespace mir
#endif

