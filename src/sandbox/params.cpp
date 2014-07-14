#include <vector>
#include <algorithm>
#include <iostream>

#include <Params.h>
#include <eckit/Log.h>
#include <eckit/Exception.h>


class GridSpecFactory {

static std::map<std::string,GridSpecFactory*> map_;


    virtual GridSpec* make(const Params&) const = 0;


public:
	GridSpecFactory(const std::string& name);
	static GridSpec* build(Param&);
};

static std::map<std::string,GridSpecFactory*> theMap;

GridSpecFactory::GridSpecFactory(const std::string& name)
{
	map_[name] = this;
}

PGridSpec* GridSpecFactory::build(const Params& params)
{

	std::map<std::string,GridSpecFactory*>::iterator j = map_.find(name);

	if(j == map_.end())
	{
		Log::error() << "GridSpec factories are:" << std::endl;
		for(j = theMap->begin() ; j != theMap->end() ; ++j)
			Log::error() << (*j).first << std::endl;
		throw eckit::SeriousBug(name + ": cannot find GridSpec factory.");
	}

	return (*j).second->make(params);
}

//==========================================================================

template<class T>
class GridSpecMaker : public GridSpecFactory {

    GridSpec* make(const Params& param)
            { return new T(param); }

public:
    GridSpecMaker() : GridSpecFactory(T::name())  {}
};

//==========================================================================

class GridSpec {
public:
    virtual ~GridSpec() { }
    virtual void print(std::ostream& s) const = 0;
    friend std::ostream& operator<<(std::ostream& s,const AccumulationStep& p)
        { p.print(s); return s; }
};


//==========================================================================

class RegularLL : public GridSpec {
private:
    double north_;
    double west_;
    double east_;
    double south_;
    double northSouthIncrement_;
    double westEastIncrement_;


    void print(std::ostream& s) const {
        s << "RegularLL[north="  << north_ <<
        s << ",west="  << west_ <<
        s << ",south="  << south_ <<
        s << ",east="  << east_ <<
        s << ",ew="  <<  westEastIncrement_<<
        s << ",ns="  <<  northSouthIncrement_<< "]";
}

public:
    static const char* name() { return "regular_ll"; }
}

static GridSpecMaker<RegularLL> makeRegularLL;


//==========================================================================

class ReducedGG : public GridSpec {
private:
    double north_;
    double west_;
    double east_;
    double south_;
    int n_;

    void print(std::ostream& s) const {
        s << "educedGGL[north="  << north_ <<
        s << ",west="  << west_ <<
        s << ",south="  << south_ <<
        s << ",east="  << east_ <<
        s << ",N="  <<  n_ << "]";
}

public:
    static const char* name() { return "reduced_gg"; }
}

static GridSpecMaker<ReducedGG> makeReducedGG;

//==========================================================================

int main()
{
    std::cout << "Hello" << std::endl;
}
