import mir

# Retrieve some data as GRIB, e.g.
#
#   echo retrieve,param=z,target=z.grib | mars

# Create a MIR Job
job = mir.MIRJob().set('grid', '1.0/1.0')

# Run the job on the retrieved GRIB file
print('Running', job)
job.execute(mir.GribFileInput('z.grib'), mir.GribFileOutput('z_ll.grib'))
