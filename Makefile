CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -g -I/usr/local/include
LDFLAGS := -L/usr/local/lib
LIBS := -lraylib -lm -lpthread -ldl -lrt
SRCDIR := src
BUILDDIR := build
TARGET := myprog

SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

.PHONY: all clean dirs

all: dirs $(BUILDDIR)/$(TARGET)

dirs:
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) $(LDFLAGS) $(LIBS) -o $@

clean:
	rm -rf $(BUILDDIR)/*
