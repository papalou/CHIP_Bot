BIN=chip_bot
SRC=                             \
	main.c

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
LDCONFIG=$(CROSS_COMPILE)ldconfig
STRIP=$(CROSS_COMPILE)strip
STAGING=$(SYSROOT)/../staging/
ARFLAGS=rcs
INCLUDE=-I.                           \
        -I$(STAGING)/usr/include      \
		-I../libcommon/include     \

CFLAGS= -Wall -g -O0 -Wextra -fPIC -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
LDFLAGS= -L. ../libcommon/libcommon.a
OBJECTS=$(SRC:.c=.o)

all:$(BIN)
	@echo "[ Chip_Bot built ]"

libcommon:
	@echo "[Config libcommon]"
	@cp ../configs/libcommon_arm.conf ../libcommon/cross_toolchain.config
	@echo "[Build libcommon]"
	@cd ../libcommon/ && ./make_target.sh

$(BIN):libcommon $(OBJECTS)
	@ echo cc link  ... -o $@
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
#	$(STRIP)  $@

%.o:%.c
	@ echo cc $< ...
	@$(CC) $(INCLUDE) $(CFLAGS) $< -c -o $@

.PHONY:clean

clean:
	@rm -f $(OBJECTS)
	@rm -f $(BIN)
