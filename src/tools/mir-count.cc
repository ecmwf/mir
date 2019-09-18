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
#include "mir/util/Pretty.h"


class MIRCount : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 0;
    }
public:
    MIRCount(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using eckit::option::SimpleOption;
        using eckit::option::VectorOption;

        //options_.push_back(new SimpleOption< bool >("sizes", "compare sizes of coordinates and values vectors, default false"));
        options_.push_back(new VectorOption<double>("area", "cropping area (North/West/South/East)", 4));
        options_.push_back(new SimpleOption<std::string>("gridname", "grid name: [FNOfno][1-9][0-9]*"));
        options_.push_back(new VectorOption<double>("grid", "regular grid increments (West-East/South-North)", 2));
        options_.push_back(new SimpleOption<bool>("ni-nj", "output number of increments in longitude/latitude (Ni:Nj)"));
    }
};


void MIRCount::usage(const std::string &tool) const {
    eckit::Log::info() <<
            "\nCount MIR representation number of values, compared to the GRIB numberOfValues."
            "\n"
            "\nUsage:"
            "\n\t" << tool << " [--area=N/W/S/E] file.grib [file.grib [...]]"
            "\n\t" << tool << " [--area=N/W/S/E] --grid=WE/SN"
            "\n\t" << tool << " [--area=N/W/S/E] --grid=WE/SN --ni-nj"
            "\n\t" << tool << " [--area=N/W/S/E] --gridname=[FNOfno][1-9][0-9]*  # ..."
            "\n"
            "\n" "Examples:"
            "\n\t" "% " << tool << " 1.grib"
            "\n\t" "% " << tool << " --area=6/0/0/6 1.grib 2.grib"
            "\n\t" "% " << tool << " --area=6/0/0/6 --gridname=O1280"
            "\n\t" "% " << tool << " --area=6/0/0/6 --grid=1/1 --ni-nj"
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


using mir::Latitude;
using mir::Longitude;
using DistanceLat = std::pair<Latitude, Latitude>;
using DistanceLon = std::pair<Longitude, Longitude>;


size_t countRepresentationInBoundingBox(
        const mir::repres::Representation& rep,
        const mir::util::BoundingBox& bbox,
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

    std::unique_ptr<mir::repres::Iterator> iter(rep.iterator());

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
            << mir::util::Pretty(count) << " out of " << mir::util::Pretty(values)
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

    std::vector<double> value;

    mir::util::BoundingBox bbox;
    if (args.get("area", value)) {
        ASSERT(value.size() == 4);
        bbox = mir::util::BoundingBox(value[0], value[1], value[2], value[3]);
    }


    // setup a regular lat/lon representation and perfom count
    if (args.get("grid", value)) {
        ASSERT(!args.has("gridname"));
        ASSERT(value.size() == 2);
        mir::util::Increments grid(value[0], value[1]);

        if (args.has("ni-nj")) {
            // Note: this *does not crop*, it is a "local" representation
            mir::repres::latlon::RegularLL rep(grid, bbox, {bbox.south(), bbox.west()});
            eckit::Log::info() << rep.Ni() << ":" << rep.Nj() << std::endl;
            return;
        }

        mir::repres::latlon::RegularLL rep(grid);
        countRepresentationInBoundingBox(rep, bbox, nn, ww, ss, ee);
        return;
    }


    // setup a representation from gridname and perfom count
    std::string gridname;
    if (args.get("gridname", gridname)) {
        ASSERT(!args.has("grid"));

        mir::repres::RepresentationHandle rep(mir::namedgrids::NamedGrid::lookup(gridname).representation());
        countRepresentationInBoundingBox(*rep, bbox, nn, ww, ss, ee);
        return;
    }


    // count each file(s) message(s)
    for (size_t i = 0; i < args.count(); ++i) {
        eckit::Log::info() << args(i) << std::endl;

        mir::input::GribFileInput grib(args(i));
        while (grib.next()) {

            mir::data::MIRField field = static_cast<const mir::input::MIRInput&>(grib).field();
            ASSERT(field.dimensions() == 1);

            mir::repres::RepresentationHandle rep(field.representation());
            countRepresentationInBoundingBox(*rep, bbox, nn, ww, ss, ee);
        }
    }
}


int main(int argc, char **argv) {
    MIRCount tool(argc, argv);
    return tool.start();
}

