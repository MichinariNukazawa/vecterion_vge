#
# License: BSD clause-2
# michinari.nukazawa@gmail.com
#

CC			= gcc
TARGET_NAME		= etaion_vector
SOURCE_DIR		= source
OBJECT_DIR		= object
BUILD_DIR		= build
PKG_CONFIG	= pkg-config
CFLAGS		= -W -Wall -MMD -MP -g -std=c11
CFLAGS		+= -lm
CFLAGS		+= -Wno-unused-parameter # please check!
CFLAGS		+= $(shell $(PKG_CONFIG) --libs --cflags gtk+-3.0)
CFLAGS		+= $(shell xml2-config --cflags --libs)
INCLUDE		= -I./ -I./include
TARGET		= $(BUILD_DIR)/$(TARGET_NAME).exe

SOURCES		= $(shell ls $(SOURCE_DIR)/*.c) 
OBJECTS		= $(subst $(SOURCE_DIR),$(OBJECT_DIR), $(SOURCES:.c=.o))
DEPENDS		= $(OBJECTS:.o=.d)



all: $(TARGET)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(TARGET): $(OBJECTS)
	$(CC) $^ $(CFLAGS) $(INCLUDE) \
		-o $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb ./$(TARGET)

clean:
	$(RM) $(TARGET) $(OBJECT_DIR)/* $(BUILD_DIR)/*

dist_clean:
	$(MAKE) clean
	$(MAKE) -f test/Makefile clean


## test

export
test_run: $(OBJECTS)
	$(MAKE) -f test/Makefile run

test_clean:
	$(MAKE) -f test/Makefile clean

-include $(DEPENDS)

