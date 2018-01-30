/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eckit/log/BigNum.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/VectorOption.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/BoundingBox.h"


using namespace mir;


class MIRCount : public tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 1;
    }
public:
    MIRCount(int argc, char **argv) : tools::MIRTool(argc, argv) {

        //options_.push_back(new SimpleOption< bool >("sizes", "compare sizes of coordinates and values vectors, default false"));
        options_.push_back(new eckit::option::VectorOption<double>("area", "Specify the cropping area: north/west/south/east", 4));
    }
};


void MIRCount::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\nCount MIR representation number of values, compared to the GRIB numberOfValues."
            "\n"
            "\nUsage: " << tool << " [--area=N/W/S/E] [--angle-precision=<real>] file.grib [file.grib [...]]"
            "\nExamples:"
            "\n  % " << tool << " 1.grib"
            "\n  % " << tool << " --area=6/0/0/6 1.grib 2.grib"
            << std::endl;
}


template<class T>
static
T abs(const T& x) {
    if (x > 0) {
        return x;
    }
    return 0 - x;
}


template<class T>
static void put(const T& q) {
    size_t i = 0;

    for(auto e: q) {
        eckit::Log::info() << ' ' << e.first << " (" << e.second << ")";
        if (++i >= 2) {
            break;
        }
    }

}


void MIRCount::execute(const eckit::option::CmdArgs& args) {

    typedef std::pair<Latitude, Latitude> DistanceLat;
    typedef std::pair<Longitude, Longitude> DistanceLon;

    std::set<DistanceLat> nn;
    std::set<DistanceLat> ss;
    std::set<DistanceLon> ww;
    std::set<DistanceLon> ee;

    double anglePrecision = 0.;
    args.get("angle-precision", anglePrecision);

    util::BoundingBox bbox;

    if (args.has("area")) {
        std::vector<double> value;
        ASSERT(args.get("area", value));
        ASSERT(value.size() == 4);
        bbox = util::BoundingBox(value[0], value[1], value[2], value[3], anglePrecision);
    }

    eckit::Log::info() << bbox << std::endl;



    // count each file(s) message(s)
    for (size_t i = 0; i < args.count(); ++i) {
        eckit::Log::info() << args(i) << std::endl;

        input::GribFileInput grib(args(i));
        const input::MIRInput& input = grib;


        while (grib.next()) {

            data::MIRField field = input.field();
            ASSERT(field.dimensions() == 1);



            repres::RepresentationHandle rep(field.representation());
            eckit::ScopedPtr< repres::Iterator > it(rep->iterator());



            Latitude n = 0;
            Latitude s = 0;
            Longitude e = 0;
            Longitude w = 0;

            size_t count = 0;
            size_t values = 0;
            bool first = true;

            eckit::ScopedPtr<repres::Iterator> iter(rep->iterator());

            while (iter->next()) {
                const repres::Iterator::point_ll_t& point = iter->pointUnrotated();

                values++;

                nn.insert(DistanceLat(abs(bbox.north() - point.lat), point.lat));
                ss.insert(DistanceLat(abs(bbox.south() - point.lat), point.lat));

                ee.insert(DistanceLon(abs(bbox.east() - point.lon), point.lon));
                ww.insert(DistanceLon(abs(bbox.west() - point.lon), point.lon));

                // std::cout << point.lat << " " << point.lon << " ====> " << bbox.contains(point.lat, point.lon) << std::endl;

                if (bbox.contains(point.lat, point.lon)) {

                    const Latitude& lat = point.lat;
                    const Longitude lon = point.lon.normalise(bbox.west());

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

            eckit::Log::info() << eckit::BigNum(count)
                               << " out of "
                               << eckit::BigNum(values)
                               << ", north=" << n << " (bbox.n-n " << bbox.north() - n << ")"
                               << ", west=" << w << " (w-bbox.w " << w - bbox.west() << ")"
                               << ", south=" << s << " (s-bbox.s " << s - bbox.south() << ")"
                               << ", east=" << e << " (bbox.e -e " << bbox.east() - e  << ")"
                               << std::endl;

            eckit::Log::info() << "N " << bbox.north() << ":"; put(nn); eckit::Log::info() << std::endl;
            eckit::Log::info() << "W " << bbox.west() << ":"; put(ww); eckit::Log::info() << std::endl;
            eckit::Log::info() << "S " << bbox.south() << ":"; put(ss); eckit::Log::info() << std::endl;
            eckit::Log::info() << "E " << bbox.south() << ":"; put(ee); eckit::Log::info() << std::endl;


        }

    }

}


int main(int argc, char **argv) {
    MIRCount tool(argc, argv);
    return tool.start();
}

