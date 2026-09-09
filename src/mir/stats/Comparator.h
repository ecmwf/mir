// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

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


class Comparator {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Comparator(const param::MIRParametrisation&, const param::MIRParametrisation&);
    Comparator(const Comparator&) = delete;

    // -- Destructor

    virtual ~Comparator();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Comparator&) = delete;

    // -- Methods

    virtual std::string execute(const data::MIRField&, const data::MIRField&) = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    const param::MIRParametrisation& parametrisation1_;
    const param::MIRParametrisation& parametrisation2_;

    // -- Methods

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

    friend std::ostream& operator<<(std::ostream& out, const Comparator& r) {
        r.print(out);
        return out;
    }
};


class ComparatorFactory {
    std::string name_;
    virtual Comparator* make(const param::MIRParametrisation&, const param::MIRParametrisation&) = 0;

protected:
    ComparatorFactory(const std::string&);
    virtual ~ComparatorFactory();

public:
    ComparatorFactory(const ComparatorFactory&)            = delete;
    ComparatorFactory(ComparatorFactory&&)                 = delete;
    ComparatorFactory& operator=(const ComparatorFactory&) = delete;
    ComparatorFactory& operator=(ComparatorFactory&&)      = delete;

    static void list(std::ostream&);
    static Comparator* build(const std::string&, const param::MIRParametrisation&, const param::MIRParametrisation&);
};


template <class T>
class ComparatorBuilder : public ComparatorFactory {
    Comparator* make(const param::MIRParametrisation& param1, const param::MIRParametrisation& param2) override {
        return new T(param1, param2);
    }

public:
    ComparatorBuilder(const std::string& name) : ComparatorFactory(name) {}
};


}  // namespace mir::stats
