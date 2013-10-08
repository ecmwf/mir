import os
main=os.path.abspath(os.path.join("%s" % os.getcwd(), "../.."))
share=os.path.join(main, "share/ecregrid")
os.environ["ECREGRID_SHARE_PATH"]=share
os.environ["ECREGRID_DEFINITIONS_PATH"]=os.path.join(share, "definitions")
#os.environ["ECREGRID_LSM_PATH"]=os.path.join(share, "lsm")
#os.environ["ECREGRID_LEGENDRE_COEFFICIENTS_PATH"]=os.path.join(main, "legendre_coefficients")
#os.environ["ECREGRID_DEBUG"]="1"


