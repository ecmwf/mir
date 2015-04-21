#include <ITransformer.h>
#include <vector>
#include <string>

#include <FieldDescription.h>

class Transformer;
class Field;

namespace ecregrid {
class IFieldRepresentation;
class TransformerOptions;
class TransformerFactory;
class FieldRepresentationFactory;
class DFO;
};

//
// Implementation of the API Interfaces using ecRegrid's internal classes
//

class FieldRepresentationImpl : public ecregrid::IFieldRepresentation {
    friend class ecregrid::FieldRepresentationFactory;
    FieldRepresentationImpl(const ecregrid::DFO& opts);
    virtual ~FieldRepresentationImpl();

    // IFieldRepresentation implementation
    virtual size_t numberOfDataValues() const;

  public:
    const Field* field() const;
    const FieldDescription& description() const;

  private:
    void makeWrappedObjects(const ecregrid::DFO& opts);

    // the wrapped objects
    FieldDescription fd_;
    Field* field_;
};


class TransformerImpl : public ecregrid::ITransformer {
    friend class ecregrid::TransformerFactory;

    TransformerImpl(const ecregrid::IFieldRepresentation& in, ecregrid::IFieldRepresentation& out, const ecregrid::TransformerOptions& opts);
    virtual ~TransformerImpl();

    // ITransformer implementation
    virtual void transform(const std::vector<double>& in, std::vector<double>& out);
    virtual void transform(const std::vector<double>& input, std::vector<double>& output, unsigned long& output_length);
    virtual void transform(const std::pair<std::vector<double>, std::vector<double> >& in, std::pair<std::vector<double>, std::vector<double> >& out);

    // own methods
    //

    // wrapped ecregrid objects
    auto_ptr<Transformer> transformer_;
    const FieldRepresentationImpl& input_;
    FieldRepresentationImpl& output_;

};



