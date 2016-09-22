/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#ifndef mir_tools_MIRCompare_h
#define mir_tools_MIRCompare_h

#include "eckit/memory/ScopedPtr.h"
#include "eckit/types/FloatCompare.h"
#include "mir/tools/MIRTool.h"


namespace mir {
namespace data {
class MIRField;
}
}


namespace mir {
namespace tools {


class MIRCompare : public MIRTool {
protected:

    // -- Overridden methods

    virtual void execute(const eckit::option::CmdArgs& args);

    virtual void usage(const std::string& tool) const;

    virtual int minimumPositionalArguments() const {
        return 2;
    }

    // -- Methods

    void compare(size_t n, mir::data::MIRField& field1, mir::data::MIRField& field2) const;
    void l2norm(size_t n, mir::data::MIRField& field1, mir::data::MIRField& field2) const;

    bool compare( double,  double) const;
    bool compare(const double *, const double *, size_t) const;

public:

    MIRCompare(int argc, char **argv);

private:

    // -- Members

    double user_absolute_;
    double user_relative_;
    double user_percent_;
    long   user_ulps_;
    double user_pack_factor_;
    bool   l2norm_;

    eckit::ScopedPtr< eckit::FloatApproxCompare<double> > real_same_;

};


}  // namespace tools
}  // namespace mir


#endif
