all:release

flash_chip_4G:
	@echo "[ Flash CHIP_bot 4G ]"
	@sudo ./scripts/flash_device.sh -d chip -F Toshiba_4G_MLC -u release/u-boot-dtb.bin -s release/sunxi-spl.bin -r release/rootfs.tar

flash_chip_8G:
	@echo "[ Flash CHIP_bot 8G ]"
	@sudo ./scripts/flash_device.sh -d chip -F Hynix_8G_MLC -u release/u-boot-dtb.bin -s release/sunxi-spl.bin -r release/rootfs.tar

flash_pocketchip_4G:
	@echo "[ Flash PocketCHIP 4G ]"
	@sudo ./scripts/flash_device.sh -d pocketchip -F Toshiba_4G_MLC -u release/u-boot-dtb.bin -s release/sunxi-spl.bin -r release/rootfs.tar

flash_pocketchip_8G:
	@echo "[ Flash PocketCHIP 8G ]"
	@sudo ./scripts/flash_device.sh -d pocketchip -F Hynix_8G_MLC -u release/u-boot-dtb.bin -s release/sunxi-spl.bin -r release/rootfs.tar

release:buildroot sunxi-tools chip-mtd-utils src
	@echo "[ Generate release ]"
	@rm -rf release/ && mkdir release/
	@cp buildroot/output/images/rootfs.tar              release/
	@cp buildroot/output/images/sunxi-spl.bin           release/
	@cp buildroot/output/images/u-boot-dtb.bin          release/
	@cp src/chip_bot                                    release/

buildroot:
	@echo "[ Build Buildroot ]"
	@cp configs/buildroot.conf buildroot/.config
	@cd buildroot/ && $(MAKE)

sunxi-tools:
	@echo "[ Build tools/sunxi-tools ]"
	@cd tools/sunxi-tools/ && $(MAKE)
	@cd tools/sunxi-tools/ && $(MAKE) misc

chip-mtd-utils:
	@echo "[ Build tools/chip-mtd-utils ]"
	@cd tools/chip-mtd-utils/ && $(MAKE)

src:
	@echo "[ Build Source ]"
	@cd src/ && ./make_target.sh

######################################
#                                    #
#            MAKE DEV ENV            #
#                                    #
######################################
dev_env:
	@echo "[ Make dev env ]"
	@cd scripts/ && ./make_dev_env.sh

######################################
#                                    #
#     DISTCLEAN AND CLEAN SECTION    #
#                                    #
######################################
distclean:distclean_buildroot distclean_libcommon distclean_release clean_sunxi_tools clean_chip_mtd_utils clean_src

distclean_buildroot:
	@echo "[ Distclean buildroot ]"
	@cd buildroot/ && $(MAKE) distclean

distclean_libcommon:
	@echo "[ Distclean libcommon ]"
	@cd libcommon/ && $(MAKE) distclean

distclean_release:
	@echo "[ Remove release folder ]"
	@rm -rf release/

clean:clean_buildroot clean_sunxi_tools clean_chip_mtd_utils clean_libcommon clean_src

clean_buildroot:
	@echo "[ Clean Buildroot ]"
	@cd buildroot/ && $(MAKE) clean

clean_sunxi_tools:
	@echo "[ Clean sunxi tools ]"
	@cd tools/sunxi-tools/ && $(MAKE) clean

clean_chip_mtd_utils:
	@echo "[ Clean chip-mtd-utils ]"
	@cd tools/chip-mtd-utils/ && $(MAKE) clean

clean_libcommon:
	@echo "[ Clean libcommon ]"
	@cd libcommon/ && $(MAKE) clean

clean_src:
	@echo "[ Clean source ]"
	@cd src/ && $(MAKE) clean

.PHONY:                \
all                    \
src                    \
release                \
buildroot              \
sunxi-tools            \
distclean              \
distclean_buildroot    \
distclean_libcommon    \
distclean_release      \
clean                  \
clean_buildroot        \
clean_sunxi_tools      \
clean_chip_mtd_utils   \
clean_libcommon        \
clean_src
