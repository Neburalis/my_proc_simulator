CXX      := g++
CXXFLAGS := -std=gnu++17 -g -O2 -Dtolerance_lvl=0
INCLUDES := -I..                              \
            -I.                               \
            -I../../string_and_thong          \
            -I../../io_utils/include          \
            -I../../smart_stack/stack         \
            -I../../smart_stack/stack/src

SRCS :=                                            \
    processor.cpp                                  \
    executors.cpp                                  \
    ../../smart_stack/stack/src/stack_.cpp         \
    ../../string_and_thong/stringNthong.cpp        \
    ../../io_utils/src/io_utils.cpp

BUILDDIR := build
OBJS     := $(addprefix $(BUILDDIR)/, $(notdir $(SRCS:.cpp=.o)))
TARGET   := newproc.out

VPATH := . ../../string_and_thong ../../io_utils/src \
           ../../smart_stack/stack/src

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILDDIR)/%.o: %.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR) $(TARGET)
