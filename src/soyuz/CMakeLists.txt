list( APPEND mir_srcs
Action.cc
Action.h
Albers.cc
Albers.h
AreaCropper.cc
AreaCropper.h
AzimuthRange.cc
AzimuthRange.h
Bitmap.cc
Bitmap.h
BitmapFilter.cc
BitmapFilter.h
DummyInput.cc
DummyInput.h
DummyOutput.cc
DummyOutput.h
ECMWFLogic.cc
ECMWFLogic.h
EquatorialAzimuthalEquidistant.cc
EquatorialAzimuthalEquidistant.h
FieldParametrisation.cc
FieldParametrisation.h
FrameFilter.cc
FrameFilter.h
Grib.h
GribFileInput.cc
GribFileInput.h
GribFileOutput.cc
GribFileOutput.h
GribInput.cc
GribInput.h
GribMemoryInput.cc
GribMemoryInput.h
GribMemoryOutput.cc
GribMemoryOutput.h
GribOutput.cc
GribOutput.h
GribStreamInput.cc
GribStreamInput.h
GribStreamOutput.cc
GribStreamOutput.h
Gridded.cc
Gridded.h
Gridded2GriddedInterpolation.cc
Gridded2GriddedInterpolation.h
IrregularLatlon.cc
IrregularLatlon.h
Lambert.cc
Lambert.h
LambertAzimuthalEqualArea.cc
LambertAzimuthalEqualArea.h
MARSLogic.cc
MARSLogic.h
MIRCombinedParametrisation.cc
MIRCombinedParametrisation.h
MIRConfiguration.cc
MIRConfiguration.h
MIRDefaults.cc
MIRDefaults.h
MIRField.cc
MIRField.h
MIRInput.cc
MIRInput.h
MIRJob.cc
MIRJob.h
MIRLogic.cc
MIRLogic.h
MIROutput.cc
MIROutput.h
MIRParametrisation.cc
MIRParametrisation.h
Mercator.cc
Mercator.h
Method.cc
Method.h
NetcdfFileInput.cc
NetcdfFileInput.h
NullAction.cc
NullAction.h
PolarStereographic.cc
PolarStereographic.h
ReducedGG.cc
ReducedGG.h
ReducedLL.cc
ReducedLL.h
RegularGG.cc
RegularGG.h
RegularLL.cc
RegularLL.h
Representation.cc
Representation.h
RotatedGG.cc
RotatedGG.h
RotatedLL.cc
RotatedLL.h
RotatedSH.cc
RotatedSH.h
Sh2GriddedTransform.cc
Sh2GriddedTransform.h
Sh2ShTransform.cc
Sh2ShTransform.h
SpaceView.cc
SpaceView.h
SphericalHarmonics.cc
SphericalHarmonics.h
StretchedGG.cc
StretchedGG.h
StretchedLL.cc
StretchedLL.h
StretchedRotatedGG.cc
StretchedRotatedGG.h
StretchedRotatedLL.cc
StretchedRotatedLL.h
StretchedRotatedSH.cc
StretchedRotatedSH.h
StretchedSH.cc
StretchedSH.h
TransverseMercator.cc
TransverseMercator.h
TriangularGrid.cc
TriangularGrid.h
UnstructuredGrid.cc
UnstructuredGrid.h
emoslib.cc
)


ecbuild_add_library(TARGET		mir
					SOURCES		${mir_srcs}
					INCLUDES  ${ECKIT_INCLUDE_DIRS} ${NETCDF_INCLUDE_DIRS}
					LIBS	  ${ATLAS_LIBRARIES} ${ECKIT_LIBRARIES} ${NETCDF_LIBRARIES})


ecbuild_add_test(
		TARGET    soyuz
		SOURCES   soyuz.cc
		RESOURCES test.bitmap test.grib 10.grib test.nc
		INCLUDES  ${ECKIT_INCLUDE_DIRS} ${NETCDF_INCLUDE_DIRS}
		LIBS      mir ${ECKIT_LIBRARIES}    ${NETCDF_LIBRARIES}
)

