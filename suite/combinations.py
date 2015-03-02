##################################################################################
# "platform_dict" is a list of platforms and machines on which tests should be
# run. It assumes that the output folder for the tests is in a shared location
# visible to all machines
#
#platform_dict={"linux": ["pepe"], "rs6000": ["ecgate"]}
platform_dict={"linux": ["localhost"]}


##################################################################################
# "comparisons" gives names of interpolators you want to recognise. 
# It is assumed here to be fixed
#
# NB It is important that these are DIFFERENT so use e.g. emos392 and emos394
# and not emos and emos for example
comparisons=["emos395", "emos401"]


# these options specficy exactly the mars client to use
#mars_folders=["/usr/local/lib/metaps", "/home/ma/maf/build/mars_client/debug"]
#mars_folders=["/usr/local/lib/metaps", "/tmp/maf/p4/mars/client/dev/grib_api"]
mars_folders=["", ""]
mars_commands=["mars", "mars -b ~/builds/mars-client/bin/mars.bin"]


# Whether to compare the files and produce difference plots if diffs are found
run_comparison=True


##################################################################################
# "dummy_run" allows the suite to be run without any output data files
# being produced - only the folders are created. Can be used for testing prior to a
# extensive test run.
# 
dummy_run=False


##################################################################################
# "memcheck_grids" allows you to state which source grids you would like to 
# include in memory checking.
# NB We cannot use red_ll as EMOS crashes valgrind so we added this
# fine-tuning
#
#memcheck_grids=["sh", "red_gg", "reg_gg", "reg_ll"] 
memcheck_grids=[]       


##################################################################################
# "grid_dict" defines the transformations that we wish to test
# in the suite. For each key (source grid), a list of target grids is defined.
#
grid_dict = {
    
    "sh":     [
                    "reg_ll", 
                    "reg_gg",
                    "red_gg", 
                    "sh",   
                    "rot_reg_ll", 
               ],

     "red_gg": [
                    "red_gg",
                    "reg_gg",
                    "reg_ll",
                    "rot_reg_ll"
               ],

     "reg_gg": [
                    "reg_gg",
                    "reg_ll",
                    #"rot_reg_ll"   # emos fails for sfc params
               ],

     "reg_ll": [
                     "reg_gg",
                     "reg_ll",
                     #"rot_reg_ll"    # emos fails for all parameters
                ],

     "red_ll":  [ 
                     "reg_ll", 
                     #"reg_gg" # removed as not required in verification yet acc to Sinisa 20130124
                ]
    
}


##################################################################################
# "src_requests" defines a set of stubs for various source MARS requests. 
# 
# Each stub, together with a line "target=filename", should be able to generate a
# source grib data file that is then interpolated into whatever target grid is
# required. NB the initial "RETRIEVE" is not required.
#
# NB We will always be using RETRIEVE requests to take data from source gribs.
#
# The form of the dict required is to use a key describing the test (e.g. t850) with its 
# value being a comma-separated MARS request. 
#
# NB 10u/10v requests broken in MARS as per JIRA MARS-382

src_requests = {
      "sh":
        {
            # name also for reference. Format is "name": "stub mars request"

            "t850": "stream=oper,step=0,levtype=pl,class=od,type=fc,param=130.128,levelist=850,",
            "z1000": "step=0,param=z,levelist=1000,",
            "uv_multilevel": "type=forecast,param=u/v,levtype=model level,levelist=1/to/45,step=48,",
            "vo_d": "class=od,type=an,stream=oper,repres=sh,levtype=pl,levelist=1000,param=vo/d,step=00,"
        },
       "red_gg":
        {
            "10uv_sfc": "stream=oper,step=0,levtype=sfc,class=od,type=fc,param=10u/10v,",
            "2t_sfc": "stream=oper,step=0,levtype=sfc,class=od,type=fc,param=2t,",
            "t850": "stream=oper,step=0,levelist=850,param=t,gaussian=reduced,grid=400,",
            "z_sfc": "stream=oper,step=0,levtype=sfc,class=od,type=fc,param=z,",
            "z1000": "stream=oper,step=0,levelist=1000,class=od,type=fc,param=z,gaussian=reduced,grid=400,",
            "z850_1280": "stream=oper,step=0,levelist=850,class=od,type=fc,param=z,gaussian=reduced,grid=1280,",
            "q": "class=od,type=an,stream=oper,levtype=ml,levelist=5,param=q,domain=g,",
        },
        "red_ll":
        { 
            "mwd": "class=od,param=mwd,levtype=sfc,stream=wave,type=fc,domain=g,",
            "mwp": "class=od,param=mwp,levtype=sfc,stream=wave,type=fc,domain=g,",

        },
        "reg_gg":
        {
            "10uv_sfc": "stream=oper,step=0,levtype=sfc,class=od,type=fc,param=10u/10v,gaussian=regular,grid=400,",
            "2t_sfc": "stream=oper,step=0,levtype=sfc,class=od,type=fc,param=2t,gaussian=regular,grid=400,",
            "t850": "stream=oper,step=0,levelist=850,param=t,gaussian=regular,grid=400,",
            "z_sfc": "stream=oper,step=0,levtype=sfc,class=od,type=fc,param=z,gaussian=regular,grid=400,",
            "z1000": "stream=oper,step=0,levelist=1000,class=od,type=fc,param=z,gaussian=regular,grid=400,",
        },
        "reg_ll":
        {
            "10uv_sfc": "stream=oper,step=0,levtype=sfc,class=od,type=fc,param=10u/10v,grid=0.25/0.25,",
            "2t_sfc": "stream=oper,step=0,levtype=sfc,param=2t,grid=0.25/0.25,",
            "t850": "stream=oper,step=0,levelist=850,param=t,grid=0.25/0.25,",
            "z_sfc": "stream=oper,step=0,levtype=sfc,param=z,grid=0.25/0.25,",
            "z1000": "stream=oper,step=0,levelist=1000,param=z,grid=0.25/0.25,",
        },
}


##################################################################################
# Below are a set of lists defining various parameters that are to be varied
# in the testing. 
#
# These correspond to MARS parameters that are to be varied in the target
# transformation
#
# For example, we might define a set of truncation values that
# are to be applied when transforming to spherical representations.
#
# NB use ANY for READ mars requests, or OFF for RETRIEVE
# (though we always use RETRIEVE)

predefined_ll_grids = [ "1.125/1.125", "1/1","0.5/0.5","0.4/0.4","0.25/0.25","0.2/0.2","0.125/0.125"]
# NB predefined 0.1/0.1 fails for emos - crashes. not sure why so removed it

ll_grids      = predefined_ll_grids   
# and some more...
ll_grids.extend(["5/5", "2/2", "1.5/1.5", "0.05/0.05" ])

# the spectral resolutions we will use
resol         = ["63", "95", "106", "160", "128", "200", "213", "511", "639", "799", "1279"]

rotation      = ["39/39", "1/1", "-40/22", "80/-10", "90/0"]
#rotation      = ["80/-10", "90/0"]

# sub areas to be defined
#area          = ("off", "90/10/-10/9","35.073/-50.975/-23.427/17.025")

ll_areas          = [ "off" , 
                     "75/60/10/90", 
                     "90/10/-10/200", 
                     "70/-20/10/60",  
                     "-40/0/-80/90",
                     "0/90/-30/10",
                     "75/10/30/40",  
                     "40/-20/-40/60",
                     "50/-135/-60/-35",
                     "40/-110/-10/-60",
                     "40/60/-60/180",
                     "80/115/-5/180",
                ]
gg_areas          = [ "off" , 
                      "75/62/10/90", 
                      "90/45/-10/180", 
                     "70/-90/10/67.5",  
                     "-40/0/-80/90",
                     "0/90/-30/11.25",
                     "75/22.5/30/45",  
                     "40/-22.5/-40/60",
                     "50/-135/-60/-22.5",
                     "40/-112.5/-10/-45",
                     "40/67.5/-60/180",
                     "80/90/-5/180",
                ]

#style         = ["off", "dissemination"]
frame         = ["off", "10", ]
packing       = ["off", "av", "si", "co"]
sh_packing    = ["off", "av", "co"]
accuracy      = ["off", "av", "12", "16", "24", "8"]
gauss_grids   = [48, 80, 100, 128, 160, 200, 256, 320, 400, 512, 640]
predefined_regular_gauss_grids = [160, 200,400,640]

reduced_gauss_grids   = [32, 48, 80, 128, 160, 200, 256, 320, 400, 512, 640, 1024, 1280]


##################################################################################
# "adjust" specifies which parameters are to be adjusted for which target grid
#
# For example, we don't want to have FRAME parameters for reduced target grids
#
# Also, transformations TO sh representation the parameters to be adjusted are
# listed in the line "sh" below, which would include a number of difference
# resolutions to test.
#

# format is { "TARGET_GRID" : {"MARS_KEYWORD": <container_of_values> } }


adjust = {
    
    "sh": { "resol": resol,
                "packing": sh_packing, 
          },

    "reg_ll": {   "area": ll_areas, 
               "frame": frame, 
               "grid": ll_grids,
               "packing": packing, 
                },

    "rot_reg_ll": { "rotation": rotation, 
                    "grid": predefined_ll_grids, 
                "packing": packing, 
                  },
    "reg_gg": { "area": gg_areas, 
                "frame": frame, 
                "grid": gauss_grids, 
                "packing": packing, 
              },
    "red_ll": { "area": ll_areas, 
                "grid": ll_grids, 
                "packing": packing, 
               },
    "red_gg": { "grid": reduced_gauss_grids, 
                "gaussian": ["reduced"], 
                "packing": packing, 
              },

    # the following to be applied to all target represenations

    "all": { 
        # "accuracy": accuracy, 
           },
}


