BIN=pibot
SRC=help.c                       \
	uart.c                       \
	motor.c                      \
	i2c_servo.c                  \
	pca9685.c                    \
	communication_server_cmd.c   \
	communication_server_video.c \
	main.c                       \

CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar
LDCONFIG=$(CROSS_COMPILE)ldconfig
STRIP=$(CROSS_COMPILE)strip
PKG_CONFIGS_CFLAGS:=$(shell $(SYSROOT)/usr/bin/pkg-config --cflags zlib)
PKG_CONFIGS_LDLAGS:=$(shell $(SYSROOT)/usr/bin/pkg-config --libs zlib)
STAGING=$(SYSROOT)/../staging/
ARFLAGS=rcs
INCLUDE=-I.                           \
        -I$(STAGING)/usr/include      \
        -I../../libcommon/src/minizip \
		-I../../libcommon/src/        \
		-I../common_pibot             \

CFLAGS=$(PKG_CONFIGS_CFLAGS) -pthread  -Wall -g -O0 -Wextra -fPIC -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
LDFLAGS=$(PKG_CONFIGS_LDLAGS) -L. ../../libcommon/libcommon.a -lrt -lssl -lz -lcrypto
OBJECTS=$(SRC:.c=.o)

all:$(BIN)
	@echo "[ PiBot built ]"

libcommon:
	@echo "[Config libcommon]"
	@cp ../../config/libcommon_arm_config ../../libcommon/.config
	@echo "[Build libcommon]"
	@cd ../../libcommon/ && ./make_target.sh

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
