#
# License: BSD clause-2
# michinari.nukazawa@gmail.com
#

CC			= gcc
TARGET_NAME		= etaion_vector
SOURCE_DIR		= source
OBJECT_DIR		= object
BUILD_DIR		= build
MKDIR			= mkdir -p
PKG_CONFIG	= pkg-config
CFLAGS		= -W -Wall -Wextra
CFLAGS		+= -MMD -MP -g -std=c11
CFLAGS		+= -lm
CFLAGS		+= -Werror
CFLAGS		+= -Wno-unused-parameter
CFLAGS		+= -Wunused -Wimplicit-function-declaration -Wincompatible-pointer-types \
			 -Wbad-function-cast -Wcast-align \
			 -Wdisabled-optimization -Wdouble-promotion \
			 -Wformat-y2k -Wuninitialized -Winit-self \
			 -Wlogical-op -Wmissing-include-dirs \
			 -Wshadow -Wswitch-default -Wundef \
			 -Wwrite-strings -Wunused-macros
#CFLAGS		+= -Wmissing-declarations -Wcast-qual -Wconversion -Wno-sign-conversion
# -Wswitch-enum -Wjump-misses-init
INCLUDE		= -I./include
TARGET		= $(BUILD_DIR)/$(TARGET_NAME).exe
INCLUDE			+= $(shell $(PKG_CONFIG) --libs --cflags gtk+-3.0)

ifeq ($(OS),Windows_NT)
INCLUDE			+= -I./library/libxml2/win32/include/libxml2 -lxml2
MKDIR			= mkdir
CFLAGS			+= -DOS_Windows=1
else
INCLUDE			+= $(shell xml2-config --cflags --libs)
endif

SOURCES		= $(wildcard $(SOURCE_DIR)/*.c) 
OBJECTS		= $(subst $(SOURCE_DIR),$(OBJECT_DIR), $(SOURCES:.c=.o))
DEPENDS		= $(OBJECTS:.o=.d)



all: $(TARGET)

$(OBJECT_DIR)/%.o: $(SOURCE_DIR)/%.c
	- $(MKDIR) $(OBJECT_DIR)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

$(TARGET): $(OBJECTS)
	- $(MKDIR) $(BUILD_DIR)
	$(CC) $^ $(CFLAGS) $(INCLUDE) \
		-o $(TARGET)

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb ./$(TARGET)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJECT_DIR)/* $(BUILD_DIR)/*

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
