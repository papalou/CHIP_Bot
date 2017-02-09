all:                        \
chip_bot                    \
pocketchip_controler        \
pc_controler

chip_bot:buildroot linux uboot
	@echo "[ Build CHIP_Bot ]"
	@cd src/chip_bot/ && ./make_target.sh

pocketchip_controler:buildroot linux uboot
	@echo "[ Build Pocketchip controler ]"
	@cd src/pocketchip_controler/ && ./make_target.sh

pc_controler:
	@echo "[ Build PC Controler ]"
	@cd src/pc_controler/ && $(MAKE)

linux:
	@echo "[ Build GNU/Linux kernel ]"
	@cp configs/chip/linux.conf linux/.config
	@cd linux && rm -rf target/ && mkdir target/
	@cd linux && ./make_linux.sh
	@echo "[ Install GNU/Linux kernel into buildroot target ]"
	@cd linux && ./make_linux.sh modules_install install

buildroot:
	@echo "[ Build Buildroot ]"
	@cp configs/chip/buildroot.conf buildroot/.config
	@cd buildroot/ && $(MAKE)

uboot:
	@echo "[ Build UBoot bootloader ]"
	@cd uboot && $(MAKE)
#TODO

distclean:
	@echo "[ Distclean all project ]"
	@cd linux/ && $(MAKE) distclean
	@cd buildroot/ && $(MAKE) distclean
	@cd uboot/ && $(MAKE) distclean
	@cd libcommon/ && $(MAKE) distclean
	@cd src/ && $(MAKE) distclean

clean:
	@echo "[ Clean all project ]"
	@cd linux/ && $(MAKE) clean
	@cd buildroot/ && $(MAKE) clean
	@cd uboot/ && $(MAKE) clean
	@cd libcommon/ && $(MAKE) clean
	@cd src/ && $(MAKE) clean

.PHONY:                \
all                    \
chip_bot               \
pocketchip_controler   \
pc_controler           \
linux                  \
buildroot              \
uboot                  \
distclean              \
clean
