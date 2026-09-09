// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>


namespace mir {
namespace data {
class MIRField;
}
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir::stats {


class Method {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Method(const param::MIRParametrisation&);
    Method(const Method&) = delete;

    // -- Destructor

    virtual ~Method();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Method&) = delete;

    // -- Methods

    virtual void resize(size_t)                  = 0;
    virtual void execute(const data::MIRField&)  = 0;
    virtual void mean(data::MIRField&) const     = 0;
    virtual void variance(data::MIRField&) const = 0;
    virtual void stddev(data::MIRField&) const   = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    const param::MIRParametrisation& parametrisation_;

    // -- Methods

    /// Output
    virtual void print(std::ostream&) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& out, const Method& r) {
        r.print(out);
        return out;
    }
};


class MethodFactory {
private:
    std::string name_;
    virtual Method* make(const param::MIRParametrisation&) = 0;

protected:
    MethodFactory(const std::string&);
    virtual ~MethodFactory();

public:
    MethodFactory(const MethodFactory&)            = delete;
    MethodFactory(MethodFactory&&)                 = delete;
    MethodFactory& operator=(const MethodFactory&) = delete;
    MethodFactory& operator=(MethodFactory&&)      = delete;

    static void list(std::ostream&);
    static Method* build(const std::string&, const param::MIRParametrisation&);
};


template <class T>
class MethodBuilder : public MethodFactory {
private:
    Method* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    MethodBuilder(const std::string& name) : MethodFactory(name) {}
};


}  // namespace mir::stats
