#!/bin/bash

#Tmp folder where all needed (definitive) flash image are put
tmp_dir=`mktemp -d -t chip-global-XXXXXX`
tmp_rootfs="${tmp_dir}/rootfs/"

verbose=false
input_rootfs=""
output_rootfs=""
release_folder_path=""

print_debug(){
	if [ $verbose = true ]; then
		printf "[debug] - $1\n"
	fi
	return 0
}

show_help(){
	echo ""
	echo "== Help =="
	echo "  -h  -- show help (this)"
	echo "  -i  -- Input rootfs.tar"
	echo "  -o  -- Output rootfs_full.tar"
	echo "  -f  -- release folder path"
	echo "  -v  -- Verbose mode"
	echo ""

	return 0
}

#
# START HERE
#
echo "Script start"

while getopts "i:o:f:hv" opt; do
	case $opt in
		i)
			input_rootfs="${OPTARG}"
			print_debug "Input rootfs: ${input_rootfs}"
		;;
		o)
			output_rootfs="${OPTARG}"
			print_debug "Output rootfs: ${output_rootfs}"
		;;
		f)
			release_folder_path="${OPTARG}"
			print_debug "Release folder path: ${output_rootfs}"
		;;
		h)
			show_help
			exit 0
		;;
		v)
			verbose=true
			printf "\n#########################################\n"
			printf "#########################################\n"
			printf "##         Verbose is activated        ##\n"
			printf "#########################################\n"
			printf "#########################################\n\n"
		;;
		\?)
			echo "== Invalid option: -${OPTARG} ==" >&2
			show_help
			exit 1
		;;
	esac
done

if [ "${output_rootfs}" = "" ]; then
	echo "output_rootfs is empty, abort..."
	exit -1
fi

if [ "${input_rootfs}" = "" ]; then
	echo "input_rootfs is empty, abort..."
	exit -1
fi

if [ "${release_folder_path}" = "" ]; then
	echo "release_folder_path is empty, abort..."
	exit -1
fi

print_debug "Tmp folder: ${tmp_dir}"

#Uncompress input rootfs tar into tmp rootfs
print_debug "Create ${tmp_rootfs}"
mkdir -p $tmp_rootfs

echo "Tar: extract ${input_rootfs} ==> ${tmp_rootfs}"
tar -xspf "${input_rootfs}" -C "${tmp_rootfs}"

#copy wat we need into the tmp rootfs
echo "Copy Data from release"
mkdir "${tmp_rootfs}/boot/"
rsync -a "${release_folder_path}/zImage" "${tmp_rootfs}/boot/"
rsync -a "${release_folder_path}/sun5i-r8-chip.dtb" "${tmp_rootfs}/boot/"
rsync -a "${release_folder_path}/sun5i-r8-pocketchip.dtb" "${tmp_rootfs}/boot/"
mkdir -p "${tmp_rootfs}/lib/modules/"
rsync -a "${release_folder_path}/linux_modules/lib/modules/" "${tmp_rootfs}/lib/modules/"

sync

echo "Tar: create ${output_rootfs} with ${tmp_rootfs}"
tar -C ${tmp_rootfs} -cpf ${output_rootfs} .
sync

print_debug "Clean up"
#rm -rf ${tmp_dir}

echo "Done"
exit 0
