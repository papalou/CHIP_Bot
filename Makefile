all:release

flash_chipbot_4G:
	@echo "[ Flash CHIP_bot 4G ]"
	@sudo ./scripts/flash_device.sh -d chip -F Toshiba_4G_MLC -u release/u-boot-dtb.bin -s release/sunxi-spl.bin -r release/prepared_rootfs.tar

flash_chipbot_8G:
	@echo "[ Flash CHIP_bot 8G ]"
	@sudo ./scripts/flash_device.sh -d chip -F Hynix_8G_MLC -u release/u-boot-dtb.bin -s release/sunxi-spl.bin -r release/prepared_rootfs.tar

flash_pocketchip_4G:
	@echo "[ Flash PocketCHIP 4G ]"
	@sudo ./scripts/flash_device.sh -d pocketchip -F Toshiba_4G_MLC -u release/u-boot-dtb.bin -s release/sunxi-spl.bin -r release/prepared_rootfs.tar

flash_pocketchip_8G:
	@echo "[ Flash PocketCHIP 8G ]"
	@sudo ./scripts/flash_device.sh -d pocketchip -F Hynix_8G_MLC -u release/u-boot-dtb.bin -s release/sunxi-spl.bin -r release/prepared_rootfs.tar

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

distclean:
	@echo "[ Distclean all project ]"
	@cd buildroot/ && $(MAKE) distclean
	@cd sunxi-tools/ && $(MAKE) clean
	@cd libcommon/ && $(MAKE) distclean
	@cd src/ && $(MAKE) distclean
	@rm -rf release/

clean:
	@echo "[ Clean all project ]"
	@cd buildroot/ && $(MAKE) clean
	@cd sunxi-tools/ && $(MAKE) clean
	@cd libcommon/ && $(MAKE) clean
	@cd src/ && $(MAKE) clean

.PHONY:                \
all                    \
src                    \
release                \
buildroot              \
sunxi-tools            \
distclean              \
clean
