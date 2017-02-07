linux:
	@cp configs/chip/linux.conf linux/.config && ./make_target.sh

buildroot:
	@cp configs/chip/buildroot.conf buildroot/.config && $(MAKE)

uboot:
	@cd uboot && $(make)


chip_bot:
	@echo "[ Build CHIP_Bot ]"

pocketchip_controler:
	@echo "[ Build Pocketchip controler ]"

pc_controler:
	@echo "[ Build PC Controler ]"



all:chipbot pocketchip_controler pc_controler

distclean:

clean:
