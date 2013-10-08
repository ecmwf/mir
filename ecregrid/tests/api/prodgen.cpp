#include <vector>
#include <memory>
#include <string>
#include <sstream>
#include <stdio.h>
#include <iostream>

#include "ITransformer.h"
#include "ecTransformer.h"
#include "ecField.h"
#include "DFO.h"


ecregrid::DFO initialiseInput(std::vector<double>& data)
{
    // Load data values from our text file
    int truncation = 213;
    std::stringstream ss;
    ss << "sh" << truncation << ".txt";
    FILE* file=fopen(ss.str().c_str(), "r");
    char line[128];
    if (file)
    {
        while ( fgets ( line, sizeof(line), file ) != NULL ) /* read a line */
        {
            double value = 0.0;
            sscanf(line, "%lf", &value);
            data.push_back(value);
        }
    }

    ecregrid::DFO opts(ecregrid::DFO::SphericalHarmonic);
    opts.setTruncation(truncation);
    return opts;
}


int main(int argc, char** argv)
{
	// Load the input field here
    std::vector<double> idata;
    ecregrid::DFO i_opts = initialiseInput(idata);
    
	// Take the options we have and make a Field Description interface
    ecregrid::FieldRepresentationFactory fd_factory;
	std::auto_ptr<ecregrid::IFieldRepresentation> ifd( fd_factory.make(i_opts) );
    std::cout << "Input field has " << ifd->numberOfDataValues() << " values" << std::endl;
	
	// Assume there is a collection of output descriptions
	// to which we want to convert this input field
	std::vector<ecregrid::DFO> outputs;

    outputs.push_back(ecregrid::DFO(ecregrid::DFO::RegularGaussian));
    outputs.back().setGaussianNumber(100);
    outputs.back().setSubArea(eckit::Area(30.0, 0.0, 15.0, 25.0));
    
    outputs.push_back(ecregrid::DFO(ecregrid::DFO::RegularLatLon));
    outputs.back().setIncrements(0.25,0.25);

    outputs.push_back(ecregrid::DFO(ecregrid::DFO::RotatedRegularLatLon));
    outputs.back().setIncrements(0.5,0.5);
    outputs.back().setSouthPoleOfRotation(std::pair<double, double>(0.0, 1.0));

    outputs.push_back(ecregrid::DFO(ecregrid::DFO::SphericalHarmonic));
    outputs.back().setTruncation(213);
        
    outputs.push_back(ecregrid::DFO(ecregrid::DFO::ReducedGaussian));
    outputs.back().setGaussianNumber(200);
    
    outputs.push_back(ecregrid::DFO(ecregrid::DFO::RegularGaussian));
    outputs.back().setGaussianNumber(200);

    // Test the gaussian representation code
    std::vector<long> spec;
    ecregrid::TransformerUtils::getReducedGaussianRepresentation(80,  spec);
    for (unsigned int i = 0; i < spec.size(); i++)
        std::cout << "Reduced Latitude " << i << " has " << spec[i] << " points" << std::endl;

	// Initialise a factory to make interpolators
    ecregrid::TransformerFactory trans_factory;

	// iterate over output descriptions for this input field
	for (std::vector<ecregrid::DFO>::const_iterator it = outputs.begin(); it != outputs.end(); it++)
	{
		// Get a Field Description interface for current output specification		 	
		std::auto_ptr<ecregrid::IFieldRepresentation> ofd( fd_factory.make(*it) );
			
        // how big do we expect the output to be?
        const long number = ofd->numberOfDataValues();

        if (0 == number)
            continue;

		// Set container size for output values (or larger according to your plans)
        std::vector<double> odata(number);
        
        // Set transformation options here
        ecregrid::TransformerOptions opts(ecregrid::TransformerOptions::Bilinear);
        opts.setLegendrePolynomialsMethod(ecregrid::TransformerOptions::MemoryMapped);
        
        // Create a transformer interface from the input and output
        std::auto_ptr<ecregrid::ITransformer> transformer( trans_factory.make(*ifd, *ofd, opts) );

        
        // Transformer needs only know about data to process the data:
        size_t length_written;
        try {
            transformer->transform(idata, odata, length_written);
            std::cout << "Result contains " << length_written << " values. Expected " << number << std::endl;
        }
        catch (std::exception& e)
        {
            // ecRegrid exceptions now derive from std::exception
            std::cout << e.what() << std::endl;
        }
        catch (...)
        {
            std::cout << "Caught exception" << std::endl;
        }
        
            
        // TODO do something with the data in odata
	}	
	return 0;
}
