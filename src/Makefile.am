
AM_CFLAGS =  @WARN_PEDANTIC@ @WERROR@ @FORCE_32_CFLAGS@
AM_LDFLAGS = @FORCE_32_LDFLAGS@


lib_LIBRARIES = libecregrid.a

include_HEADERS = ecregrid_api.h api/DFO.h  api/ITransformer.h  api/ecField.h  api/ecTransformer.h

noinst_HEADERS = \
         machine.h  \
         Exception.h \
         Factory.h \
         GridContext.h \
         Input.h \
         GribApiInput.h \
         BinaryInput.h \
         AsciiInput.h \
         Output.h \
         GribApiOutput.h \
         GribApiOutputGrid.h \
         GribApiOutputSpectral.h \
         BinaryOutput.h \
         AsciiOutput.h \
         Tokenizer.h \
         Timer.h \
         Seconds.h \
         FieldIdentity.h \
         FieldFeatures.h \
         FieldDescription.h \
         FieldDescriptionGribApi.h \
         Constrains.h \
         Lsm.h \
         GTopo30.h \
         GTopo30Dem.h \
         LsmEmos10minute.h \
         LsmPreDefined.h \
         LsmFromGrid.h \
         Field.h \
         SpectralField.h \
         LegendrePolynomials.h \
         LegendrePolynomialsOnFly.h \
         LegendrePolynomialsRead.h \
         LegendrePolynomialsReadFromFile.h \
         LegendrePolynomialsMemoryMap.h \
         LegendreTransform.h \
         LegendreTransformSpectralToCoefficients.h \
         LegendreTransformCoefficientsToSpectral.h \
		 LegendrePolynomialsSharedMemory.h \
         FastFourierTransform.h \
         InverseFastFourierTransform.h \
         DirectFastFourierTransform.h \
         Area.h \
         Grid.h \
         Gaussian.h \
         LatLon.h \
         Extraction.h \
         Frame.h \
         Bitmap.h \
         SubGrid.h \
         SubArea.h \
         Interpolator.h \
         NearestNeigbour.h \
         NearestNeigbourAdjusted.h \
         DoubleLinearBitmapAdjusted.h \
         Linear.h \
         LinearBitmap.h \
         BiLinear.h \
		 BiLinearPrecipitation.h \
		 BiLinearLsmInteger.h \
         BiLinearBitmap.h \
         DoubleLinear.h \
         DoubleLinearBitmap.h \
         Cubic12pts.h \
         Cubic12ptsBitmap.h \
         LinearFit3Times.h \
         LinearFit3TimesBitmap.h \
         Average.h \
         AverageWeightedRegular.h \
         AverageWeightedRegularLsm.h \
         AverageWeightedReduced.h \
         AverageWeightedReducedLsm.h \
         FluxConservingRegular.h \
         FluxConservingReduced.h \
         Conserving.h \
         InterpolatorLsm.h \
         NearestNeigbourLsm.h \
         BiLinearLsm.h \
         BiLinearLsmBitmap.h \
         DoubleLinearLsm.h \
         DoubleLinearLsmBitmap.h \
         Cubic12ptsLsm.h \
         Cubic12ptsLsmBitmap.h \
         LinearFit3TimesLsm.h \
         LinearFit3TimesLsmBitmap.h \
         AverageLsm.h \
         DerivedSubgridParameters.h \
         Anisotropy.h \
         Orientation.h \
         Slope.h \
         PartialDerivatives.h \
         Transformer.h \
         GridToGridTransformer.h \
         GridToGridMatrixTransformer.h \
         SpectralToSpectralTransformer.h \
         SpectralToListOfPointsTransformer.h \
         SpectralToGridTransformer.h \
         SpectralToRotatedGridTransformer.h \
         GridToSpectralTransformer.h \
         FieldPoint.h \
         Point.h \
         Wind.h \
         Parameter.h \
         GridField.h \
         Rotation.h \
         GridType.h \
         Reduced.h \
         ReducedGaussian.h \
         ReducedLatLon.h \
         ReducedDataHelper.h \
         ReducedDataHelperBool.h \
         RegularDataIndexHelper.h \
         Regular.h \
         RegularDataHelper.h \
         RegularDataHelperBool.h \
         RegularLatLon.h \
         RegularLatLonCellCentered.h \
         RotatedRegularLatLon.h \
         RegularGaussian.h \
         PseudoRegularGaussian.h \
         Projection.h \
         PolarStereographic.h \
         ListOfPoints.h \
	 LegendrePolynomialsCollection.h \
	 ThreadSafeMap.h \
	 ref_counted_ptr.h \
	 DefinitionsCache.h \
	 api/ecImpl.h

#         VerticalTransformer.h VerticalTransformerSpectral.h VerticalTransformerGrid.h VerticalInterpolator.h VerticalLinear.h VerticalQuadratic.h 


libecregrid_a_SOURCES = \
         Exception.cpp \
         ecregrid_api.cpp  \
         Factory.cpp \
         Tokenizer.cpp \
         Input.cpp \
         BinaryInput.cpp \
         AsciiInput.cpp \
         Output.cpp \
         BinaryOutput.cpp \
         AsciiOutput.cpp \
         FieldIdentity.cpp \
         FieldDescription.cpp \
         FieldDescriptionGribApi.cpp \
         Constrains.cpp \
         Lsm.cpp \
         Timer.cpp \
         Seconds.cpp \
         GTopo30.cpp \
         GTopo30Dem.cpp \
         LsmEmos10minute.cpp \
         LsmPreDefined.cpp \
         LsmFromGrid.cpp \
         FieldFeatures.cpp \
         Grid.cpp \
         Gaussian.cpp \
         LatLon.cpp \
         Extraction.cpp \
         Frame.cpp \
         Bitmap.cpp \
         SubGrid.cpp \
         SubArea.cpp \
         Interpolator.cpp \
         Linear.cpp \
         LinearBitmap.cpp \
         BiLinear.cpp \
		 BiLinearPrecipitation.cpp \
		 BiLinearLsmInteger.cpp \
         BiLinearBitmap.cpp \
         DoubleLinear.cpp \
         DoubleLinearBitmap.cpp \
         DoubleLinearLsm.cpp \
         DoubleLinearLsmBitmap.cpp \
         NearestNeigbour.cpp \
         NearestNeigbourAdjusted.cpp \
         DoubleLinearBitmapAdjusted.cpp \
         Cubic12pts.cpp \
         Cubic12ptsBitmap.cpp \
         LinearFit3Times.cpp \
         LinearFit3TimesBitmap.cpp \
         Average.cpp \
         Conserving.cpp \
         AverageWeightedRegular.cpp \
         AverageWeightedRegularLsm.cpp \
         AverageWeightedReduced.cpp \
         AverageWeightedReducedLsm.cpp \
         FluxConservingRegular.cpp \
         FluxConservingReduced.cpp \
         InterpolatorLsm.cpp \
         NearestNeigbourLsm.cpp \
         BiLinearLsm.cpp \
         BiLinearLsmBitmap.cpp \
         Cubic12ptsLsm.cpp \
         Cubic12ptsLsmBitmap.cpp \
         LinearFit3TimesLsm.cpp \
         LinearFit3TimesLsmBitmap.cpp \
         AverageLsm.cpp \
         DerivedSubgridParameters.cpp \
         Anisotropy.cpp \
         Orientation.cpp \
         Slope.cpp \
         PartialDerivatives.cpp \
         Transformer.cpp \
         GridToGridTransformer.cpp \
         GridToGridMatrixTransformer.cpp \
         SpectralToSpectralTransformer.cpp \
         SpectralToGridTransformer.cpp \
         SpectralToListOfPointsTransformer.cpp \
         SpectralToRotatedGridTransformer.cpp \
         GridToSpectralTransformer.cpp \
         Point.cpp \
         Wind.cpp \
         Parameter.cpp \
         FieldPoint.cpp \
         Field.cpp       \
         SpectralField.cpp \
         LegendrePolynomials.cpp \
         LegendrePolynomialsOnFly.cpp \
         LegendrePolynomialsRead.cpp \
         LegendrePolynomialsReadFromFile.cpp \
         LegendrePolynomialsMemoryMap.cpp \
         LegendreTransform.cpp \
         LegendreTransformSpectralToCoefficients.cpp \
         LegendreTransformCoefficientsToSpectral.cpp \
         LegendrePolynomialsSharedMemory.cpp \
         FastFourierTransform.cpp \
         InverseFastFourierTransform.cpp \
         DirectFastFourierTransform.cpp \
         Area.cpp \
         GridField.cpp \
         GridType.cpp \
         Rotation.cpp \
         Reduced.cpp \
         ReducedGaussian.cpp \
         ReducedLatLon.cpp \
         Regular.cpp \
         RegularLatLon.cpp \
         RegularLatLonCellCentered.cpp \
         RotatedRegularLatLon.cpp \
         RegularGaussian.cpp \
         PseudoRegularGaussian.cpp \
         Projection.cpp \
         PolarStereographic.cpp \
         ListOfPoints.cpp \
         GribApiOutput.cpp \
         GribApiOutputGrid.cpp \
         GribApiOutputSpectral.cpp \
	 LegendrePolynomialsCollection.cpp \
	 ThreadSafeMap.cpp \
         GribApiInput.cpp \
	 DefinitionsCache.cpp \
	 api/DFO.cpp  api/ecImpl.cpp  api/ecTransformer.cpp

#         VerticalTransformer.cpp VerticalTransformerSpectral.cpp VerticalTransformerGrid.cpp VerticalInterpolator.cpp VerticalLinear.cpp VerticalQuadratic.cpp 



# One day to build optionaly
#         EXTRA_libecregrid_a_SOURCES = GribApiOutput.h GribApiOutput.cpp GribApiInput.h GribApiInput.cpp

LDADD = $(GRIB_API_LIB)
libecregrid_a_LIBADD = $(GRIB_API_DEP) @LIBOBJS@
libecregrid_a_DEPENDENCIES = $(libecregrid_a_LIBADD)

# set the include path
INCLUDES=  -I. -Iapi

## Make sure these will be cleaned even when they're not built by
## default.
CLEANFILES = libecregrid.a

