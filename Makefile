TARGET=mtkeepmgr

OBJ=\
	con_file.o	\
	mt7610.o	\
	mt7620.o	\
	mt7628.o	\
	mtkeepmgr.o	\
	utils.o

DEP=$(OBJ:%.o=%.d)

DEFS=

HAVE_LIBUSB?=$(shell pkg-config libusb-1.0 && echo y || echo n)

CONFIG_CON_USB?=$(HAVE_LIBUSB)

ifeq ($(CONFIG_CON_USB),y)
DEFS+=-DCONFIG_CON_USB
OBJ+=con_usb.o
con_usb.o: CFLAGS+=$(shell pkg-config --cflags libusb-1.0)
LDFLAGS+=$(shell pkg-config --libs libusb-1.0)
endif

CFLAGS += -Wall -g

DEPFLAGS=-MMD -MP

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(DEPFLAGS) $(CFLAGS) $(DEFS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJ)
	rm -rf $(DEP)

-include $(DEP)
