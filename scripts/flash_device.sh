#!/bin/bash

#Tmp folder where all needed (definitive) flash image are put
tmp_dir=`mktemp -d -t chip-global-XXXXXX`

#
# Needed binary to build
#
sunxi_nand_image_builder="tools/sunxi-tools/sunxi-nand-image-builder"
fel_binary="tools/sunxi-tools/sunxi-fel"
mkfs_ubifs="tools/chip-mtd-utils/mkfs.ubifs/mkfs.ubifs"
ubinize="tools/chip-mtd-utils/ubi-utils/ubinize"
img2simg="buildroot/output/host/usr/bin/img2simg"

#Default variable value
verbose=false
custom_output_folder=false
buildroot_folder_path=""
device="chip"
nand_type="Toshiba_4G_MLC"
timeout_secondes=120

#Ram addr onboard
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
		mlcopts="-M dist3"
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

	echo "[rootfs]"           	> $tmp_ubicfg
	echo "mode=ubi"           	>> $tmp_ubicfg
	echo "vol_id=0"           	>> $tmp_ubicfg
	echo "vol_type=dynamic"   	>> $tmp_ubicfg
	echo "vol_name=rootfs"    	>> $tmp_ubicfg
	echo "vol_alignment=1"      >> $tmp_ubicfg
	echo "vol_flags=autoresize" >> $tmp_ubicfg
	echo "image=$tmp_ubifs"     >> $tmp_ubicfg

	print_debug "--> ubinize -o $tmp_ubi -p $erase_block_size -m $page_size -s $sub_page_size $mlcopts $tmp_ubicfg"
	$ubinize -o $tmp_ubi -p $erase_block_size -m $page_size -s $sub_page_size $mlcopts $tmp_ubicfg -vv

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
	local nand_spl=$tmp_dir_spl/nand-spl.bin
	local padding=$tmp_dir_spl/padding

	print_debug "output_spl_file   --> $output_spl_file"
	print_debug "input_spl_file    --> $input_spl_file"
	print_debug "erase_block_size  --> $erase_block_size"
	print_debug "page_size         --> $page_size"
	print_debug "oob_size          --> $oob_size"
	print_debug "nand_spl          --> $nand_spl"
	
	print_debug "CMD --> $sunxi_nand_image_builder -c 64/1024 -p $page_size -o $oob_size -u 1024 -e $erase_block_size -b -s $input_spl_file $nand_spl"
	$sunxi_nand_image_builder -c 64/1024 -p $page_size -o $oob_size -u 1024 -e $erase_block_size -b -s $input_spl_file $nand_spl
	
	local nand_spl_size=`filesize $nand_spl`
	local padding_size=$((($erase_block_size-$nand_spl_size)/1024))

	#create padding
	print_debug "spl size: ${nand_spl_size}, padding size: ${padding_size} x 1024 bits"
	dd if=/dev/urandom of=$padding bs=1024 count=$padding_size

	#put nand spl + padding into output_spl
	#cat $nand_spl $padding > $output_spl_file
	cat $nand_spl > $output_spl_file
	
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

	#Erase nand scrub method
	echo "nand erase.chip" > $output_uboot_cmds
	#echo "nand scrub.chip -y" > $output_uboot_cmds
	
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
		echo "Device is POCKETCHIP"
		echo "setenv clear_fastboot 'i2c mw 0x34 0x4 0x00 4;'" >> $output_uboot_cmds
		echo "setenv write_fastboot 'i2c mw 0x34 0x4 66 1; i2c mw 0x34 0x5 62 1; i2c mw 0x34 0x6 30 1; i2c mw 0x34 0x7 00 1'" >> $output_uboot_cmds
		echo "setenv test_fastboot 'i2c read 0x34 0x4 4 0x80200000; if itest.s *0x80200000 -eq fb0; then echo (Fastboot); i2c mw 0x34 0x4 0x00 4; fastboot 0; fi'" >> $output_uboot_cmds

		echo "setenv bootargs root=ubi0:rootfs rootfstype=ubifs rw ubi.mtd=4 quiet lpj=501248 loglevel=3 splash plymouth.ignore-serial-consoles" >> $output_uboot_cmds
		echo "setenv bootpaths 'initrd noinitrd'" >> $output_uboot_cmds
		echo "setenv bootcmd '${NO_LIMIT}run test_fastboot; if test -n \${fel_booted} && test -n \${scriptaddr}; then echo (FEL boot); source \${scriptaddr}; fi; for path in \${bootpaths}; do run boot_\$path; done'" >> $output_uboot_cmds
		echo "setenv boot_initrd 'mtdparts; ubi part UBI; ubifsmount ubi0:rootfs; ubifsload \$fdt_addr_r /boot/sun5i-r8-chip.dtb; ubifsload 0x44000000 /boot/initrd.uimage; ubifsload \$kernel_addr_r /boot/zImage; bootz \$kernel_addr_r 0x44000000 \$fdt_addr_r'" >> $output_uboot_cmds
		echo "setenv boot_noinitrd 'mtdparts; ubi part UBI; ubifsmount ubi0:rootfs; ubifsload \$fdt_addr_r /boot/sun5i-r8-pocketchip.dtb; ubifsload \$kernel_addr_r /boot/zImage; bootz \$kernel_addr_r - \$fdt_addr_r'" >> $output_uboot_cmds
		echo "setenv video-mode" >> $output_uboot_cmds
		echo "setenv dip_addr_r 0x43400000" >> $output_uboot_cmds
		echo "setenv dip_overlay_dir /lib/firmware/nextthingco/chip/early" >> $output_uboot_cmds
		echo "setenv dip_overlay_cmd 'if test -n \"\${dip_overlay_name}\"; then ubifsload \$dip_addr_r \$dip_overlay_dir/\$dip_overlay_name; fi'" >> $output_uboot_cmds
		echo "setenv fel_booted 0" >> $output_uboot_cmds
		echo "setenv bootdelay 1" >> "$output_uboot_cmds"
	else
		echo "Device is CHIP"
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
	
	echo "echo going to fastboot mode" >> $output_uboot_cmds
	echo "fastboot 0" >> $output_uboot_cmds
	echo "reset" >> $output_uboot_cmds

	return 0
}

show_help(){
	echo ""
	echo "== Help =="
	echo "  -h  -- show help (this)"
	echo "  -v  -- Verbose mode"
	echo "  -F  --  Format(optional)   ['Toshiba_4G_MLC' or 'Hynix_8G_MLC' or 'Toshiba_512M_MLC']"
	echo "            -> Default: 'Toshiba_4G_MLC'"
	echo "  -d  -- Device ['chip' or 'pocketchip']"
	echo "            -> Default: 'chip'"
	echo "  -u  -- u-boot.bin path [ex: release/u-boot.bin]"
	echo "  -s  -- sunxi-spl.bin path [ex: release/sunxi-spl.bin]"
	echo "  -r  -- rootfs.tar path [ex: release/prepared_rootfs.tar]"
	echo "  -o  -- output debug folder to put build stuff for analyze path [ex: release/foo]"
	echo ""
	echo ""

	return 0
}

echo "Script start"

while getopts "hvF:d:u:s:r:o:" opt; do
	case $opt in
		F)
			nand_type="${OPTARG}"
			echo "== Format ${nand_type} selected =="
		;;
		v)
			verbose=true
			printf "\n#########################################\n"
			printf "#########################################\n"
			printf "##         Verbose is activated        ##\n"
			printf "#########################################\n"
			printf "#########################################\n\n"
		;;
		h)
			show_help
			exit 0
		;;
		d)
			device="${OPTARG}"
		;;
		u)
			uboot_bin_path="${OPTARG}"
		;;
		s)
			spl_bin_path="${OPTARG}"
		;;
		r)
			rootfs_tar_path="${OPTARG}"
		;;
		o)
			echo "Warning debug folder set"
			custom_output_folder=true
			rm -rf ${tmp_dir}
			tmp_dir="${OPTARG}"
		;;
		\?)
			echo "== Invalid option: -${OPTARG} ==" >&2
			exit 1
		;;
	esac
done

if [ "${uboot_bin_path}" = "" ]; then
	echo "Uboot binary path is empty, abort..."
	exit -1
fi

if [ "${spl_bin_path}" = "" ]; then
	echo "spl binary path is empty, abort..."
	exit -1
fi

if [ "${rootfs_tar_path}" = "" ]; then
	echo "Rootfs.tar path is empty, abort..."
	exit -1
fi

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

#Output binary
output_spl="${tmp_dir}/spl.bin"
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



case $nand_type in
	"Hynix_8G_MLC")
		UBI_nand_type="mlc"
		UBI_max_leb_count=4096
		UBI_max_leb_size=4194304
		UBI_page_size=16384
		UBI_sub_page_size=16384
		UBI_oobsize=1664
	;;
	"Toshiba_4G_MLC")
		UBI_nand_type="mlc"
		UBI_max_leb_count=4096
		UBI_max_leb_size=4194304
		UBI_page_size=16384
		UBI_sub_page_size=16384
		UBI_oobsize=1280
	;;
	"Toshiba_512M_MLC")
		echo "Flash no supported for the moment"
		exit 0
	;;
	\?)
		echo "== Invalid nand type: $nand_type ==" >&2
		exit 1
	;;
esac

print_banner "Prepare SPL"
prepare_spl $output_spl $spl_bin_path $UBI_max_leb_size $UBI_page_size $UBI_oobsize

print_banner "Prepare UBOOT"
prepare_uboot $output_uboot_bin $uboot_bin_path $UBI_max_leb_size

## Prepare ubifs (ROOTFS)
print_banner "Prepare UBI"
prepare_ubi $rootfs_tar_path $output_ubi_rootfs_sparse $UBI_nand_type $UBI_max_leb_count $UBI_max_leb_size $UBI_page_size $UBI_sub_page_size

uboot_size=$(filesize $output_uboot_bin | xargs printf "0x%08x")
PADDED_SPL_SIZE=$(filesize "${output_spl}")
size_to_write_padded_spl=$((${PADDED_SPL_SIZE} / ( ${UBI_page_size} + ${UBI_oobsize})))

print_debug "uboot size -> $uboot_size"

print_banner "Create uboot cmd"
create_uboot_start_cmd $output_uboot_cmds $uboot_size $size_to_write_padded_spl

print_banner "make image uboot"
mkimage -A arm -T script -C none -n "flash $device" -d $output_uboot_cmds $output_uboot_script

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
$fel_binary spl $spl_bin_path
# wait for DRAM initialization to complete
sleep 1

echo "Write uboot: addr: $uboot_memory_addr, bin: $output_uboot_bin"
$fel_binary write $uboot_memory_addr $output_uboot_bin

echo "Write SPL: addr: $spl_memory_addr, bin: $output_spl"
$fel_binary write $spl_memory_addr $output_spl

echo "Write Uboot script: addr: $uboot_script_addr, path: $output_uboot_script"
$fel_binary write $uboot_script_addr $output_uboot_script

echo "Execute UBoot: addr: $uboot_memory_addr"
$fel_binary exe $uboot_memory_addr

###################################################################
##                                                               ##
##                     Flash UBIFS into the NAND                 ##
##                                                               ##
###################################################################
print_big_banner "Flash UBIFS into NAND"

if wait_for_fastboot; then
	fastboot -i 0x1f3a -u flash UBI $output_ubi_rootfs_sparse
else
	echo "failed to flash the UBI image"
fi

#Don't remove tmp folder if custom_output_folder option used
if [ $custom_output_folder = false ]; then
	print_debug "Remove ${tmp_dir}"
	rm -rf ${tmp_dir}
fi

print_banner "All DONE exit"
exit 0
