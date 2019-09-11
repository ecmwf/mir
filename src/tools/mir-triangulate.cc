/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>
#include <deque>

#include "atlas/output/Gmsh.h"
#include "atlas/util/CoordinateEnums.h"

#include "eckit/geometry/Point2.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/api/Atlas.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/MeshGeneratorParameters.h"


class MIRTriangulate : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string& tool) const;
    int minimumPositionalArguments() const {
        return 0;
    }
public:
    MIRTriangulate(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("output", "Gmsh output file"));
    }
};


void MIRTriangulate::usage(const std::string &tool) const {
    eckit::Log::info() <<
                       "\nGRIB Delaunay triangulation."
                       "\n"
                       "\nUsage:"
                       "\n\t" << tool << " [--output=example.msh] file.grib [file.grib [...]]"
                       << std::endl;
}

class Segment {
    std::deque<eckit::geometry::Point2> points_;

public:
    Segment() {}

    explicit Segment(const eckit::geometry::Point2 start, eckit::geometry::Point2 end) {
        points_.push_back(start);
        points_.push_back(end);
    }

    const eckit::geometry::Point2& start() const { return points_.front(); }
    const eckit::geometry::Point2& end() const { return points_.back(); }

    bool operator<(const Segment& other) const {
        return points_ < other.points_;
    }

    bool operator==(const Segment& other) const {
        return points_ == other.points_;
    }

    bool merge(const Segment& other) {
        ASSERT(other.points_.size() == 2);

        if (other.end() == start()) {
            points_.push_front(other.start());
            return true;
        }

        if (other.start() == start()) {
            points_.push_front(other.end());
            return true;
        }

        if (other.end() == end()) {
            points_.push_back(other.start());
            return true;
        }

        if (other.start() == end()) {
            points_.push_back(other.end());
            return true;
        }

        return false;
    }


    friend std::ostream& operator<<(std::ostream& s, const Segment& p) {
        p.print(s);
        return s;
    }

    void print(std::ostream& out) const {
        const char *sep = "";
        for (auto p : points_) {
            out << sep << p;
            sep = " ";
        }
    }

};

static std::map<Segment, eckit::geometry::Point2> cache;

static eckit::geometry::Point2 middle(const eckit::geometry::Point2& p1, const eckit::geometry::Point2& p2) {
    // return eckit::geometry::Point2::middle(p1, p2);
    Segment s(p1, p2);
    auto j = cache.find(s);
    if (j == cache.end()) {
        auto p = eckit::geometry::Point2::middle(p1, p2);
        cache[Segment(p1, p2)] = p;
        cache[Segment(p2, p1)] = p;
        j = cache.find(s);
    }
    return (*j).second;
}

static bool connect(const eckit::geometry::Point2& p,
                    Segment& line,
                    const std::map<eckit::geometry::Point2, std::set<Segment > >& ends,
                    std::set<Segment >& segments
                   ) {

    auto j = ends.find(p);
    if (j != ends.end()) {
        const std::set<Segment>& q = (*j).second;

        std::set<Segment> i;
        std::set_intersection(q.begin(), q.end(), segments.begin(), segments.end(),
                              std::inserter(i, i.begin()));

        if (!i.empty()) {
            const Segment& s = *(i.begin());
            ASSERT(line.merge(s));
            segments.erase(s);
            return true;
        }
    }
    return false;
}



void MIRTriangulate::execute(const eckit::option::CmdArgs& args) {
    using namespace atlas;
    // auto& log = eckit::Log::info();

    std::string output = args.getString("output", "");
    bool alternate     = args.getBool("alternate", false);


    // loop over each file(s) message(s)
    for (size_t i = 0; i < args.count(); ++i) {

        mir::input::GribFileInput grib(args(i));
        const mir::input::MIRInput& input = grib;

        while (grib.next()) {
            // eckit::Timer tim(alternate ? "Delaunay triangulation (alternate)" : "Delaunay triangulation");

            auto field = input.field();
            double missingValue = field.missingValue();

            // Build a mesh from grid
            mir::repres::RepresentationHandle rep(field.representation());

            mir::util::MeshGeneratorParameters param;
            rep->fill(param);
            param.set("triangulate", true);


            MeshGenerator generate(param.meshGenerator_, param);
            Mesh mesh = generate(rep->atlasGrid());

            const auto& values = field.values(0);

            if (!output.empty()) {
                eckit::PathName path(output);

                for (size_t counter = 1; path.exists(); ++counter) {
                    std::ostringstream name;
                    name << output << "." << std::setw(4) << std::setfill('0') << counter;
                    path = name.str();
                }

                output::Gmsh(path).write(mesh);
            }


            // Write mesh connectivity
            const auto& connectivity = mesh.cells().node_connectivity();
            const auto coord         = array::make_view<double, 2, atlas::array::Intent::ReadOnly>(mesh.nodes().lonlat());

            // log << "---" "\nfile: " << args(i) << "\nnumberOfElements: " << connectivity.rows() << "\nelements:";

            eckit::geometry::Point2 pa, pb;

            std::map<eckit::geometry::Point2, std::set<Segment > > ends;

            std::set<Segment> segments;

            for (idx_t e = 0; e < connectivity.rows(); ++e) {
                const auto row = connectivity.row(e);

                size_t size = row.size();
                size_t missing = 0;

                ASSERT(size == 3);

                size_t n = 0;
                for (size_t i = 0; i < size; ++i) {
                    if (values[row(i)] == missingValue) {
                        missing ++;
                        n |= 1 << i;
                    }
                }

                if (missing == size || missing == 0) {
                    continue;
                }


                auto row0 = row(0);
                eckit::geometry::Point2 p0(coord(row0, LON), coord(row0, LAT));
                // double val0 = values[row0];

                auto row1 = row(1);
                eckit::geometry::Point2 p1(coord(row1, LON), coord(row1, LAT));
                // double val1 = values[row1];

                auto row2 = row(2);
                eckit::geometry::Point2 p2(coord(row2, LON), coord(row2, LAT));
                // double val2 = values[row2];

                switch (n) {

                case 1: // .xx
                case 6:
                    pa = middle(p0, p1);
                    pb = middle(p0, p2);
                    break;

                case 2: // .x.
                case 5:
                    pa = middle(p1, p0);
                    pb = middle(p1, p2);
                    break;

                case 3: // ..x
                case 4:
                    pa = middle(p2, p0);
                    pb = middle(p2, p1);
                    break;

                }

                Segment s(pa, pb);
                segments.insert(s);
                ends[pa].insert(s);
                ends[pb].insert(s);

            }

            // std::cerr << "segments " << segments.size() << std::endl;

            size_t count = 0;
            while (!segments.empty()) {
                Segment line = *segments.begin();
                segments.erase(segments.begin());

                bool more = true;
                while (more) {
                    more = false;
                    if (connect(line.start(), line, ends, segments)) {
                        more = true;
                    }

                    if (connect(line.end(), line, ends, segments)) {
                        more = true;
                    }
                }

                std::cout << line << std::endl;
                count++;
            }
            // std::cerr << "isolines " << count << std::endl;

        }
    }
}


int main(int argc, char** argv) {
    MIRTriangulate tool(argc, argv);
    return tool.start();
}
