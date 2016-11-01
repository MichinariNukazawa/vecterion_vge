#
# License: BSD clause-2
# michinari.nukazawa@gmail.com
#


CXX			:= g++
CXX_FLAGS		:= -std=c++11 -g -MMD -MP -W -Wall -Wextra



GTEST_DIR		:= library/googletest/googletest
GTEST_LIBS		:= $(GTEST_DIR)/libgtest.a $(GTEST_DIR)/libgtest_main.a

TEST_SOURCE_DIR		:= test/utest
TEST_OBJECT_DIR		:= $(OBJECT_DIR)
TEST_BUILD_DIR		:= $(OBJECT_DIR)
TEST_FLAGS		:= -lpthread -lgtest_main -lgtest
TEST_FLAGS		+= -Ltest
TEST_FLAGS		+= -Dinclude_PV_TEST -Dinclude_ET_TEST
TEST_INCLUDE		:= -I$(GTEST_DIR)/include
TEST_INCLUDE		+= -Iinclude

TEST_SOURCES		:= $(wildcard $(TEST_SOURCE_DIR)/*.cpp)
TEST_OBJECTS		:= $(subst $(TEST_SOURCE_DIR),$(TEST_OBJECT_DIR),$(TEST_SOURCES:.cpp=.o))
TEST_DEPENDS		:= $(TEST_OBJECTS:.o=.d)
TEST_BINS		:= $(TEST_OBJECTS:.o=.exe)
TEST_TARGETS		:= $(TEST_BINS)

TEST_O_OBJECTS		:= $(filter-out object/main.o,$(OBJECTS))
TEST_COMMAND := true $(foreach TEST_TARGET,$(TEST_TARGETS), && $(TEST_TARGET))



.PHONY : test argtest_run unittest_run test_clean
test : argtest_run unittest_run

$(TEST_OBJECT_DIR)/%.o : $(TEST_SOURCE_DIR)/%.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(INCLUDE) $(TEST_INCLUDE) $(CXX_FLAGS) $(TEST_FLAGS) \
		-c $< -o $@

$(TEST_BUILD_DIR)/test_%.exe : $(TEST_OBJECT_DIR)/test_%.o $(TEST_O_OBJECTS)
	$(MKDIR_P) $(dir $@)
	$(CXX) \
		$^ \
		$(INCLUDE) $(TEST_INCLUDE) \
		$(CXX_FLAGS) $(TEST_FLAGS) \
		$(GTEST_LIBS) \
		-o $@

unittest_run : $(TEST_TARGETS)
	$(TEST_COMMAND)



ARGTEST_OUTPUT=$(OBJECT_DIR)/argtest_output.svg
argtest_run : $(TARGET)
	$(RM) $(ARGTEST_OUTPUT)
	./$(TARGET) -i library/23.svg -o $(ARGTEST_OUTPUT)
	[ -s $(ARGTEST_OUTPUT) ] # file is not zero size



test_clean :
	$(RM) $(TEST_TARGETS)
	$(RM) $(TEST_OBJECTS)
	$(RM) -r $(TEST_OBJECT_DIR)
	$(RM) -r $(TEST_BUILD_DIR)



-include $(TEST_DEPENDS)

