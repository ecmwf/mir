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
#include <vector>

#include "eckit/memory/Counted.h"


namespace mir {
namespace netcdf {
class Variable;
}
}  // namespace mir


namespace mir {
namespace netcdf {


class Codec : public eckit::Counted {
public:
    Codec();
    ~Codec() override;

    Codec(const Codec&)          = delete;
    void operator=(const Codec&) = delete;

    virtual void decode(std::vector<double>&) const;
    virtual void decode(std::vector<float>&) const;
    virtual void decode(std::vector<long>&) const;
    virtual void decode(std::vector<short>&) const;
    virtual void decode(std::vector<unsigned char>&) const;
    virtual void decode(std::vector<long long>&) const;

    virtual void encode(std::vector<double>&) const;
    virtual void encode(std::vector<float>&) const;
    virtual void encode(std::vector<long>&) const;
    virtual void encode(std::vector<short>&) const;
    virtual void encode(std::vector<unsigned char>&) const;
    virtual void encode(std::vector<long long>&) const;

    virtual void addAttributes(Variable&) const;
    virtual void updateAttributes(int nc, int varid, const std::string& path);

    virtual bool timeAxis() const;

private:
    // -- Methods
    virtual void print(std::ostream&) const = 0;

    // -- Friends
    friend std::ostream& operator<<(std::ostream& out, const Codec& v) {
        v.print(out);
        return out;
    }
};


class CodecFactory {

    std::string name_;
    virtual Codec* make(const Variable&) = 0;

    CodecFactory(const CodecFactory&)            = delete;
    CodecFactory& operator=(const CodecFactory&) = delete;

protected:
    CodecFactory(const std::string&);

    virtual ~CodecFactory();

public:
    static Codec* build(const std::string&, const Variable&);

    static void list(std::ostream&);
};


template <class T>
class CodecBuilder : public CodecFactory {
    Codec* make(const Variable& variable) override { return new T(variable); }

public:
    CodecBuilder(const std::string& name) : CodecFactory(name) {}
};


}  // namespace netcdf
}  // namespace mir
