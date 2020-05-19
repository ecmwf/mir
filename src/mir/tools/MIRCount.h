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

#include <set>
#include <vector>

#include "mir/tools/MIRTool.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Types.h"


namespace eckit {
class JSON;
}


namespace mir {
namespace tools {


class MIRCount : public tools::MIRTool {
public:
    struct counter_t {
        using DistanceLat = std::pair<Latitude, Latitude>;
        using DistanceLon = std::pair<Longitude, Longitude>;

        counter_t(const util::BoundingBox&);
        counter_t(std::vector<double>& area);

        void insert(const PointLatLon&);
        void json(eckit::JSON&, bool enclose = true) const;

        const util::BoundingBox bbox;
        bool first;

        size_t count;
        size_t values;

        Latitude n;
        Latitude s;
        Longitude e;
        Longitude w;

        std::set<DistanceLat> nn;
        std::set<DistanceLat> ss;
        std::set<DistanceLon> ww;
        std::set<DistanceLon> ee;
    };

protected:
    virtual void execute(const eckit::option::CmdArgs&);
    virtual void usage(const std::string& tool) const;
    virtual int minimumPositionalArguments() const { return 0; }

    static void addOptions(std::vector<eckit::option::Option*>&);
    static void countOnNamedGrid(counter_t&, std::string grid);
    static void countOnGridIncrements(counter_t&, std::vector<double> grid);

public:
    MIRCount(int argc, char** argv);
    virtual ~MIRCount();
};


}  // namespace tools
}  // namespace mir


#endif
