#include "ecImpl.h"
#include <stdio.h>
#include <stdexcept>

// ecRegrid API includes
#include <DFO.h>
#include <Transformer.h>
#include <ecTransformer.h>

// ecRegrid Internal classes:
#include <Factory.h>
#include <SpectralField.h>
#include <GridField.h>
#include <BinaryOutput.h>

TransformerImpl::TransformerImpl(const ecregrid::IFieldRepresentation& in, ecregrid::IFieldRepresentation& out, const ecregrid::TransformerOptions& opts)
:   input_(dynamic_cast<const FieldRepresentationImpl&>(in)),
    output_(dynamic_cast<FieldRepresentationImpl&>(out))
{
    FieldDescription fd; 
    ecregrid::TransformerOptionsReader reader(opts);
    fd.ft_.auresol_ = reader.autoResolution();

    // The way the ecregrid code is structured is that there are two methods
    // for transformation: transform() and transformVector(). Setting the
    // vdConversion flag determines what the transformVector method does and
    // appears not to affect the other method. Hence we always set this to
    // true as we will always be calling transformVector when processing wind
    // fields.

    fd.ft_.vdConversion_ = true;

    // the other options need to be translated into strings...
    switch (reader.interpolationMethod())
    {
        case ecregrid::TransformerOptions::Bilinear:
            fd.ft_.interpolationMethod_ = "bilinear";
            break;
        case ecregrid::TransformerOptions::NearestNeighbour:
            fd.ft_.interpolationMethod_ = "nearestneighbour";
            break;
        case ecregrid::TransformerOptions::Cubic:
            fd.ft_.interpolationMethod_ = "cubic";
            break;
        case ecregrid::TransformerOptions::Linear:
            fd.ft_.interpolationMethod_ = "linear";
            break;
        case ecregrid::TransformerOptions::LinearFit:
            fd.ft_.interpolationMethod_ = "linearfit";
            break;
        case ecregrid::TransformerOptions::Average:
            fd.ft_.interpolationMethod_ = "average";
            break;
        case ecregrid::TransformerOptions::AverageWeighted:
            fd.ft_.interpolationMethod_ = "averageweighted";
            break;
        case ecregrid::TransformerOptions::NoInterpolation:
        default:
            break;
    }

    switch (reader.lsmMethod())
    {
        case ecregrid::TransformerOptions::Predefined:
            fd.ft_.lsmMethod_ = "predefined";
            break;
        case ecregrid::TransformerOptions::TenMin:
            fd.ft_.lsmMethod_ = "10min";
            break;
        case ecregrid::TransformerOptions::GTOPO:
            fd.ft_.lsmMethod_ = "gtopo";
            break;
        case ecregrid::TransformerOptions::NoLSM:
        default:
            break;
    }

    switch (reader.legendrePolynomialsMethod())
    {
        case ecregrid::TransformerOptions::OnFly:
            fd.ft_.legendrePolynomialsMethod_ = "on_fly";
            break;
        case ecregrid::TransformerOptions::FileIO:
            fd.ft_.legendrePolynomialsMethod_ = "fileio";
            break;
        case ecregrid::TransformerOptions::SharedMemory:
            fd.ft_.legendrePolynomialsMethod_ = "shared";
            break;
        case ecregrid::TransformerOptions::MemoryMapped:
            fd.ft_.legendrePolynomialsMethod_ = "mapped";
            break;
        case ecregrid::TransformerOptions::NoPolynomials:
        default:
            break;
    }

    switch (reader.poleExtrapolationMethod())
    {
        case ecregrid::TransformerOptions::AveragePole:
            fd.ft_.extrapolateOnPole_ = "average";
            break;
        case ecregrid::TransformerOptions::LinearPole:
            fd.ft_.extrapolateOnPole_ = "linear";
            break;
        case ecregrid::TransformerOptions::NoPole:
        default:
            break;
    }

    Factory factory;
    transformer_ = auto_ptr<Transformer>(factory.getTransformer(*input_.field(), *output_.field(), fd));
}

TransformerImpl::~TransformerImpl()
{
}

void TransformerImpl::transform(const std::vector<double>& input_data, std::vector<double>& output_data, unsigned long& output_data_length) 
{ 

    // jiggery pokery to get a field object we can transform with the correct
    // data
    auto_ptr<Field> input;
    
    const SpectralField* sf = dynamic_cast<const SpectralField*>(input_.field());
    const GridField* gf = dynamic_cast<const GridField*>(input_.field());

    if (sf)
    {
        input = auto_ptr<Field>(new SpectralField(sf->truncation(), sf->parameter(), sf->units(), sf->editionNumber(), sf->centre(), sf->levelType(), sf->level(), sf->date(), sf->time(), sf->stepUnits(), sf->startStep(), sf->endStep(), sf->bitsPerValue(), input_data));
    }
    else if (gf)
    {
        input = auto_ptr<Field>(new GridField( gf->grid().newGrid(), gf->parameter(), gf->units(), gf->editionNumber(), gf->centre(), gf->levelType(), gf->level(), gf->date(), gf->time(), gf->stepUnits(), gf->startStep(), gf->endStep(), gf->bitsPerValue(), gf->scanningMode(), gf->bitmap(), input_data, gf->missingValue()));
    }
    else
        throw std::runtime_error("Unknown field type.");

    auto_ptr<Field> end(transformer_->transform(*input, *output_.field()));

    // check for different scanning mode if grid
    const GridField* gridField = dynamic_cast<const GridField*>(output_.field());
    if (gridField)
        if (gridField->scanningMode() != 1)
        {
            // As yet unhandled scanning mode issue
            throw std::runtime_error("Unknown scanning mode");
        }


    BinaryOutput out;
    std::vector<double> result_data;

    // In certain circumstances, the output field has no data inside it
    // e.g. if the input field is the same as the output field requested
    // for spectral transformations. Handled here as output data length = 0
    if (end.get())
        out.deliverData(*end, result_data);
    output_data_length = result_data.size();

    // TODO do we throw here if output buffer supplied is not long enough, or reallocate?
    output_data.resize(std::max(output_data.size(), result_data.size()));
    std::copy(result_data.begin(), result_data.end(), output_data.begin());

} 

void TransformerImpl::transform(const std::vector<double>& input_data, std::vector<double>& output_data) 
{ 
    unsigned long dummylength;
    return transform(input_data, output_data, dummylength);
} 

void TransformerImpl::transform(const std::pair<std::vector<double>, std::vector<double> >& input, std::pair<std::vector<double>, std::vector<double> >& output) 
{ 
    printf("Not Yet Implemented: Transformer::transform(pair)\n"); 
    //auto_ptr<Field> end(transformer_->transformVector(*input, *output_.field()));
}

FieldRepresentationImpl::FieldRepresentationImpl(const ecregrid::DFO& opts)
    : field_(0)
{
    makeWrappedObjects(opts);
}

FieldRepresentationImpl::~FieldRepresentationImpl()
{
    if (field_)
    {
        delete field_;
        field_ = 0;
    }
}

void FieldRepresentationImpl::makeWrappedObjects(const ecregrid::DFO& opts)
{
    ecregrid::DFOReader reader(opts);
    std::pair<double, double> pole;

    switch (reader.representation())
    {
        case ecregrid::DFO::PolarStereographic:
            fd_ = FieldDescription("polar_stereographic");
        break;
        case ecregrid::DFO::RegularLatLon:
            fd_ = FieldDescription("regular_ll");
            fd_.increments(reader.westEastIncrement(), reader.northSouthIncrement());
        break;
        case ecregrid::DFO::RotatedRegularLatLon:
            fd_ = FieldDescription("rotated_ll");
            fd_.increments(reader.westEastIncrement(), reader.northSouthIncrement());
            reader.southPoleOfRotation(pole);
            fd_.southPole(pole.first, pole.second);
        break;
        case ecregrid::DFO::ReducedLatLon:
            fd_ = FieldDescription("reduced_ll");
            fd_.increments(reader.westEastIncrement(), reader.northSouthIncrement());            
        break;
        case ecregrid::DFO::RegularGaussian:
            fd_ = FieldDescription("regular_gg");
            fd_.gaussianNumber(reader.gaussianNumber());
        break;
        case ecregrid::DFO::ReducedGaussian:
            fd_ = FieldDescription("reduced_gg");
            fd_.gaussianNumber(reader.gaussianNumber());
        break;
        case ecregrid::DFO::SphericalHarmonic:
            fd_ = FieldDescription("sh");
            fd_.truncation(reader.truncation());
        break;
        case ecregrid::DFO::ListOfPoints :
            fd_ = FieldDescription("list");
        break;
        default:
            throw std::runtime_error("Unknown representation");
        break;

    }
    
    if (reader.representation() != ecregrid::DFO::SphericalHarmonic)
    {
        // grid-only settings common to all grid representations
        
        if (reader.representation() != ecregrid::DFO::ReducedGaussian &&
            reader.representation() != ecregrid::DFO::ReducedLatLon)
        {
            // Area / Frame 
            eckit::Area sub_area;
            reader.subArea(sub_area);
            fd_.area(sub_area.north(), sub_area.west(), sub_area.south(), sub_area.east());
            fd_.frameNumber(reader.frameNumber());
        }
        else
        {
            // is a reduced grid definitionreduced
            std::vector<long> grid;         
            reader.reducedGridDefinition(grid);
            if (grid.size() > 0)
            {
                fd_.reducedGridDefinition(&grid[0], grid.size());
            }
        }

        // Cell-centred
        fd_.cellCentered(reader.cellCentred(), reader.shifted());
        
        // Bitmap
        if (reader.usingBitmap())
        {
            std::string bitmapFile;
            double missingValue = 0.0;
            // get the bitmap info
            reader.bitmapInfo(bitmapFile, missingValue);
            // and pass it on
            fd_.bitmapFile(bitmapFile);
            fd_.missingValue(missingValue);
        } 
       
    }
                      
    // From this definition we make a field object
    field_ = fd_.defineField();
}

const Field* FieldRepresentationImpl::field() const
{
    return field_;
}

const FieldDescription& FieldRepresentationImpl::description() const
{
    return fd_;
}

size_t FieldRepresentationImpl::numberOfDataValues() const
{ 
    // different handling for grid fields
    GridField* gridField = dynamic_cast<GridField*>(field_);
    if (gridField)
    {
        return gridField->grid().calculatedNumberOfPoints();
    }

    // Other representations work fine...
    if (field())
        return field()->calculatedLength();

    return 0; 
}


