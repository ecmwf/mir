// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
