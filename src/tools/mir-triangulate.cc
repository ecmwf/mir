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

#include "eckit/log/Log.h"
#include "eckit/log/Timer.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "atlas/output/Gmsh.h"
#include "atlas/util/CoordinateEnums.h"

#include "mir/api/Atlas.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"



// For the Delaunay class:
#if 0
Copyright (c) 2015-2019 Simon Zeni (simonzeni@gmail.com)


Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:


The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.


THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
#endif

/**
 * @brief use of machine epsilon to compare floating-point values for equality
 * http://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
 */
bool almost_equal(const double x, const double y, int ulp=2)
{
    // the machine epsilon has to be scaled to the magnitude of the values used
    // and multiplied by the desired precision in ULPs (units in the last place)
    return std::abs(x-y) <= std::numeric_limits<double>::epsilon() * std::abs(x+y) * static_cast<double>(ulp)
           // unless the result is subnormal
           || std::abs(x-y) < std::numeric_limits<double>::min();
}

double half(const double x)
{
    return 0.5 * x;
}

struct Vector2
{
    Vector2() = default;
    Vector2(const Vector2 &v) = default;
    Vector2(Vector2&&) = default;
    Vector2(const double vx, const double vy);

    double dist2(const Vector2 &v) const;
    double dist(const Vector2 &v) const;
    double norm2() const;

    Vector2 &operator=(const Vector2&) = default;
    Vector2 &operator=(Vector2&&) = default;
    bool operator ==(const Vector2 &v) const;
    friend std::ostream &operator <<(std::ostream &str, const Vector2 &v);

    double x;
    double y;
};

Vector2::Vector2(const double vx, const double vy) :
    x(vx), y(vy)
{}

double
Vector2::dist2(const Vector2 &v) const
{
    const double dx = x - v.x;
    const double dy = y - v.y;
    return dx * dx + dy * dy;
}

double
Vector2::dist(const Vector2 &v) const
{
    return hypot(x - v.x, y - v.y);
}

double
Vector2::norm2() const
{
    return x * x + y * y;
}

bool
Vector2::operator ==(const Vector2 &v) const
{
    return (this->x == v.x) && (this->y == v.y);
}

std::ostream &
operator <<(std::ostream &str, const Vector2 &v)
{
    return str << "Point x: " << v.x << " y: " << v.y;
}

bool almost_equal(const Vector2 &v1, const Vector2 &v2, int ulp=2)
{
    return almost_equal(v1.x, v2.x, ulp) && almost_equal(v1.y, v2.y, ulp);
}

struct Edge
{
    using VertexType = Vector2;

    Edge() = default;
    Edge(const Edge&) = default;
    Edge(Edge&&) = default;
    Edge(const VertexType &v1, const VertexType &v2);

    Edge &operator=(const Edge&) = default;
    Edge &operator=(Edge&&) = default;
    bool operator ==(const Edge &e) const;
    friend std::ostream &operator <<(std::ostream &str, const Edge &e);

    const VertexType *v;
    const VertexType *w;
    bool isBad = false;
};

Edge::Edge(const VertexType &v1, const VertexType &v2) :
    v(&v1), w(&v2)
{}

bool
Edge::operator ==(const Edge &e) const
{
    return (*(this->v) == *e.v && *(this->w) == *e.w) ||
           (*(this->v) == *e.w && *(this->w) == *e.v);
}

std::ostream&
operator <<(std::ostream &str, const Edge &e)
{
    return str << "Edge " << *e.v << ", " << *e.w;
}

bool almost_equal(const Edge &e1, const Edge &e2)
{
    return	(almost_equal(*e1.v, *e2.v) && almost_equal(*e1.w, *e2.w)) ||
           (almost_equal(*e1.v, *e2.w) && almost_equal(*e1.w, *e2.v));
}

struct Triangle
{
    using EdgeType = Edge;
    using VertexType = Vector2;

    Triangle() = default;
    Triangle(const Triangle&) = default;
    Triangle(Triangle&&) = default;
    Triangle(const VertexType &v1, const VertexType &v2, const VertexType &v3);

    bool containsVertex(const VertexType &v) const;
    bool circumCircleContains(const VertexType &v) const;

    Triangle &operator=(const Triangle&) = default;
    Triangle &operator=(Triangle&&) = default;
    bool operator ==(const Triangle &t) const;
    friend std::ostream &operator <<(std::ostream &str, const Triangle &t);

    const VertexType *a;
    const VertexType *b;
    const VertexType *c;
    bool isBad = false;
};

Triangle::Triangle(const VertexType &v1, const VertexType &v2, const VertexType &v3) :
    a(&v1), b(&v2), c(&v3), isBad(false)
{}

bool
Triangle::containsVertex(const VertexType &v) const
{
    // return p1 == v || p2 == v || p3 == v;
    return almost_equal(*a, v) || almost_equal(*b, v) || almost_equal(*c, v);
}

bool
Triangle::circumCircleContains(const VertexType &v) const
{
    const double ab = a->norm2();
    const double cd = b->norm2();
    const double ef = c->norm2();

    const double ax = a->x;
    const double ay = a->y;
    const double bx = b->x;
    const double by = b->y;
    const double cx = c->x;
    const double cy = c->y;

    const double circum_x = (ab * (cy - by) + cd * (ay - cy) + ef * (by - ay)) / (ax * (cy - by) + bx * (ay - cy) + cx * (by - ay));
    const double circum_y = (ab * (cx - bx) + cd * (ax - cx) + ef * (bx - ax)) / (ay * (cx - bx) + by * (ax - cx) + cy * (bx - ax));

    const VertexType circum(half(circum_x), half(circum_y));
    const double circum_radius = a->dist2(circum);
    const double dist = v.dist2(circum);
    return dist <= circum_radius;
}

bool
Triangle::operator ==(const Triangle &t) const
{
    return	(*this->a == *t.a || *this->a == *t.b || *this->a == *t.c) &&
           (*this->b == *t.a || *this->b == *t.b || *this->b == *t.c) &&
           (*this->c == *t.a || *this->c == *t.b || *this->c == *t.c);
}

std::ostream&
operator <<(std::ostream &str, const Triangle &t)
{
    return str << "Triangle:" << "\n\t" <<
           *t.a << "\n\t" <<
           *t.b << "\n\t" <<
           *t.c << '\n';
}

bool almost_equal(const Triangle &t1, const Triangle &t2)
{
    return	(almost_equal(*t1.a , *t2.a) || almost_equal(*t1.a , *t2.b) || almost_equal(*t1.a , *t2.c)) &&
           (almost_equal(*t1.b , *t2.a) || almost_equal(*t1.b , *t2.b) || almost_equal(*t1.b , *t2.c)) &&
           (almost_equal(*t1.c , *t2.a) || almost_equal(*t1.c , *t2.b) || almost_equal(*t1.c , *t2.c));
}

class Delaunay
{
public:
    using TriangleType = Triangle;
    using EdgeType = Edge;
    using VertexType = Vector2;

    Delaunay() = default;
    Delaunay(const Delaunay&) = delete;
    Delaunay(Delaunay&&) = delete;

    const std::vector<TriangleType>& triangulate(std::vector<VertexType> &vertices);
    const std::vector<TriangleType>& getTriangles() const;
    const std::vector<EdgeType>& getEdges() const;
    const std::vector<VertexType>& getVertices() const;

    Delaunay& operator=(const Delaunay&) = delete;
    Delaunay& operator=(Delaunay&&) = delete;

private:
    std::vector<TriangleType> _triangles;
    std::vector<EdgeType> _edges;
    std::vector<VertexType> _vertices;
};

const std::vector<Delaunay::TriangleType>&
Delaunay::triangulate(std::vector<VertexType> &vertices)
{
    // Store the vertices locally
    _vertices = vertices;

    // Determinate the super triangle
    double minX = vertices[0].x;
    double minY = vertices[0].y;
    double maxX = minX;
    double maxY = minY;

    for(std::size_t i = 0; i < vertices.size(); ++i)
    {
        if (vertices[i].x < minX) minX = vertices[i].x;
        if (vertices[i].y < minY) minY = vertices[i].y;
        if (vertices[i].x > maxX) maxX = vertices[i].x;
        if (vertices[i].y > maxY) maxY = vertices[i].y;
    }

    const double dx = maxX - minX;
    const double dy = maxY - minY;
    const double deltaMax = std::max(dx, dy);
    const double midx = half(minX + maxX);
    const double midy = half(minY + maxY);

    const VertexType p1(midx - 20 * deltaMax, midy - deltaMax);
    const VertexType p2(midx, midy + 20 * deltaMax);
    const VertexType p3(midx + 20 * deltaMax, midy - deltaMax);

    // Create a list of triangles, and add the supertriangle in it
    _triangles.push_back(TriangleType(p1, p2, p3));

    for(auto p = begin(vertices); p != end(vertices); p++)
    {
        std::vector<EdgeType> polygon;

        for(auto & t : _triangles)
        {
            if(t.circumCircleContains(*p))
            {
                t.isBad = true;
                polygon.push_back(Edge{*t.a, *t.b});
                polygon.push_back(Edge{*t.b, *t.c});
                polygon.push_back(Edge{*t.c, *t.a});
            }
        }

        _triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [](TriangleType &t){
                             return t.isBad;
                         }), end(_triangles));

        for(auto e1 = begin(polygon); e1 != end(polygon); ++e1)
        {
            for(auto e2 = e1 + 1; e2 != end(polygon); ++e2)
            {
                if(almost_equal(*e1, *e2))
                {
                    e1->isBad = true;
                    e2->isBad = true;
                }
            }
        }

        polygon.erase(std::remove_if(begin(polygon), end(polygon), [](EdgeType &e){
                          return e.isBad;
                      }), end(polygon));

        for(const auto e : polygon)
            _triangles.push_back(TriangleType(*e.v, *e.w, *p));

    }

    _triangles.erase(std::remove_if(begin(_triangles), end(_triangles), [p1, p2, p3](TriangleType &t){
                         return t.containsVertex(p1) || t.containsVertex(p2) || t.containsVertex(p3);
                     }), end(_triangles));

    for(const auto t : _triangles)
    {
        _edges.push_back(Edge{*t.a, *t.b});
        _edges.push_back(Edge{*t.b, *t.c});
        _edges.push_back(Edge{*t.c, *t.a});
    }

    return _triangles;
}

const std::vector<Delaunay::TriangleType>&
Delaunay::getTriangles() const
{
    return _triangles;
}

const std::vector<Delaunay::EdgeType>&
Delaunay::getEdges() const
{
    return _edges;
}

const std::vector<Delaunay::VertexType>&
Delaunay::getVertices() const
{
    return _vertices;
}


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
        options_.push_back(new eckit::option::SimpleOption<bool>("alternate", "Alternate algorithm"));
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


void MIRTriangulate::execute(const eckit::option::CmdArgs& args) {
    using namespace atlas;
    auto& log = eckit::Log::info();

    std::string output = args.getString("output", "");
    bool alternate     = args.getBool("alternate", false);


    // loop over each file(s) message(s)
    for (size_t i = 0; i < args.count(); ++i) {
        mir::input::GribFileInput grib(args(i));
        while (grib.next()) {
            eckit::Timer tim(alternate ? "Delaunay triangulation (alternate)" : "Delaunay triangulation");

            // Build a mesh from grid
            mir::repres::RepresentationHandle rep(
                static_cast<const mir::input::MIRInput&>(grib).field().representation());

            if (alternate) {
                std::vector<Vector2> points;

                for (std::unique_ptr<mir::repres::Iterator> it(rep->iterator()); it->next();) {
                    auto& p(*(*it));
                    points.emplace_back(Vector2{p[0], p[1]});
                }

                auto triangles = Delaunay().triangulate(points);

                log << "---" "\nfile: " << args(i) << "\nnumberOfElements: " << triangles.size() << "\nelements:";

                for (auto& tri : triangles) {
                    log << "\n  - "
                        << (tri.a)->x << " " << (tri.a)->y << " "
                        << (tri.b)->x << " " << (tri.b)->y << " "
                        << (tri.c)->x << " " << (tri.c)->y;
                }

                log << std::endl;
                continue;
            }

            MeshGenerator generate("delaunay");
            Mesh mesh = generate(rep->atlasGrid());

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
            auto& connectivity = mesh.cells().node_connectivity();
            auto coord         = array::make_view<double, 2, atlas::array::Intent::ReadOnly>(mesh.nodes().lonlat());

            log << "---" "\nfile: " << args(i) << "\nnumberOfElements: " << connectivity.rows() << "\nelements:";

            for (idx_t e = 0; e < connectivity.rows(); ++e) {
                auto row = connectivity.row(e);
                ASSERT(row.size() == 3);
                log << "\n  - "
                    << coord(row(0), LAT) << " " << coord(row(0), LON) << " "
                    << coord(row(1), LAT) << " " << coord(row(1), LON) << " "
                    << coord(row(2), LAT) << " " << coord(row(2), LON);
            }

            log << std::endl;

        }
    }
}


int main(int argc, char** argv) {
    MIRTriangulate tool(argc, argv);
    return tool.start();
}
