#ifndef ITRANSFORMER_H
#define ITRANSFORMER_H
//
// Pure interfaces to be implemented by ecRegrid
//
#include <cstddef>
#include <vector>


namespace ecregrid {

// Interface to transformation functionality between two fields or groups of
// fields
class ITransformer {
  public:


    // Transform between two single fields
    virtual void transform(const std::vector<double>& input, std::vector<double>& output) = 0;

    // Transform between two single fields with information on the size of the
    // data written to the output
    virtual void transform(const std::vector<double>& input, std::vector<double>& output, unsigned long& output_length) = 0;

    // We need transform method(s) for vectors. If we require that the input
    // and output sizes are the same perhaps we should type them e.g. "Pair"
    // so any mismatch is spotted at compile-time
    //
    // Will we ever need more than two to be passed at once (u,v conversion)?
    //
    // Alternatively using simple std::vector<FieldData> type would need to fail at
    // runtime if there is a size mismatch
    virtual void transform(const std::pair<std::vector<double>, std::vector<double> >& input, std::pair<std::vector<double>, std::vector<double> >& output) = 0;

    virtual ~ITransformer() {}
};

// Interface to class hierarchy of spectral or grid fields of
// various representations (gg, ll etc)
class IFieldRepresentation {
  public:
    // returns the size of a particular representation
    // for use e.g. in allocation
    virtual size_t numberOfDataValues() const = 0;
    // etc...
    virtual ~IFieldRepresentation() {}
};

}

#endif //ITRANSFORMER_H
