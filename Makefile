#
# License: BSD clause-2
# michinari.nukazawa@gmail.com
#

CC		:= gcc
TARGET_NAME	:= vecterion_vge
SOURCE_DIR	:= source
OBJECT_DIR	:= object
BUILD_DIR	:= build
PKG_CONFIG	:= pkg-config
CFLAGS		:= -W -Wall -Wextra
CFLAGS		+= -MMD -MP -g -std=c11
CFLAGS		+= -lm
CFLAGS		+= -Werror
CFLAGS		+= -Wno-unused-parameter
CFLAGS		+= -Wunused -Wimplicit-function-declaration \
		 -Wincompatible-pointer-types \
		 -Wbad-function-cast -Wcast-align \
		 -Wdisabled-optimization -Wdouble-promotion \
		 -Wformat-y2k -Wuninitialized -Winit-self \
		 -Wlogical-op -Wmissing-include-dirs \
		 -Wshadow -Wswitch-default -Wundef \
		 -Wwrite-strings -Wunused-macros
#CFLAGS		+= -Wmissing-declarations -Wcast-qual -Wconversion -Wno-sign-conversion
#		 -Wswitch-enum -Wjump-misses-init
INCLUDE		:= -I./include
TARGET		:= $(BUILD_DIR)/$(TARGET_NAME).exe
INCLUDE		+= $(shell $(PKG_CONFIG) --libs --cflags gtk+-3.0)

ifeq ($(OS),Windows_NT)
INCLUDE		+= -I./library/libxml2/win32/include/libxml2 -lxml2
MKDIR_P		:= mkdir
CFLAGS		+= -DOS_Windows=1
else
INCLUDE		+= $(shell xml2-config --cflags --libs)
MKDIR_P		:= mkdir -p
endif

SOURCES		:= $(wildcard $(SOURCE_DIR)/*.c) 
OBJECTS		:= $(subst $(SOURCE_DIR),$(OBJECT_DIR),$(SOURCES:.c=.o))
DEPENDS		:= $(OBJECTS:.o=.d)



.PHONY : all run gdb clean dist_clean

all : $(TARGET)

$(OBJECT_DIR)/%.o : $(SOURCE_DIR)/%.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@


$(OBJECT_DIR)/main.o : include/version.h
$(TARGET) : $(OBJECTS)
	bash ./version.sh $(OBJECT_DIR)
	$(MKDIR_P) $(dir $@)
	$(CC) \
		$^ \
		$(OBJECT_DIR)/version.c \
		$(CFLAGS) \
		$(INCLUDE) \
		-o $(TARGET)

run : $(TARGET)
	./$(TARGET) -i ./library/23.svg

gdb : $(TARGET)
	gdb ./$(TARGET)

clean :
	$(RM) $(TARGET)
	$(RM) $(OBJECTS)
	$(RM) -r $(OBJECT_DIR) $(BUILD_DIR)

dist_clean :
	$(MAKE) clean
	$(MAKE) test_clean

# test
include test/test.Makefile



-include $(DEPENDS)

