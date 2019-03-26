/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <memory>

#include "eckit/log/BigNum.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/VectorOption.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"


using namespace mir;


class MIRCount : public tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 0;
    }
public:
    MIRCount(int argc, char **argv) : tools::MIRTool(argc, argv) {
        using namespace eckit::option;

        //options_.push_back(new SimpleOption< bool >("sizes", "compare sizes of coordinates and values vectors, default false"));
        options_.push_back(new VectorOption<double>("area", "cropping area: North/West/South/East", 4));
        options_.push_back(new VectorOption<double>("grid", "regular latitude/longitude grid: West-East/South-North increments", 2));
        options_.push_back(new SimpleOption<std::string>("gridname", "grid name: [FNOfno][1-9][0-9]*"));
    }
};


void MIRCount::usage(const std::string &tool) const {
    eckit::Log::info() <<
            "\nCount MIR representation number of values, compared to the GRIB numberOfValues."
            "\n"
            "\nUsage:"
            "\n\t" << tool << " [--area=N/W/S/E] file.grib [file.grib [...]]     # from GRIB file(s)"
            "\n\t" << tool << " [--area=N/W/S/E] --grid=WE/SN                    # specify grid"
            "\n\t" << tool << " [--area=N/W/S/E] --gridname=[FNOfno][1-9][0-9]*  # ..."
            "\n"
            "\nExamples:"
            "\n  % " << tool << " 1.grib"
            "\n  % " << tool << " --area=6/0/0/6 1.grib 2.grib"
            "\n  % " << tool << " --area=6/0/0/6 --gridname=O1280"
            << std::endl;
}


template<class T>
std::ostream& operator<<(std::ostream& s, const std::set< std::pair< T, T > >& x) {
    size_t i = 0;
    for (auto e: x) {
        s << ' ' << e.first << " (" << e.second << ")";
        if (++i >= 2) {
            break;
        }
    }
    return s;
}


using DistanceLat = std::pair<Latitude, Latitude>;
using DistanceLon = std::pair<Longitude, Longitude>;


size_t countRepresentationInBoundingBox(
        const repres::Representation& rep,
        const util::BoundingBox& bbox,
        std::set<DistanceLat>& nn,
        std::set<DistanceLon>& ww,
        std::set<DistanceLat>& ss,
        std::set<DistanceLon>& ee) {
    Latitude n = 0;
    Latitude s = 0;
    Longitude e = 0;
    Longitude w = 0;

    size_t count = 0;
    size_t values = 0;
    bool first = true;

    std::unique_ptr<repres::Iterator> iter(rep.iterator());

    while (iter->next()) {
        const auto& point = iter->pointUnrotated();

        values++;

        nn.insert(DistanceLat(bbox.north().distance(point.lat()), point.lat()));
        ss.insert(DistanceLat(bbox.south().distance(point.lat()), point.lat()));

        ee.insert(DistanceLon(bbox.east().distance(point.lon()), point.lon()));
        ww.insert(DistanceLon(bbox.west().distance(point.lon()), point.lon()));

        // std::cout << point.lat << " " << point.lon << " ====> " << bbox.contains(point.lat, point.lon) << std::endl;

        if (bbox.contains(point)) {

            const Latitude& lat = point.lat();
            const Longitude lon = point.lon().normalise(bbox.west());

            if (first) {
                n = s = lat;
                e = w = lon;
                first = false;
            } else {
                if (n < lat) { n = lat; }
                if (s > lat) { s = lat; }
                if (e < lon) { e = lon; }
                if (w > lon) { w = lon; }
            }


            count++;

        }
    }

    eckit::Log::info()
            << eckit::BigNum(count) << " out of " << eckit::BigNum(values)
            << ", north=" << n << " (bbox.n - n " << bbox.north() - n << ")"
            << ", west="  << w << " (w - bbox.w " << w - bbox.west()  << ")"
            << ", south=" << s << " (s - bbox.s " << s - bbox.south() << ")"
            << ", east="  << e << " (bbox.e - e " << bbox.east() - e  << ")"
            << "\n" "N " << bbox.north() << ":" << nn
            << "\n" "W " << bbox.west()  << ":" << ww
            << "\n" "S " << bbox.south() << ":" << ss
            << "\n" "E " << bbox.east() << ":" << ee
            << std::endl;

    return count;
}


void MIRCount::execute(const eckit::option::CmdArgs& args) {

    std::set<DistanceLat> nn;
    std::set<DistanceLat> ss;
    std::set<DistanceLon> ww;
    std::set<DistanceLon> ee;

    util::BoundingBox bbox;
    std::vector<double> value;
    std::string name;

    if (args.get("area", value)) {
        ASSERT(value.size() == 4);
        bbox = util::BoundingBox(value[0], value[1], value[2], value[3]);
    }

    eckit::Log::info() << bbox << std::endl;


    // setup a regular lat/lon representation and perfom count
    if (args.get("grid", value)) {
        ASSERT(!args.has("gridname"));
        ASSERT(value.size() == 2);

        repres::RepresentationHandle rep(new repres::latlon::RegularLL(
                                             util::Increments(value[0], value[1]) ));
        countRepresentationInBoundingBox(*rep, bbox, nn, ww, ss, ee);
        return;
    }


    // setup a representation from gridname and perfom count
    if (args.get("gridname", name)) {
        ASSERT(!args.has("grid"));

        repres::RepresentationHandle rep(namedgrids::NamedGrid::lookup(name).representation());
        countRepresentationInBoundingBox(*rep, bbox, nn, ww, ss, ee);
        return;
    }


    // count each file(s) message(s)
    for (size_t i = 0; i < args.count(); ++i) {
        eckit::Log::info() << args(i) << std::endl;

        input::GribFileInput grib(args(i));
        while (grib.next()) {

            data::MIRField field = static_cast<const input::MIRInput&>(grib).field();
            ASSERT(field.dimensions() == 1);

            repres::RepresentationHandle rep(field.representation());
            countRepresentationInBoundingBox(*rep, bbox, nn, ww, ss, ee);
        }
    }
}


int main(int argc, char **argv) {
    MIRCount tool(argc, argv);
    return tool.start();
}

