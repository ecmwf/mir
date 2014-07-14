#ifndef GridSpec_H
#define GridSpec_H
#include <iostream>

class GridSpec {
public:
    virtual ~GridSpec() { }

protected:
    virtual void print(std::ostream& s) const = 0;
    friend std::ostream& operator<<(std::ostream& s,const GridSpec& p)
        { p.print(s); return s; }
};


class GridSpecFactory {

    virtual GridSpec* make(const Params&) const = 0;


public:
    GridSpecFactory(const std::string& name);
    static GridSpec* build(Param&);
};

//==========================================================================

template<class T>
class GridSpecMaker : public GridSpecFactory {
    virtual GridSpec* make(const Params& param) { return new T(param); }
public:
    GridSpecMaker() : GridSpecFactory(T::name())  {}
};

//==========================================================================

#endif
