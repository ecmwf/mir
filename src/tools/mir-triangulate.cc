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


#include <algorithm>
#include <deque>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "atlas/output/Gmsh.h"
#include "atlas/util/CoordinateEnums.h"

#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Types.h"


using namespace mir;


struct MIRTriangulate : tools::MIRTool {
    MIRTriangulate(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("output", "Gmsh output file"));
    }

    int minimumPositionalArguments() const override { return 0; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nGRIB Delaunay triangulation."
                       "\n"
                       "\nUsage:"
                       "\n\t"
                    << tool << " [--output=example.msh] file.grib [file.grib [...]]" << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


class Segment {
    std::deque<Point2> points_;
    Point2 inside_;

public:
    // Segment() {}

    explicit Segment(const Point2& start, const Point2& end, const Point2& inside) : inside_(inside) {
        points_.push_back(start);
        points_.push_back(end);
    }

    const Point2& start() const { return points_.front(); }
    const Point2& end() const { return points_.back(); }

    bool inside(const Point2& pt) const {
        bool in  = false;
        double y = pt.y();
        double x = pt.x();

        auto i = points_.begin();
        auto j = i + 1;

        for (; j != points_.end(); ++i, ++j) {
            const auto& p1 = *i;
            const auto& p2 = *j;

            if (y <= std::min(p1.y(), p2.y())) {
                continue;
            }

            if (y > std::max(p1.y(), p2.y())) {
                continue;
            }

            if (x > std::max(p1.x(), p2.x())) {
                continue;
            }

            if (p1.y() == p2.y()) {
                continue;
            }

            double xints = (y - p1.y()) * (p2.x() - p1.x()) / (p2.y() - p1.y()) + p1.x();
            if ((p2.x() == p1.x()) || (x <= xints)) {
                in = !in;
            }
        }

        return in;
    }


    bool ccw() const {
        // Assumes closest poly have first == last point
        auto i = points_.begin();
        auto j = i + 1;

        double order = 0;
        for (; j != points_.end(); ++i, ++j) {
            const auto& p1 = *i;
            const auto& p2 = *j;
            order += (p2.x() - p1.x()) * (p2.y() + p1.y());
        }

        bool swapped = inside(inside_);
        // return order > 0;
        return swapped ? (order > 0) : (order < 0);
    }

    bool operator<(const Segment& other) const { return points_ < other.points_; }

    bool operator==(const Segment& other) const { return points_ == other.points_; }

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


    friend eckit::JSON& operator<<(eckit::JSON& s, const Segment& q) {
        q.json(s);
        return s;
    }

    void json(eckit::JSON& json) const {
        json.startObject();
        json << "points";
        json.startList();
        for (auto q : points_) {
            json.startList();
            json << q.x();
            json << q.y();
            json.endList();
        }
        json.endList();
        json << "ccw" << ccw();
        json.endObject();
    }
};

static std::map<Segment, Point2> cache;

static Point2 middle(const Point2& p1, const Point2& p2) {
    return Point2::middle(p1, p2);
    // Segment s(p1, p2);
    // auto j = cache.find(s);
    // if (j == cache.end()) {
    //     auto q = Point2::middle(p1, p2);
    //     cache[Segment(p1, p2)] = q;
    //     cache[Segment(p2, p1)] = q;
    //     j = cache.find(s);
    // }
    // return j->second;
}

static bool connect(const Point2& q, Segment& line, const std::map<Point2, std::set<Segment> >& ends,
                    std::set<Segment>& segments) {

    auto j = ends.find(q);
    if (j != ends.end()) {
        const std::set<Segment>& q = j->second;

        std::set<Segment> i;
        std::set_intersection(q.begin(), q.end(), segments.begin(), segments.end(), std::inserter(i, i.begin()));

        if (!i.empty()) {
            const Segment& s = *(i.begin());
            ASSERT(line.merge(s));
            segments.erase(s);
            return true;
        }
    }
    return false;
}

#if 0
static void p(int n, Point2 p0, double val0, Point2 p1, double val1,
              Point2 p2, double val2) {


    std::cout << n << " " << val0 << " " << val1 << " " << val2 << std::endl;
    return;
    std::cout << "                    [" << n << "]" << std::endl;

    if (p0.y() > p1.y() && p0.y() > p2.y()) {
        std::cout << "            " << p0 << " (" << val0 << ")" << std::endl;
        if (p1.x() < p2.x()) {
            std::cout << p1 << " (" << val1 << ") --- " << p2 << " (" << val2 << ")" << std::endl;
        }
        else {
            std::cout << p2 << " (" << val2 << ") --- " << p1 << " (" << val1 << ")" << std::endl;
        }
        return;
    }


    if (p0.y() > p1.y() && p0.y() == p2.y()) {
        if (p0.x() < p2.x()) {
            std::cout << p0 << " (" << val0 << ") --- " << p2 << " (" << val2 << ")" << std::endl;
        }
        else {
            std::cout << p2 << " (" << val2 << ") --- " << p0 << " (" << val0 << ")" << std::endl;
        }
        std::cout << "            " << p1 << " (" << val1 << ")" << std::endl;

        return;
    }

    std::cout << "+++ " << p0.y() << "  " << p1.y() << " " << p2.y() << std::endl;

    NOTIMP;
}
#endif

void MIRTriangulate::execute(const eckit::option::CmdArgs& args) {
    std::string output = args.getString("output", "");
    // bool alternate     = args.getBool("alternate", false);


    // loop over each file(s) message(s)
    for (size_t a = 0; a < args.count(); ++a) {
        std::unique_ptr<input::MIRInput> input(new input::GribFileInput(args(a)));

        while (input->next()) {
            // trace::Timer tim(alternate ? "Delaunay triangulation (alternate)" : "Delaunay triangulation");

            auto field          = input->field();
            double missingValue = field.missingValue();

            // Build a mesh from grid
            repres::RepresentationHandle rep(field.representation());

            util::MeshGeneratorParameters param;
            rep->fill(param);
            param.set("triangulate", true);


            atlas::MeshGenerator generate(param.meshGenerator_, param);
            auto mesh = generate(rep->atlasGrid());

            const auto& values = field.values(0);

            if (!output.empty()) {
                eckit::PathName path(output);

                for (size_t counter = 1; path.exists(); ++counter) {
                    std::ostringstream name;
                    name << output << "." << std::setw(4) << std::setfill('0') << counter;
                    path = name.str();
                }

                atlas::output::Gmsh(path).write(mesh);
            }


            // Write mesh connectivity
            const auto& connectivity = mesh.cells().node_connectivity();
            const auto coord         = atlas::array::make_view<double, 2>(mesh.nodes().lonlat());


            Point2 pa;
            Point2 pb;

            std::map<Point2, std::set<Segment> > ends;

            std::set<Segment> segments;

            for (atlas::idx_t e = 0; e < connectivity.rows(); ++e) {
                const auto row = connectivity.row(e);

                auto size      = size_t(row.size());
                size_t missing = 0;

                ASSERT(size == 3);

                size_t n = 0;
                for (size_t i = 0; i < size; ++i) {
                    if (values[size_t(row(i))] == missingValue) {
                        missing++;
                        n |= 1 << i;
                    }
                }

                if (missing == size || missing == 0) {
                    continue;
                }


                auto row0 = row(0);
                Point2 p0(coord(row0, LLCOORDS::LON), coord(row0, LLCOORDS::LAT));
                // double val0 = values[row0];

                auto row1 = row(1);
                Point2 p1(coord(row1, LLCOORDS::LON), coord(row1, LLCOORDS::LAT));
                // double val1 = values[row1];

                auto row2 = row(2);
                Point2 p2(coord(row2, LLCOORDS::LON), coord(row2, LLCOORDS::LAT));
                // double val2 = values[row2];
                // p(n, p0, val0, p1, val1, p2, val2);

                Point2 inside;

                switch (n) {

                    case 1:  // missing = p0
                        pa     = middle(p0, p1);
                        pb     = middle(p0, p2);
                        inside = p0;
                        break;

                    case 6:  // missing = p2

                        pa     = middle(p0, p2);
                        pb     = middle(p0, p1);
                        inside = p2;
                        break;

                    case 2:  // missing = p1

                        pa     = middle(p1, p0);
                        pb     = middle(p1, p2);
                        inside = p1;
                        break;

                    case 5:  // missing = p0, p2
                        pa     = middle(p1, p2);
                        pb     = middle(p1, p0);
                        inside = p0;
                        break;

                    case 3:  // missing = p0, p1
                        pa     = middle(p2, p0);
                        pb     = middle(p2, p1);
                        inside = p0;
                        break;

                    case 4:  // missing = p2
                        pa     = middle(p2, p1);
                        pb     = middle(p2, p0);
                        inside = p2;
                        break;
                }

                Segment s(pa, pb, inside);
                segments.insert(s);
                ends[pa].insert(s);
                ends[pb].insert(s);
            }


            eckit::JSON json(std::cout);
            json.startList();

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

                json << line;

                count++;
            }
            json.endList();
        }
    }
}


int main(int argc, char** argv) {
    MIRTriangulate tool(argc, argv);
    return tool.start();
}
