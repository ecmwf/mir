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
/// @author Tiago Quintino
/// @date May 2015


#ifndef mir_method_Method_H
#define mir_method_Method_H

#include <iosfwd>
#include <string>
#include <vector>

#include "eckit/memory/NonCopyable.h"


namespace eckit {
class MD5;
}


namespace atlas {
class Grid;
}


namespace mir {
namespace context {
class Context;
}
namespace param {
class MIRParametrisation;
}

namespace util {
class MIRStatistics;
}
}


namespace mir {
namespace method {


class Method : private eckit::NonCopyable {
  public:

    Method(const param::MIRParametrisation &);

    virtual ~Method();

    virtual void hash( eckit::MD5 & ) const = 0;

    virtual void execute(context::Context&, const atlas::Grid& in, const atlas::Grid& out) const = 0;

  protected:

    const param::MIRParametrisation &parametrisation_;

    virtual void print(std::ostream &) const = 0;

  private:

    friend std::ostream &operator<<(std::ostream &s, const Method &p) {
        p.print(s);
        return s;
    }

};



class MethodFactory {
    std::string name_;
    virtual Method *make(const param::MIRParametrisation &) = 0;

  protected:

    MethodFactory(const std::string &);
    virtual ~MethodFactory();

  public:

    static void list(std::ostream &);
    static Method *build(const std::string &, const param::MIRParametrisation &);

};



template< class T>
class MethodBuilder : public MethodFactory {
    virtual Method *make(const param::MIRParametrisation &param) {
        return new T(param);
    }
  public:
    MethodBuilder(const std::string &name) : MethodFactory(name) {}
};



}  // namespace method
}  // namespace mir

#endif

