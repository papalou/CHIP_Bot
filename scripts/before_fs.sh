#!/bin/sh
#
# Script used by buildroot before building filesystem
#

# Get in the good folder wathever the call path is...
script_folder=$(dirname "$(readlink -f $0)")
project_root_folder="${script_folder}/../"

#Go to the project src folder
cd "$project_root_folder/src/"

#Build src
make clean && ./make_target.sh

#Copy src binary
cp chip_bot ${project_root_folder}/buildroot/output/target/chip_bot/chip_bot

exit 0
