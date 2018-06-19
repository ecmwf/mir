/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/util/PointSearch.h"

#include <iostream>
#include <unistd.h>

#include "eckit/config/Resource.h"
#include "eckit/container/KDTree.h"
#include "eckit/container/kdtree/KDNode.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Plural.h"
#include "eckit/log/Timer.h"
#include "eckit/os/AutoUmask.h"
#include "eckit/os/Semaphore.h"
#include "eckit/runtime/Main.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace util {


//----------------------------------------------------------------------------------------------------------------------


const long VERSION = 1;

PointSearchTree::~PointSearchTree() = default;

void PointSearchTree::build(std::vector<PointValueType>&) {
    std::ostringstream os;
    os << "PointSearchTree::build() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void PointSearchTree::insert(const PointValueType&) {
    std::ostringstream os;
    os << "PointSearchTree::insert() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void PointSearchTree::statsPrint(std::ostream&, bool) {
    std::ostringstream os;
    os << "PointSearchTree::statsPrint() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void PointSearchTree::statsReset() {
    std::ostringstream os;
    os << "PointSearchTree::statsReset() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


PointSearchTree::PointValueType PointSearchTree::nearestNeighbour(const Point&) {
    std::ostringstream os;
    os << "PointSearchTree::nearestNeighbour() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

std::vector<PointSearchTree::PointValueType> PointSearchTree::kNearestNeighbours(const Point&, size_t) {
    std::ostringstream os;
    os << "PointSearchTree::kNearestNeighbours() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

std::vector<PointSearchTree::PointValueType> PointSearchTree::findInSphere(const Point&, double) {
    std::ostringstream os;
    os << "PointSearchTree::findInSphere() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

bool PointSearchTree::ready() const {
    std::ostringstream os;
    os << "PointSearchTree::ready() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void PointSearchTree::commit() {
    std::ostringstream os;
    os << "PointSearchTree::commit() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}

void PointSearchTree::print(std::ostream& out) const {
    out << "PointSearchTree[]" << std::endl;
}

void PointSearchTree::lock() {
    // Empty
}

void PointSearchTree::unlock() {
    // Empty
}

//----------------------------------------------------------------------------------------------------------------------


class PointSearchTreeMemory: public PointSearchTree {

    typedef PointSearchTree::Point Point;
    typedef eckit::KDTreeMemory<PointSearchTree> Tree;

    Tree tree_;

    virtual void build(std::vector<PointValueType>& v) {
        tree_.build(v);
    }

    virtual void insert(const PointValueType& pt) {
        tree_.insert(pt);
    }

    virtual void statsPrint(std::ostream& out, bool pretty) {
        tree_.statsPrint(out, pretty);
    }

    virtual void statsReset() {
        tree_.statsReset();
    }

    virtual PointValueType nearestNeighbour(const PointSearchTree::Point& pt) {
        const auto& nn = tree_.nearestNeighbour(pt).value();
        return PointValueType(nn.point(), nn.payload());
    }

    virtual std::vector<PointValueType> kNearestNeighbours(const Point& pt, size_t k) {
        std::vector<PointValueType> result;
        for (const auto& n : tree_.kNearestNeighbours(pt, k)) {
            result.push_back(PointValueType(n.point(), n.payload()));
        }
        return result;
    }

    virtual std::vector<PointValueType> findInSphere(const Point& pt, double radius) {
        std::vector<PointValueType> result;
        for (const auto& n : tree_.findInSphere(pt, radius)) {
            result.push_back(PointValueType(n.point(), n.payload()));
        }
        return result;
    }

    virtual bool ready() const  {
        return false;
    }

    virtual void commit() {
    }

    virtual void print(std::ostream& out)  const {
        out << "KDTreeMemory[]";
    }

public:
    PointSearchTreeMemory( const repres::Representation&,
                           const param::MIRParametrisation&,
                           size_t itemCount) {}
};

static PointSearchTreeBuilder<PointSearchTreeMemory> builder1("memory");


//----------------------------------------------------------------------------------------------------------------------


class PointSearchTreeMapped: public PointSearchTree {


    typedef PointSearchTree::Point Point;
    typedef eckit::KDTreeMapped<PointSearchTree> Tree;

protected:
    eckit::AutoUmask umask_; // Must be first
    eckit::PathName path_;
    Tree tree_;

public:

    virtual void build(std::vector<PointValueType>& v) {
        tree_.build(v);
    }

    virtual void insert(const PointValueType& pt) {
        tree_.insert(pt);
    }

    virtual void statsPrint(std::ostream& out, bool pretty) {
        tree_.statsPrint(out, pretty);
    }

    virtual void statsReset() {
        tree_.statsReset();
    }

    virtual PointValueType nearestNeighbour(const PointSearchTree::Point& pt) {
        const auto& nn = tree_.nearestNeighbour(pt).value();
        return PointValueType(nn.point(), nn.payload());
    }

    virtual std::vector<PointValueType> kNearestNeighbours(const Point& pt, size_t k) {
        std::vector<PointValueType> result;
        for (const auto& n : tree_.kNearestNeighbours(pt, k)) {
            result.push_back(PointValueType(n.point(), n.payload()));
        }
        return result;
    }

    virtual std::vector<PointValueType> findInSphere(const Point& pt, double radius) {
        std::vector<PointValueType> result;
        for (const auto& n : tree_.findInSphere(pt, radius)) {
            result.push_back(PointValueType(n.point(), n.payload()));
        }
        return result;
    }


public:
    PointSearchTreeMapped(const eckit::PathName& path, size_t itemCount):
        umask_(0),
        path_(path),
        tree_(path, path.exists() ? 0 : itemCount, 0) {
    }

};


//----------------------------------------------------------------------------------------------------------------------


template<class T>
class PointSearchTreeMappedFile : public PointSearchTreeMapped {

protected:
    eckit::PathName real_;
    eckit::Semaphore lock_; // Must be after real

    virtual bool ready() const  {
        return path_ == real_;
    }

    virtual void commit() {
        eckit::PathName::rename(path_, real_);
    }

    virtual  void lock() {
        eckit::AutoUmask umask(0);

        eckit::PathName path = lockFile(real_);

        eckit::Log::debug<LibMir>() << "Wait for lock " << path << std::endl;
        lock_.lock();
        eckit::Log::debug<LibMir>() << "Got lock " << path << std::endl;


        std::string hostname = eckit::Main::hostname();

        std::ofstream os(path.asString().c_str());
        os << hostname << " " << ::getpid() << std::endl;
    }

    virtual void unlock() {
        eckit::PathName path = lockFile(real_);

        eckit::Log::debug<LibMir>() << "Unlock " << path << std::endl;
        std::ofstream os(path.asString().c_str());
        os << std::endl;
        lock_.unlock();
    }


    static eckit::PathName treePath(const repres::Representation& r,
                                    const param::MIRParametrisation& param,
                                    bool makeUnique) {

        // LocalPathName::unique calls mkdir, make sure it uses umask = 0
        eckit::AutoUmask umask(0);

        eckit::PathName p = T::path(r, param);
        if (makeUnique && !p.exists()) {
            p = eckit::PathName::unique(p);
        }

        return p;

    }

    static eckit::PathName lockFile(const std::string& path) {
        eckit::AutoUmask umask(0);

        eckit::PathName lock(path + ".lock");
        lock.touch();
        return lock;
    }


public:

    PointSearchTreeMappedFile( const repres::Representation& r,
                               const param::MIRParametrisation& param,
                               size_t itemCount) :
        PointSearchTreeMapped(treePath(r, param, true), itemCount),
        real_(treePath(r, param, false)),
        lock_(lockFile(real_))
    {

        lockFile(real_).touch();

        if (ready()) {
            eckit::Log::info() << "Loading " << *this << std::endl;
        }
    }
};

//===============================================================================================================

class PointSearchTreeMappedCacheFile : public PointSearchTreeMappedFile<PointSearchTreeMappedCacheFile> {
    virtual void print(std::ostream& out) const  {
        out << "PointSearchTreeMappedCacheFile[" << real_ << "]";
    }

public:

    static eckit::PathName path(const repres::Representation& r,
                                const param::MIRParametrisation&) {

        std::ostringstream oss;
        oss  << LibMir::cacheDir()
             << "/mir/trees/"
             << VERSION
             << "/"
             << r.uniqueName()
             << ".kdtree";

        return oss.str();

    }

public:
    PointSearchTreeMappedCacheFile( const repres::Representation& r,
                                    const param::MIRParametrisation& param,
                                    size_t itemCount):
        PointSearchTreeMappedFile<PointSearchTreeMappedCacheFile>(r, param, itemCount) {
    }
};

static PointSearchTreeBuilder<PointSearchTreeMappedCacheFile> builder2("mapped-cache-file");

//===============================================================================================================

class PointSearchTreeMappedTempFile : public PointSearchTreeMappedFile<PointSearchTreeMappedTempFile> {
    virtual void print(std::ostream& out) const  {
        out << "PointSearchTreeMappedTempFile[" << real_ << "]";
    }


public:
    static eckit::PathName path(const repres::Representation& r,
                                const param::MIRParametrisation&) {

        std::ostringstream oss;
        oss  << "/tmp/"
             << r.uniqueName()
             << "-"
             << VERSION
             << ".kdtree";

        return oss.str();

    }


public:
    PointSearchTreeMappedTempFile( const repres::Representation& r,
                                   const param::MIRParametrisation& param,
                                   size_t itemCount):
        PointSearchTreeMappedFile<PointSearchTreeMappedTempFile>(r, param, itemCount) {}
};

static PointSearchTreeBuilder<PointSearchTreeMappedTempFile> builder3("mapped-temporary-file");

//===============================================================================================================


class PointSearchTreeMappedDevZero: public PointSearchTreeMapped {

    virtual bool ready() const  {
        return false;
    }

    virtual void commit() {
    }

    virtual void print(std::ostream& out) const  {
        out << "PointSearchTreeMappedDevZero[]";
    }

public:

    PointSearchTreeMappedDevZero( const repres::Representation&,
                                  const param::MIRParametrisation&,
                                  size_t itemCount):
        PointSearchTreeMapped("/dev/zero", itemCount) {}
};


static PointSearchTreeBuilder<PointSearchTreeMappedDevZero> builder4("mapped-anonymous-memory");


PointSearch::PointSearch(const param::MIRParametrisation& parametrisation,
                         const repres::Representation& r):
    parametrisation_(parametrisation)
{
    const size_t npts = r.numberOfPoints();
    ASSERT(npts > 0);

    tree_.reset(PointSearchTreeFactory::build(r, parametrisation, npts));

    eckit::AutoLock<PointSearchTree> lock(*tree_);


    eckit::Log::info() << "PointSearch using " << *tree_ << std::endl;

    if (!tree_->ready()) {
        build(r);
        tree_->commit();
    }

}

void PointSearch::build(const repres::Representation& r) {
    const size_t npts = r.numberOfPoints();

    eckit::Timer timer("Building KDTree");
    eckit::Log::info() << "Building " << *tree_ << " for " << r << " (" << eckit::Plural(npts, "point") << ")" << std::endl;

    static bool fastBuildKDTrees = eckit::Resource<bool>("$ATLAS_FAST_BUILD_KDTREES", true); // We use the same Resource as ATLAS for now

    if (fastBuildKDTrees) {
        std::vector<PointValueType> points;
        points.reserve(npts);

        const eckit::ScopedPtr<repres::Iterator> it(r.iterator());
        size_t i = 0;
        while (it->next()) {
            ASSERT(i < npts);
            points.push_back(PointValueType(it->point3D(), i));
            ++i;
        }

        tree_->build(points);
    }
    else {
        const eckit::ScopedPtr<repres::Iterator> it(r.iterator());
        size_t i = 0;
        while (it->next()) {
            ASSERT(i < npts);
            tree_->insert(PointValueType(it->point3D(), i));
            ++i;
        }
    }
}


void PointSearch::statsPrint(std::ostream& s, bool fancy) const {
    tree_->statsPrint(s, fancy);
}


void PointSearch::statsReset() const {
    tree_->statsReset();
}


PointSearch::PointValueType PointSearch::closestPoint(const PointSearch::PointType& pt) const {
    return tree_->nearestNeighbour(pt);
}


void PointSearch::closestNPoints(const PointType& pt, size_t n, std::vector<PointValueType>& closest) const {

    // Small optimisation
    if (n == 1) {
        closest.clear();
        closest.push_back(closestPoint(pt));
        return;
    }

    closest = tree_->kNearestNeighbours(pt, n);
}


void PointSearch::closestWithinRadius(const PointType& pt, double radius, std::vector<PointValueType>& closest) const {
    closest = tree_->findInSphere(pt, radius);
}


//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex *local_mutex = 0;
static std::map< std::string, PointSearchTreeFactory* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, PointSearchTreeFactory* >();
}
}  // (anonymous namespace)


PointSearchTreeFactory::PointSearchTreeFactory(const std::string& name):
    name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("PointSearchTreeFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


PointSearchTreeFactory::~PointSearchTreeFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


PointSearchTree *PointSearchTreeFactory::build(
    const repres::Representation& r,
    const param::MIRParametrisation& params,
    size_t itemCount) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    std::string name = "mapped-cache-file";
    params.get("point-search-trees", name);

    eckit::Log::info() << "PointSearchTreeFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "PointSearchTreeFactory: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("PointSearchTreeFactory: unknown '" + name + "'");
    }

    return (*j).second->make(r, params, itemCount);
}


void PointSearchTreeFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace util
}  // namespace mir
