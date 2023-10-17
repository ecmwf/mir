#!/usr/bin/env python3

# script to create parameter-class.yaml for MIR from parameter database
# execute in separate folder as it creates temporary files
# robert.osinski@ecmwf.int 17/10/2023

import pandas as pd
import sys
import yaml
import os

## Connection with database ## 
################################################
## IMPORTANT ##
# define the database credentials as envvars !!!
# !!! use the read-only access !!!
####################
# PARAM_DB_HOST    #
# PARAM_DB_USER_RO #
# PARAM_DB_PASS_RO #
####################
################################################
OUTFILENAME="parameter-class.yaml"

import mysql.connector
## Create a connection object
## IP address of the MySQL database server
Host = os.environ['PARAM_DB_HOST']
## User name of the database server
User =os.environ['PARAM_DB_USER_RO']
## Password for the database user
Password = os.environ['PARAM_DB_PASS_RO']
## opens directly the database param
database = "param"

# connect
conn  = mysql.connector.connect(host=Host, user=User, password=Password, database=database)

# Create a cursor object
cur  = conn.cursor()

query=f"select mir_classification.param_id,mir_classifiers.classifier_name from mir_classification INNER JOIN mir_classifiers ON mir_classification.mir_classifier = mir_classifiers.classifier"

cur.execute(query)

mir_classification_dicts = cur.fetchall()

conn.close()

paramids2 = [i[0] for i in mir_classification_dicts]
classifiers = [i[1] for i in mir_classification_dicts]

classifiers_uniq = list(set(classifiers))
classifiers_uniq.sort()

output_dict = {}

for j in range(len(classifiers_uniq)):
    indextmp = [i for i, e in enumerate(classifiers) if e == classifiers_uniq[j]]
    pidtmp = [paramids2[index] for index in indextmp]
    print(classifiers_uniq[j],pidtmp)
    output_dict[classifiers_uniq[j]] = pidtmp

# now we write to the yaml

import ruamel.yaml
yaml = ruamel.yaml.YAML()
yaml.indent(sequence=4, offset=2)

file=open(OUTFILENAME,"w")
yaml.dump(output_dict,file)
file.close()

# Add --- in first line
os.system('sed -i "1i\---" ' + OUTFILENAME)

# add parameter names as comments after the paramId's
import subprocess	

shellScript = '''
echo "select id, name from param order by id ;" > request.sql
mysql -h $PARAM_DB_HOST -u $PARAM_DB_USER_RO  -p${PARAM_DB_USER_RO} param < request.sql  > id_name.dat
sed -i 's/\\t/,/' id_name.dat
rm -f request.sql

for i in $(grep -n '^  -' OUTFILENAME | awk -F: '{print $1}') ; do
 paramId=$(sed -n "${i}p" OUTFILENAME | awk '{print $2}')
 name=$(grep "^${paramId}," id_name.dat | awk -F',' '{print $2}')
 echo $i $paramId ${name}
# echo "sed -i "${i}s/$/\ \# ${name/\//\\\\/}/" OUTFILENAME"
 sed -i "${i}s/$/\ \ \# ${name/\//\\\\/}/" OUTFILENAME
done

rm -f id_name.dat'''

subprocess.run(shellScript.replace('OUTFILENAME',OUTFILENAME),
shell=True, check=True,
executable='/bin/bash')
