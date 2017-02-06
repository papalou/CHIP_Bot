#!/bin/bash

#Tmp folder where all needed (definitive) flash image are put
tmp_dir=`mktemp -d -t chip-global-XXXXXX`

verbose=false

#Buildroot folder given in parameter -i
buildroot_folder_path=""

#default device info
device="chip"

#Default nand info
nand_type="Toshiba_4G_MLC"

#Flash option and adress variable
timeout_secondes=120
fel_binary="sunxi-fel"

spl_memory_addr="0x43000000"
uboot_memory_addr="0x4a000000"
uboot_script_addr="0x43100000"

# All function

print_big_banner(){
	printf "\n"
	printf "################################################################################################\n"
	printf "##                                                                                            ##\n"
	printf "## %-90.90s ##\n" "$1"
	printf "##                                                                                            ##\n"
	printf "################################################################################################\n"
	printf "\n"
}
print_banner(){
	printf "\n"
	printf "+------------------------------------------+\n"
	printf "| %-40.40s |\n" "$1"
	printf "+------------------------------------------+\n"
	printf "\n"
}

check_return(){
	if [ "$1" = "" ]; then
		echo "Error, function check return must have parameter in first parameter"
	fi

	if [ "$2" = "" ]; then
		echo "Error, function check return must have parameter in seconde parameter"
	fi

	if [ $? != 0 ];then
		echo "[ERROR] $1, return: $? --> exit: $2"
		exit $2
	fi
}

print_debug(){
	if [ $verbose = true ]; then
		printf "[debug] - $1\n"
	fi
	return 0
}

onMac() {
  if [ "$(uname)" == "Darwin" ]; then
    return 0;
  else
    return 1;
  fi
}

filesize() {
  if onMac; then
    stat -f "%z" $1
  else
    stat --printf="%s" $1
  fi
}

prepare_ubi() {
	#Create temp folder for all image build
  	local prepare_ubi_tmp_dir=`mktemp -d -t chip-ubi-XXXXXX`

	#Get all function argument
	local input_tar="$1"
	local output_ubifs_sparse="$2"
	local nand_type="$3"
	local max_logical_erase_block_count="$4"
	local erase_block_size="$5"
	local page_size="$6"
	local sub_page_size="$7"

	print_debug "prepare_ubi -> input_tar        : $input_tar"
	print_debug "prepare_ubi -> output_ubifs     : $output_ubifs_sparse"
	print_debug "prepare_ubi -> nand_type        : $nand_type"
	print_debug "prepare_ubi -> max_leb_count    : $max_logical_erase_block_count"
	print_debug "prepare_ubi -> erase_block_size : $erase_block_size"
	print_debug "prepare_ubi -> page_size        : $page_size"
	print_debug "prepare_ubi -> sub_page_size    : $sub_page_size"

	#Configure needed var for the image build
	local tmp_rootfs=$prepare_ubi_tmp_dir/rootfs
	local tmp_ubifs=$prepare_ubi_tmp_dir/rootfs.ubifs
	local tmp_ubicfg=$prepare_ubi_tmp_dir/ubi.cfg
	local tmp_ubi=$prepare_ubi_tmp_dir/chip-rootfs.ubi
	local tmp_sparse_ubi=$prepare_ubi_tmp_dir/chip-rootfs.ubi.sparse
	local mlcopts=""

	if [ -z $sub_page_size ]; then
		sub_page_size=$page_size
	fi
	
	if [ "$nand_type" = "mlc" ]; then
		logical_erase_block_size=$((erase_block_size/2-$page_size*2))
		echo "WARNING FIX OPTION"
		#mlcopts="-M dist3"
	elif [ $sub_page_size -lt $page_size ]; then
		logical_erase_block_size=$((erase_block_size-page_size))
	else
		logical_erase_block_size=$((erase_block_size-page_size*2))
	fi

	mkdir -p $tmp_rootfs
	tar -xf $input_tar -C $tmp_rootfs


	print_banner "Create mkfs UBIFS"
	print_debug "--> mkfs.ubifs -d $tmp_rootfs -m $page_size -e $logical_erase_block_size -c $max_logical_erase_block_count -o $tmp_ubifs"
	$mkfs_ubifs -d $tmp_rootfs -m $page_size -e $logical_erase_block_size -c $max_logical_erase_block_count -o $tmp_ubifs -v
	print_banner "Create mkfs UBIFS --> DONE"

	echo "[rootfs]"           	> $tmp_ubicfg
	echo "mode=ubi"           	>> $tmp_ubicfg
	echo "vol_id=0"           	>> $tmp_ubicfg
	echo "vol_type=dynamic"   	>> $tmp_ubicfg
	echo "vol_name=rootfs"    	>> $tmp_ubicfg
	echo "vol_alignment=1"      >> $tmp_ubicfg
	echo "vol_flags=autoresize" >> $tmp_ubicfg
	echo "image=$tmp_ubifs"     >> $tmp_ubicfg

	print_debug "--> ubinize -o $tmp_ubi -p $erase_block_size -m $page_size -s $sub_page_size $mlcopts $tmp_ubicfg"
	$ubinize -o $tmp_ubi -p $erase_block_size -m $page_size -s $sub_page_size $mlcopts $tmp_ubicfg

	print_debug "--> img2simg $tmp_ubi $tmp_sparse_ubi $erase_block_size"
	$img2simg $tmp_ubi $tmp_sparse_ubi $erase_block_size

	print_debug "Create file done -> copy to: ${output_ubifs_sparse}"
	print_debug "Ubifs sparse size: $(filesize $tmp_sparse_ubi)"
	cp $tmp_sparse_ubi $output_ubifs_sparse

	echo "Rm tmp dir: $prepare_ubi_tmp_dir"
	rm -rf $prepare_ubi_tmp_dir

	return 0
}

# build the SPL image
prepare_spl() {
	local tmp_dir_spl=`mktemp -d -t chip-spl-XXXXXX`
	local output_spl_file=$1
	local input_spl_file=$2
	local erase_block_size=$3
	local page_size=$4
	local oob_size=$5
	local repeat=$((erase_block_size/page_size/64))
	local nand_spl=$tmp_dir_spl/nand-spl.bin
	local nand_padded_spl=$tmp_dir_spl/nand-padded-spl.bin
	local ebsize=`printf %x $erase_block_size`
	local psize=`printf %x $page_size`
	local osize=`printf %x $oob_size`
	local padding=$tmp_dir_spl/padding
	local spl_padding=$tmp_dir_spl/nand-spl-padding

	print_debug "output_spl_file   --> $output_spl_file"
	print_debug "input_spl_file    --> $input_spl_file"
	print_debug "erase_block_size  --> $erase_block_size"
	print_debug "page_size         --> $page_size"
	print_debug "oob_size          --> $oob_size"
	print_debug "repeat            --> $repeat"
	print_debug "nand_spl          --> $nand_spl"
	print_debug "nand_padded_spl   --> $nand_padded_spl"
	print_debug "ebsize            --> $ebsize"
	print_debug "psize             --> $psize"
	print_debug "osize             --> $osize"
	print_debug "padding           --> $padding"
	print_debug "spl_padding       --> $spl_padding"
	
	print_debug "CMD --> $sunxi_nand_image_builder -c 64/1024 -p $page_size -o $oob_size -u 1024 -e $erase_block_size -b -s $input_spl_file $nand_spl"
	$sunxi_nand_image_builder -c 64/1024 -p $page_size -o $oob_size -u 1024 -e $erase_block_size -b -s $input_spl_file $nand_spl
	
	local nand_spl_size=`filesize $nand_spl`
	local padding_size=$((64-(nand_spl_size/(page_size+oob_size))))

	local i=0

	print_debug "nand spl size : $nand_spl_size"
	print_debug "page size     : $page_size"
	print_debug "oob size      : $oob_size" #wtf is OOB 
	print_debug "padding size  : $padding_size"
	
	while [ $i -lt $repeat ]; do
		dd if=/dev/urandom of=$padding bs=1024 count=$padding_size
		$sunxi_nand_image_builder -c 64/1024 -p $page_size -o $oob_size -u 1024 -e $erase_block_size -b -s $padding $spl_padding
		cat $nand_spl $spl_padding > $nand_padded_spl
	
		if [ "$i" -eq "0" ]; then
			print_debug "spl i equal 0"
			cat $nand_padded_spl > $output_spl_file
		else
			print_debug "Spl in Else"
			cat $nand_padded_spl >> $output_spl_file
		fi
	
		i=$((i+1))
	done
	
	print_debug "Write spl file into: $output_spl_file size: $(filesize $output_spl_file)"
	rm -rf $tmp_dir_spl
	return 0
}

# build the bootloader image
prepare_uboot() {
	local output_uboot_file=$1
	local input_uboot_file=$2
	local eraseblocksize=$3
	local ebsize=`printf %x $eraseblocksize`
	local paddeduboot=$output_uboot_file
	
	dd if=$input_uboot_file of=$paddeduboot bs=$eraseblocksize conv=sync
	echo "Write uboot file into: $paddeduboot"
	return 0
}

wait_for_fastboot() {
  echo -n "waiting for fastboot...";
  for ((i=$timeout_secondes; i>0; i--)) {
    if [[ ! -z "$(fastboot -i 0x1f3a $@ devices)" ]]; then
      echo "OK";
      return 0;
    fi
    echo -n ".";
    sleep 1
  }

  echo "TIMEOUT";
  return 1
}

wait_for_fel() {
  echo -n "waiting for fel...";
  for ((i=$timeout_secondes; i>0; i--)) {
    if ${fel_binary} $@ ver 2>/dev/null >/dev/null; then
      echo "OK"
      return 0;
    fi
    echo -n ".";
    sleep 1
  }

  echo "TIMEOUT";
  return 1
}

create_uboot_start_cmd(){
	local output_uboot_cmds=$1
  	local uboot_size=$2
  	local padded_spl_size_to_write=$3

	print_debug  "Output uboot cmd file --> $output_uboot_cmds"
	print_debug  "uboot_size            --> $uboot_size"
	print_debug  "pages_per_erase_block --> $pages_per_erase_block"

	#if [ "x$ERASEMODE" = "xscrub" ]; then
	#	echo "nand scrub.chip -y" > $output_uboot_cmds
	#else
	#	echo "nand erase.chip" > $output_uboot_cmds
	#fi

	#Erase nand scrub method
	echo "nand erase.chip" > $output_uboot_cmds
	
	#Print spl memory addr
	echo "echo Write SPL --> addr: $spl_memory_addr 0x0 peb: $padded_spl_size_to_write" >> $output_uboot_cmds
	echo "nand write.raw.noverify $spl_memory_addr 0x0 $padded_spl_size_to_write" >> $output_uboot_cmds

	#Print spl backup memory write
	echo "echo Write SPL back --> addr: $spl_memory_addr 0x400000 peb: $padded_spl_size_to_write" >> $output_uboot_cmds
	echo "nand write.raw.noverify $spl_memory_addr 0x400000 $padded_spl_size_to_write" >> $output_uboot_cmds

	#Print uboot memory write info
	echo "echo Write UBOOT --> addr: $uboot_memory_addr 0x800000 size: $uboot_size" >> $output_uboot_cmds
	echo "nand write $uboot_memory_addr 0x800000 $uboot_size" >> $output_uboot_cmds

	#Set env
	echo "setenv mtdparts mtdparts=sunxi-nand.0:4m(spl),4m(spl-backup),4m(uboot),4m(env),-(UBI)" >> $output_uboot_cmds
	echo "setenv bootargs root=ubi0:rootfs rootfstype=ubifs rw earlyprintk ubi.mtd=4" >> $output_uboot_cmds
	echo "setenv bootcmd 'gpio set PB2; if test -n \${fel_booted} && test -n \${scriptaddr}; then echo '(FEL boot)'; source \${scriptaddr}; fi; mtdparts; ubi part UBI; ubifsmount ubi0:rootfs; ubifsload \$fdt_addr_r /boot/sun5i-r8-chip.dtb; ubifsload \$kernel_addr_r /boot/zImage; bootz \$kernel_addr_r - \$fdt_addr_r'" >> $output_uboot_cmds
	echo "setenv fel_booted 0" >> $output_uboot_cmds
	
	echo "echo Enabling Splash" >> $output_uboot_cmds
	echo "setenv stdout serial" >> $output_uboot_cmds
	echo "setenv stderr serial" >> $output_uboot_cmds
	echo "setenv splashpos m,m" >> $output_uboot_cmds
	
	echo "echo Configuring Video Mode" >> $output_uboot_cmds

	if [ "$device" = "pocketchip" ]; then
		echo "setenv clear_fastboot 'i2c mw 0x34 0x4 0x00 4;'" >> $output_uboot_cmds
		echo "setenv write_fastboot 'i2c mw 0x34 0x4 66 1; i2c mw 0x34 0x5 62 1; i2c mw 0x34 0x6 30 1; i2c mw 0x34 0x7 00 1'" >> $output_uboot_cmds
		echo "setenv test_fastboot 'i2c read 0x34 0x4 4 0x80200000; if itest.s *0x80200000 -eq fb0; then echo (Fastboot); i2c mw 0x34 0x4 0x00 4; fastboot 0; fi'" >> $output_uboot_cmds

		echo "setenv bootargs root=ubi0:rootfs rootfstype=ubifs rw ubi.mtd=4 quiet lpj=501248 loglevel=3 splash plymouth.ignore-serial-consoles" >> $output_uboot_cmds
		echo "setenv bootpaths 'initrd noinitrd'" >> $output_uboot_cmds
		echo "setenv bootcmd '${NO_LIMIT}run test_fastboot; if test -n \${fel_booted} && test -n \${scriptaddr}; then echo (FEL boot); source \${scriptaddr}; fi; for path in \${bootpaths}; do run boot_\$path; done'" >> $output_uboot_cmds
		echo "setenv boot_initrd 'mtdparts; ubi part UBI; ubifsmount ubi0:rootfs; ubifsload \$fdt_addr_r /boot/sun5i-r8-chip.dtb; ubifsload 0x44000000 /boot/initrd.uimage; ubifsload \$kernel_addr_r /boot/zImage; bootz \$kernel_addr_r 0x44000000 \$fdt_addr_r'" >> $output_uboot_cmds
		echo "setenv boot_noinitrd 'mtdparts; ubi part UBI; ubifsmount ubi0:rootfs; ubifsload \$fdt_addr_r /boot/sun5i-r8-chip.dtb; ubifsload \$kernel_addr_r /boot/zImage; bootz \$kernel_addr_r - \$fdt_addr_r'" >> $output_uboot_cmds
		echo "setenv video-mode" >> $output_uboot_cmds
		echo "setenv dip_addr_r 0x43400000" >> $output_uboot_cmds
		echo "setenv dip_overlay_dir /lib/firmware/nextthingco/chip/early" >> $output_uboot_cmds
		echo "setenv dip_overlay_cmd 'if test -n \"\${dip_overlay_name}\"; then ubifsload \$dip_addr_r \$dip_overlay_dir/\$dip_overlay_name; fi'" >> $output_uboot_cmds
		echo "setenv fel_booted 0" >> $output_uboot_cmds
		echo "setenv bootdelay 1" >> "$output_uboot_cmds"
	else
		echo "setenv bootpaths 'initrd noinitrd'" >> $output_uboot_cmds
		echo "setenv bootcmd '${NO_LIMIT}run test_fastboot; if test -n \${fel_booted} && test -n \${scriptaddr}; then echo (FEL boot); source \${scriptaddr}; fi; for path in \${bootpaths}; do run boot_\$path; done'" >> $output_uboot_cmds
		echo "setenv boot_initrd 'mtdparts; ubi part UBI; ubifsmount ubi0:rootfs; ubifsload \$fdt_addr_r /boot/sun5i-r8-chip.dtb; ubifsload 0x44000000 /boot/initrd.uimage; ubifsload \$kernel_addr_r /boot/zImage; bootz \$kernel_addr_r 0x44000000 \$fdt_addr_r'" >> $output_uboot_cmds
		echo "setenv boot_noinitrd 'mtdparts; ubi part UBI; ubifsmount ubi0:rootfs; ubifsload \$fdt_addr_r /boot/sun5i-r8-chip.dtb; ubifsload \$kernel_addr_r /boot/zImage; bootz \$kernel_addr_r - \$fdt_addr_r'" >> $output_uboot_cmds
		echo "setenv dip_addr_r 0x43400000" >> $output_uboot_cmds
		echo "setenv dip_overlay_dir /lib/firmware/nextthingco/chip/early" >> $output_uboot_cmds
		echo "setenv dip_overlay_cmd 'if test -n \"\${dip_overlay_name}\"; then ubifsload \$dip_addr_r \$dip_overlay_dir/\$dip_overlay_name; fi'" >> $output_uboot_cmds
		echo "setenv video-mode sunxi:640x480-24@60,monitor=composite-ntsc,overscan_x=40,overscan_y=20" >> $output_uboot_cmds
	fi
	
	echo "saveenv" >> $output_uboot_cmds
	
#	echo "echo going to fastboot mode" >> $output_uboot_cmds
#	echo "fastboot 0" >> $output_uboot_cmds
	echo "reset" >> $output_uboot_cmds

	return 0
}

show_help(){
	echo ""
	echo "== Help =="
	echo "  -h  -- show help (this)"
	echo "  -F  --  Format(optional)   ['Toshiba_4G_MLC' or 'Hynix_8G_MLC' or 'Toshiba_512M_MLC']"
	echo "            -> Default: 'Toshiba_4G_MLC'"
	echo "  -b  -- Buildroot folder path [ex: /home/foo/CHIP-buildroot/]"
	echo "  -d  -- Device ['chip' or 'pocketchip']"
	echo "            -> Default: 'chip'"
	echo "  -v  -- Verbose mode"
	echo ""

	return 0
}

echo "Script start"

while getopts "hF:b:d:v" opt; do
	case $opt in
		F)
			nand_type="${OPTARG}"
			echo "== Format ${nand_type} selected =="
		;;
		h)
			show_help
			exit 0
		;;
		b)
			buildroot_folder_path="${OPTARG}"
		;;
		d)
			device="${OPTARG}"
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
			exit 1
		;;
	esac
done

if [ "${buildroot_folder_path}" = "" ]; then
	echo "Buildroot image folder is empty, abort..."
	exit -1
fi

#mtd4
#Name:                           rootfs
#Type:                           mlc-nand
#Eraseblock size:                4194304 bytes, 4.0 MiB
#Amount of eraseblocks:          1020 (4278190080 bytes, 4.0 GiB)
#Minimum input/output unit size: 16384 bytes
#Sub-page size:                  16384 bytes
#OOB size:                       1280 bytes
#Character device major/minor:   90:8
#Bad blocks are allowed:         true
#Device is writable:             true
#data offset = 00008000 vid_hdr_offs = 00004000
#Default UBI VID header offset:  16384
#Default UBI data offset:        32768
#Default UBI LEB size:           4161536 bytes, 4.0 MiB
#Maximum UBI volumes count:      128

case $nand_type in
	"Hynix_8G_MLC")
		nand_level_cell="mlc"
		max_leb_count=4096
		nand_erase_block_size=4194304
		nand_page_size=16384
		nand_subpage_page_size=16384
		nand_oob_size=680
		nand_oob_size=1664
		nand_erase_size=400000
		nand_write_size=4000
	;;
	"Toshiba_4G_MLC")
		echo "Nand type: 'Toshiba_4G_MLC'"
		nand_level_cell="mlc"
		nand_erase_block_size=4194304
		nand_page_size=16384
		nand_subpage_page_size=16384
		#Max leb count = 1020 but i put 1000
		max_leb_count=1000
		nand_oob_size=1280
		#nand_oob_size=500

		#wtf
		nand_erase_size=400000
		nand_write_size=4000
	;;
	"Toshiba_512M_MLC")
		nand_level_cell="mlc"
		max_leb_count=4096
		nand_erase_block_size=4194304
		nand_page_size=16384
		nand_subpage_page_size=16384
		nand_oob_size=100
		nand_oob_size=1280
		nand_erase_size=40000
		nand_write_size=1000
	;;
	\?)
		echo "== Invalid nand type: $nand_type ==" >&2
		exit 1
	;;
esac

case $device in
	"chip")
		echo "Device selected CHIP"
	;;
	"pocketchip")
		echo "Device selected PocketCHIP"
	;;
	\?)
		echo "== Invalid device type: $device ==" >&2
		exit 1
	;;
esac

#
# Needed binary to build
#
sunxi_nand_image_builder="${buildroot_folder_path}/output/host/usr/bin/sunxi-nand-image-builder"
mkfs_ubifs="${buildroot_folder_path}/output/host/usr/sbin/mkfs.ubifs"
ubinize="${buildroot_folder_path}/output/host/usr/sbin/ubinize"
img2simg="img2simg"

#Input binary
buildroot_sunxi_spl_bin_path="$buildroot_folder_path/output/images/sunxi-spl.bin"
buildroot_sunxi_spl_ecc_bin_path="$buildroot_folder_path/output/images/sunxi-spl-with-ecc.bin"
buildroot_rootfs_tar_path="$buildroot_folder_path/output/images/rootfs.tar"
buildroot_uboot_bin_path="$buildroot_folder_path/output/images/u-boot-dtb.bin"

#Output binary
#output_spl="${tmp_dir}/sunxi_spl_padded.bin"
output_spl="${buildroot_sunxi_spl_ecc_bin_path}" ##Padded SPL build by buildroot
output_ubi_rootfs="${tmp_dir}/rootfs.ubifs"
output_ubi_rootfs_sparse="${tmp_dir}/rootfs.ubi.sparse"
output_uboot_bin="${tmp_dir}/uboot.bin"
output_uboot_cmds="${tmp_dir}/uboot.cmds"
output_uboot_script="${tmp_dir}/uboot.scr"

###################################################################
##                                                               ##
##                      Script start here                        ##
##                                                               ##
###################################################################
print_big_banner "Start Scripts to flash buildroot images"

echo "Clean tmp dir: ${tmp_dir}"
if [ -e ${tmp_dir} ]; then
	rm -rf ${tmp_dir}
fi
mkdir ${tmp_dir}

###################################################################
##                                                               ##
##                Create what is needed in tmp_dir               ##
##                                                               ##
###################################################################
print_big_banner "Create what is needed in tmp_dir"

## Prepare ubifs (ROOTFS)
print_banner "Prepare UBI"
prepare_ubi $buildroot_rootfs_tar_path $output_ubi_rootfs_sparse  $nand_level_cell $max_leb_count $nand_erase_block_size $nand_page_size $nand_subpage_page_size
check_return "Prepare UBIFS fail" -1

print_banner "Prepare UBOOT"
prepare_uboot $output_uboot_bin $buildroot_uboot_bin_path $nand_erase_block_size
check_return "Prepare UBOOT fail" -2

uboot_size=$(filesize $output_uboot_bin | xargs printf "0x%08x")
PADDED_SPL_SIZE=$(filesize "${output_spl}")
size_to_write_padded_spl=$(($PADDED_SPL_SIZE / ($nand_page_size + $nand_oob_size)))
pages_per_erase_block=$(echo $((nand_erase_size/nand_write_size)) | xargs printf "%x")

print_debug "uboot size -> $uboot_size"
print_debug "page_per_eb -> $pages_per_erase_block"

print_banner "Create uboot cmd"
create_uboot_start_cmd $output_uboot_cmds $uboot_size $size_to_write_padded_spl
check_return "Prepare UBOOT command fail" -3

print_banner "Prepare SPL"
#prepare_spl $output_spl $buildroot_sunxi_spl_ecc_bin_path $nand_erase_block_size $nand_page_size $nand_oob_size
#prepare_spl $output_spl $buildroot_sunxi_spl_bin_path $nand_erase_block_size $nand_page_size 1280
#check_return "Prepare SPL fail" -4

print_banner "make image uboot"
mkimage -A arm -T script -C none -n "flash $device" -d $output_uboot_cmds $output_uboot_script
check_return "mkimage fail" -5

###################################################################
##                                                               ##
##                         Wait for FEL mode                     ##
##                                                               ##
###################################################################
print_big_banner "Wait for FEL mode"

if ! wait_for_fel; then
	echo "ERROR: please make sure CHIP is connected and jumpered in FEL mode"
	exit 1
fi

###################################################################
##                                                               ##
##           Load All Binary into memory for Fastboot            ##
##                                                               ##
###################################################################
print_big_banner "Load All Binary into memory for fastboot"

echo "Load boot spl bin for flash: $output_spl"
$fel_binary spl $buildroot_sunxi_spl_bin_path
check_return "Load spl bin fail" -10
# wait for DRAM initialization to complete
sleep 1

echo "Write uboot: addr: $uboot_memory_addr, bin: $output_uboot_bin"
$fel_binary write $uboot_memory_addr $output_uboot_bin
check_return "Write uboot fail" -11

echo "Write SPL: addr: $spl_memory_addr, bin: $output_spl"
$fel_binary write $spl_memory_addr $output_spl
check_return "Write SPL fail" -12

echo "Write Uboot script: addr: $uboot_script_addr, path: $output_uboot_script"
$fel_binary write $uboot_script_addr $output_uboot_script
check_return "Write uboot script fail" -13

echo "Execute UBoot: addr: $uboot_memory_addr"
$fel_binary exe $uboot_memory_addr
check_return "Execute Uboot fail" -14

###################################################################
##                                                               ##
##                     Flash UBIFS into the NAND                 ##
##                                                               ##
###################################################################
print_big_banner "Flash UBIFS into NAND"

echo "FORCE EXIT"
exit 0
if wait_for_fastboot; then
	fastboot -i 0x1f3a -u flash UBI $output_ubi_rootfs_sparse
	check_return "Fastboot write ubifs fail" -15
else
	echo "failed to flash the UBI image"
fi

print_debug "Remove ${tmp_dir}"
rm -rf ${tmp_dir}

print_banner "All DONE exit"
exit 0
