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


namespace mir::netcdf {
class MergePlan;
}  // namespace mir::netcdf


namespace mir::netcdf {

class Step {
public:
    Step() = default;

    Step(const Step&) = delete;
    Step(Step&&)      = delete;

    virtual ~Step() = default;

    void operator=(const Step&) = delete;
    void operator=(Step&&)      = delete;

    virtual int rank() const         = 0;
    virtual void execute(MergePlan&) = 0;
    virtual bool merge(Step*);

private:
    virtual void print(std::ostream&) const = 0;
    friend std::ostream& operator<<(std::ostream& s, const Step& v) {
        v.print(s);
        return s;
    }
};


struct CompareSteps {
    bool operator()(const Step* a, const Step* b) const { return a->rank() > b->rank(); }
};


}  // namespace mir::netcdf
