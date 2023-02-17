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


namespace mir::util {


class SpectralOrder {
public:
    // -- Exceptions
    // None

    // -- Constructors

    SpectralOrder()                     = default;
    SpectralOrder(const SpectralOrder&) = delete;

    // -- Destructor

    virtual ~SpectralOrder() = default;

    // -- Convertors
    // None

    // -- Operators

    void operator=(const SpectralOrder&) = delete;

    // -- Methods

    virtual long getTruncationFromGaussianNumber(long) const;
    virtual long getGaussianNumberFromTruncation(long) const;
    virtual void print(std::ostream&) const = 0;

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
    // None

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

    friend std::ostream& operator<<(std::ostream& s, const SpectralOrder& p) {
        p.print(s);
        return s;
    }
};


class SpectralOrderFactory {
    std::string name_;
    virtual SpectralOrder* make() = 0;

    SpectralOrderFactory(const SpectralOrderFactory&)            = delete;
    SpectralOrderFactory& operator=(const SpectralOrderFactory&) = delete;

protected:
    SpectralOrderFactory(const std::string&);
    virtual ~SpectralOrderFactory();

public:
    static SpectralOrder* build(const std::string&);
    static void list(std::ostream&);
};


template <class T>
class SpectralOrderBuilder : public SpectralOrderFactory {
    SpectralOrder* make() override { return new T(); }

public:
    SpectralOrderBuilder(const std::string& name) : SpectralOrderFactory(name) {}
};


}  // namespace mir::util
