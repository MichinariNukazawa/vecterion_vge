#
# License: BSD clause-2
# michinari.nukazawa@gmail.com
#


GTEST_DIR	= test/googletest/googletest
GTEST_LIBS	= test/libgtest.a test/libgtest_main.a

CXX			= g++
TEST_SOURCE_DIR		= test/utest
TEST_OBJECT_DIR		= object
TEST_BUILD_DIR		= object
CXX_FLAGS		= -std=c++11 -g -MMD -MP -W -Wall -Wextra
CXX_FLAGS		+= $(filter-out -std=c11,$(CFLAGS))
TEST_FLAGS		= -lpthread -lgtest_main -lgtest
TEST_FLAGS		+= -Ltest
TEST_FLAGS		+= -Dinclude_PV_TEST -Dinclude_ET_TEST
TEST_INCLUDE		= -I$(GTEST_DIR)/include
TEST_INCLUDE		+= -Iinclude

TEST_SOURCES		= $(wildcard $(TEST_SOURCE_DIR)/*.cpp)
TEST_OBJECTS		= $(subst $(TEST_SOURCE_DIR),$(TEST_OBJECT_DIR), $(TEST_SOURCES:.cpp=.o))
TEST_DEPENDS		= $(TEST_OBJECTS:.o=.d)
TEST_BINS		= $(TEST_OBJECTS:.o=.exe)
TEST_TARGETS		= $(TEST_BINS)


utest_all: $(TEST_TARGETS)

$(TEST_OBJECT_DIR)/%.o: $(TEST_SOURCE_DIR)/%.cpp
	$(CXX) -o $@ $(INCLUDE) $(TEST_INCLUDE) $(CXX_FLAGS) $(TEST_FLAGS) \
		-c $<

O_OBJECTS = $(filter-out object/main.o,$(OBJECTS))
$(TEST_OBJECT_DIR)/test_test.exe: object/pv_general.o
$(TEST_OBJECT_DIR)/test_pv_general.exe: object/pv_general.o
$(TEST_OBJECT_DIR)/test_pv_vg.exe: $(O_OBJECTS)
$(TEST_OBJECT_DIR)/test_et_doc_history.exe: $(O_OBJECTS)

$(TEST_BUILD_DIR)/test_%.exe: $(TEST_OBJECT_DIR)/test_%.o
	$(CXX) \
		$^ \
		$(INCLUDE) $(TEST_INCLUDE) \
		$(CXX_FLAGS) $(TEST_FLAGS) \
		$(GTEST_LIBS) \
		-o $@

empty =
space = $(empty) $(empty)
command_glue = $(empty) && $(empty)
TEST_COMMAND = $(subst $(space),$(command_glue),$(TEST_TARGETS))
run: $(TEST_TARGETS)
	$(TEST_COMMAND)

clean:
	rm -rf $(TEST_OBJECT_DIR)/test_*.o
	rm -rf $(TEST_OBJECT_DIR)/test_*.d
	rm -rf $(TEST_OBJECT_DIR)/test_*.exe
	rm -rf $(TEST_BUILD_DIR)/test_*.o
	rm -rf $(TEST_BUILD_DIR)/test_*.d
	rm -rf $(TEST_BUILD_DIR)/test_*.exe

-include $(TEST_DEPENDS)
