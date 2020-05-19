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


#ifndef mir_tools_MIRCount_h
#define mir_tools_MIRCount_h

#include <vector>

#include "mir/tools/MIRTool.h"


namespace mir {
namespace tools {


class MIRCount : public tools::MIRTool {
protected:
    virtual void execute(const eckit::option::CmdArgs&);
    virtual void usage(const std::string& tool) const;
    virtual int minimumPositionalArguments() const { return 0; }

    static void addOptions(std::vector<eckit::option::Option*>&);

public:
    MIRCount(int argc, char** argv);
    virtual ~MIRCount();
};


}  // namespace tools
}  // namespace mir


#endif
