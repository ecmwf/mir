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


#ifndef mir_compare_Comparator_H
#define mir_compare_Comparator_H

#include <iosfwd>
#include <string>
#include "eckit/exception/Exceptions.h"
#include "eckit/memory/NonCopyable.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace context {
class Context;
}
namespace data {
class MIRField;
}
namespace param {
class MIRParametrisation;
}
namespace util {
class MIRStatistics;
}
}


namespace mir {
namespace compare {


class Comparator : public eckit::NonCopyable {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Comparator(const param::MIRParametrisation& param1, const param::MIRParametrisation& param2);

    // -- Destructor

    virtual ~Comparator(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual void execute(const data::MIRField&, const data::MIRField&) const = 0;

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

    virtual void print(std::ostream &) const = 0; // Change to virtual if base class

    template< typename T >
    T getSameParameter(const std::string& parameter) {
        T value1;
        T value2;

        ASSERT(parametrisation1_.get(parameter, value1));
        ASSERT(parametrisation2_.get(parameter, value2));

        ASSERT(value1 == value2);
        return value1;
    }

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    Comparator(const Comparator &);
    Comparator &operator=(const Comparator &);

    // -- Members

    const param::MIRParametrisation& parametrisation1_;
    const param::MIRParametrisation& parametrisation2_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Comparator &p) {
        p.print(s);
        return s;
    }

};


class ComparatorFactory {

    std::string name_;

    virtual Comparator *make(const param::MIRParametrisation&, const param::MIRParametrisation&) = 0;

  protected:

    ComparatorFactory(const std::string &);

    virtual ~ComparatorFactory();

  public:

    static Comparator *build(const std::string&, const param::MIRParametrisation&, const param::MIRParametrisation&);

    static void list(std::ostream&);

};


template<class T>
class ComparatorBuilder : public ComparatorFactory {
    virtual Comparator *make(const param::MIRParametrisation &param1, const param::MIRParametrisation& param2) {
        return new T(param1, param2);
    }
  public:
    ComparatorBuilder(const std::string &name) : ComparatorFactory(name) {}
};


}  // namespace compare
}  // namespace mir


#endif
