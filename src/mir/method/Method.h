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


#ifndef mir_method_Method_h
#define mir_method_Method_h

#include <iosfwd>
#include <string>


namespace eckit {
class MD5;
}

namespace mir {
namespace context {
class Context;
}
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
namespace util {
class BoundingBox;
}
}


namespace mir {
namespace method {


class Method {
public:

    Method(const param::MIRParametrisation&);
    Method(const Method&) = delete;

    virtual ~Method();

    void operator=(const Method&) = delete;

    virtual void hash(eckit::MD5&) const = 0;

    virtual void execute(context::Context&,
                         const repres::Representation& in,
                         const repres::Representation& out) const = 0;

    virtual bool sameAs(const Method& other) const = 0;

    // For optimising plan
    virtual bool canCrop() const = 0;
    virtual void setCropping(const util::BoundingBox&) = 0;
    virtual bool hasCropping() const = 0;
    virtual const util::BoundingBox& getCropping() const = 0;

protected:

    const param::MIRParametrisation& parametrisation_;

    virtual void print(std::ostream&) const = 0;

private:

    friend std::ostream& operator<<(std::ostream& s, const Method& p) {
        p.print(s);
        return s;
    }

};


class MethodFactory {
    std::string name_;
    virtual Method *make(const param::MIRParametrisation&) = 0;

protected:

    MethodFactory(const std::string&);
    virtual ~MethodFactory();

public:

    static void list(std::ostream&);
    static Method *build(const std::string&, const param::MIRParametrisation&);

};


template< class T>
class MethodBuilder : public MethodFactory {
    virtual Method *make(const param::MIRParametrisation& param) {
        return new T(param);
    }
public:
    MethodBuilder(const std::string& name) : MethodFactory(name) {}
};


}  // namespace method
}  // namespace mir


#endif

