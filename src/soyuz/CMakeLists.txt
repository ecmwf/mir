list( APPEND mir_srcs
action/Action.cc
action/Action.h
action/AreaCropper.cc
action/AreaCropper.h
action/BitmapFilter.cc
action/BitmapFilter.h
action/FrameFilter.cc
action/FrameFilter.h
action/Gridded2GriddedInterpolation.cc
action/Gridded2GriddedInterpolation.h
action/NullAction.cc
action/NullAction.h
action/Sh2GriddedTransform.cc
action/Sh2GriddedTransform.h
action/Sh2ShTransform.cc
action/Sh2ShTransform.h
api/MIRJob.cc
api/MIRJob.h
api/emoslib.cc
inout/DummyInput.cc
inout/DummyInput.h
inout/DummyOutput.cc
inout/DummyOutput.h
inout/GribFileInput.cc
inout/GribFileInput.h
inout/GribFileOutput.cc
inout/GribFileOutput.h
inout/GribInput.cc
inout/GribInput.h
inout/GribMemoryInput.cc
inout/GribMemoryInput.h
inout/GribMemoryOutput.cc
inout/GribMemoryOutput.h
inout/GribOutput.cc
inout/GribOutput.h
inout/GribStreamInput.cc
inout/GribStreamInput.h
inout/GribStreamOutput.cc
inout/GribStreamOutput.h
inout/MIRInput.cc
inout/MIRInput.h
inout/MIROutput.cc
inout/MIROutput.h
inout/NetcdfFileInput.cc
inout/NetcdfFileInput.h
logic/ECMWFLogic.cc
logic/ECMWFLogic.h
logic/MARSLogic.cc
logic/MARSLogic.h
logic/MIRLogic.cc
logic/MIRLogic.h
method/Method.cc
method/Method.h
param/FieldParametrisation.cc
param/FieldParametrisation.h
param/MIRCombinedParametrisation.cc
param/MIRCombinedParametrisation.h
param/MIRConfiguration.cc
param/MIRConfiguration.h
param/MIRDefaults.cc
param/MIRDefaults.h
param/MIRParametrisation.cc
param/MIRParametrisation.h
repres/Albers.cc
repres/Albers.h
repres/AzimuthRange.cc
repres/AzimuthRange.h
repres/EquatorialAzimuthalEquidistant.cc
repres/EquatorialAzimuthalEquidistant.h
repres/Gridded.cc
repres/Gridded.h
repres/IrregularLatlon.cc
repres/IrregularLatlon.h
repres/Lambert.cc
repres/Lambert.h
repres/LambertAzimuthalEqualArea.cc
repres/LambertAzimuthalEqualArea.h
repres/Mercator.cc
repres/Mercator.h
repres/PolarStereographic.cc
repres/PolarStereographic.h
repres/ReducedGG.cc
repres/ReducedGG.h
repres/ReducedLL.cc
repres/ReducedLL.h
repres/RegularGG.cc
repres/RegularGG.h
repres/RegularLL.cc
repres/RegularLL.h
repres/Representation.cc
repres/Representation.h
repres/RotatedGG.cc
repres/RotatedGG.h
repres/RotatedLL.cc
repres/RotatedLL.h
repres/RotatedSH.cc
repres/RotatedSH.h
repres/SpaceView.cc
repres/SpaceView.h
repres/SphericalHarmonics.cc
repres/SphericalHarmonics.h
repres/StretchedGG.cc
repres/StretchedGG.h
repres/StretchedLL.cc
repres/StretchedLL.h
repres/StretchedRotatedGG.cc
repres/StretchedRotatedGG.h
repres/StretchedRotatedLL.cc
repres/StretchedRotatedLL.h
repres/StretchedRotatedSH.cc
repres/StretchedRotatedSH.h
repres/StretchedSH.cc
repres/StretchedSH.h
repres/TransverseMercator.cc
repres/TransverseMercator.h
repres/TriangularGrid.cc
repres/TriangularGrid.h
repres/UnstructuredGrid.cc
repres/UnstructuredGrid.h
util/Bitmap.cc
util/Bitmap.h
util/Grib.h
util/MIRField.cc
util/MIRField.h
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

