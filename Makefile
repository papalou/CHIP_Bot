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

prepare_rootfs:linux buildroot uboot sunxi-tools chip-mtd-utils src
	@echo "[ Prepare Rootfs.tar archive with release folder ]"
	@sudo ./scripts/prepare_rootfs.sh -i release/rootfs.tar -o release/prepared_rootfs.tar -f release/

release:prepare_rootfs
	@echo "[ Generate release ]"
	@rm -rf release/ && mkdir release/
	@cp linux/arch/arm/boot/zImage                      release/
	@mkdir release/linux_modules/
	@cp -r linux/target/lib/                            release/linux_modules/
	@cp linux/arch/arm/boot/dts/sun5i-r8-chip.dtb       release/
	@cp linux/arch/arm/boot/dts/sun5i-r8-pocketchip.dtb release/
	@cp buildroot/output/images/rootfs.tar              release/
	@cp uboot/spl/sunxi-spl.bin                         release/
	@cp uboot/u-boot-dtb.bin                            release/
	@cp src/chip_bot                                    release/

linux:
	@echo "[ Build GNU/Linux kernel ]"
	@cp configs/linux.conf linux/.config
	@cd linux && rm -rf target/ && mkdir target/
	@cd linux && ./make_linux.sh
	@echo "[ Install GNU/Linux kernel modules ]"
	@cd linux && ./make_linux.sh modules_install install

buildroot:
	@echo "[ Build Buildroot ]"
	@cp configs/buildroot.conf buildroot/.config
	@cd buildroot/ && $(MAKE)

uboot:
	@echo "[ Build UBoot bootloader ]"
	@cp configs/uboot.conf uboot/.config
	@cd uboot && ./make_uboot.sh

sunxi-tools:
	@echo "[ Build tools/sunxi-tools ]"
	@cd tools/sunxi-tools/ && $(MAKE)
	@cd tools/sunxi-tools/ && $(MAKE) misc

chip-mtd-utils:
	@echo "[ Build tools/chip-mtd-utils ]"
	@cd tools/chip-mtd-utils/ && $(MAKE)

src:buildroot
	@echo "[ Build Source ]"
	@cd src/ && ./make_target.sh

distclean:
	@echo "[ Distclean all project ]"
	@cd linux/ && $(MAKE) distclean
	@cd buildroot/ && $(MAKE) distclean
	@cd uboot/ && $(MAKE) distclean
	@cd sunxi-tools/ && $(MAKE) clean
	@cd libcommon/ && $(MAKE) distclean
	@cd src/ && $(MAKE) distclean
	@rm -rf release/

clean:
	@echo "[ Clean all project ]"
	@cd linux/ && $(MAKE) clean
	@cd buildroot/ && $(MAKE) clean
	@cd uboot/ && $(MAKE) clean
	@cd sunxi-tools/ && $(MAKE) clean
	@cd libcommon/ && $(MAKE) clean
	@cd src/ && $(MAKE) clean

.PHONY:                \
all                    \
src                    \
linux                  \
release                \
buildroot              \
uboot                  \
sunxi-tools            \
distclean              \
clean
