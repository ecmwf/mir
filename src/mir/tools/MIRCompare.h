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

#include "mir/tools/MIRTool.h"


namespace mir {
namespace tools {


class MIRCompare : public MIRTool {
protected:

    virtual void execute(const eckit::option::CmdArgs& args);

    virtual int numberOfPositionalArguments() const {
        return 2;
    }

    virtual void usage(const std::string& tool) const;

public:

    MIRCompare(int argc, char **argv);

};


}  // namespace tools
}  // namespace mir


#endif
