import os
import pathlib
p = pathlib.Path(__file__).parent.parent.parent.parent.parent
os.environ["ECCODES_DEFINITION_PATH"]=f"{p}/share/eccodes/definitions/"
os.environ["ECCODES_SAMPLES_PATH"]=f"{p}/share/eccodes/samples/"

from mir import *
