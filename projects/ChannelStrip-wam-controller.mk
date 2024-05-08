include ../config/ChannelStrip-web.mk

TARGET = ../build-web/scripts/ChannelStrip-web.js

SRC += $(WEB_SRC)
CFLAGS += $(WEB_CFLAGS)
CFLAGS += $(EXTRA_CFLAGS)
LDFLAGS += $(WEB_LDFLAGS) \
-s EXPORTED_FUNCTIONS=$(WEB_EXPORTS)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRC)
