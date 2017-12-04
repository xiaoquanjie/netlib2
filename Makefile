ALL_DIR_SRC 	= ./ ./demo
INCLUDES 	= -Isocket -Inetio -I./
CPPFLAGS	= -pthread -gdwarf-2

all : project

include Makefile.inc

project : $(ALL_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^
