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

#include <iosfwd>
#include <string>


namespace eckit {
class JSON;
class MD5;
}  // namespace eckit

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
}  // namespace mir


namespace mir::method {


class Method {
public:
    explicit Method(const param::MIRParametrisation&);

    Method(const Method&) = delete;
    Method(Method&&)      = delete;

    virtual ~Method();

    void operator=(const Method&) = delete;
    void operator=(Method&&)      = delete;

    virtual const char* type() const         = 0;
    virtual void hash(eckit::MD5&) const     = 0;
    virtual bool sameAs(const Method&) const = 0;
    virtual int version() const              = 0;

    virtual void execute(context::Context&, const repres::Representation& in,
                         const repres::Representation& out) const = 0;

    // For optimising plan
    virtual bool canCrop() const                         = 0;
    virtual void setCropping(const util::BoundingBox&)   = 0;
    virtual bool hasCropping() const                     = 0;
    virtual const util::BoundingBox& getCropping() const = 0;

    void json(eckit::JSON&, bool lookupKnownMethods) const;

    std::string json_str(bool lookupKnownMethods = true) const;

protected:
    const param::MIRParametrisation& parametrisation_;

    virtual void print(std::ostream&) const = 0;
    virtual void json(eckit::JSON&) const   = 0;

private:
    bool getKnownName(std::string&) const;

    friend std::ostream& operator<<(std::ostream& s, const Method& m) {
        m.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const Method& m) {
        m.json(s);
        return s;
    }
};


class MethodFactory {
    std::string name_;
    virtual Method* make(const param::MIRParametrisation&) = 0;

protected:
    explicit MethodFactory(const std::string&);
    virtual ~MethodFactory();

public:
    MethodFactory(const MethodFactory&) = delete;
    MethodFactory(MethodFactory&&)      = delete;

    MethodFactory& operator=(const MethodFactory&) = delete;
    MethodFactory& operator=(MethodFactory&&)      = delete;

    static void list(std::ostream&);
    static Method* build(const std::string&, const param::MIRParametrisation&);

    [[nodiscard]] static Method* make_from_string(const std::string&);
};


template <class T>
class MethodBuilder : public MethodFactory {
    Method* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    explicit MethodBuilder(const std::string& name) : MethodFactory(name) {}
};


}  // namespace mir::method
