CXX=g++

ifdef DEBUG
	BOOST_ROOT=/opt/boost/1.55.0-debug
	CXXFLAGS+=-O0 -g
else
	BOOST_ROOT=/opt/boost/1.55.0-release
	CXXFLAGS+=-O3 -finline
endif


CXXFLAGS+=-std=c++14 -L$(BOOST_ROOT)/stage/lib -Wl,-rpath $(BOOST_ROOT)/stage/lib
INCLUDES=-I$(BOOST_ROOT)
LIBS=-lrt -lboost_thread -lboost_system -lboost_program_options -lboost_serialization -lboost_chrono
ADDITIONAL_SOURCES=
PROGRAMS=async_stm_prototype
DIRECTORIES=build

all: directories $(PROGRAMS)

.PHONY: directories
directories: $(DIRECTORIES)/  

$(DIRECTORIES)/:
	mkdir -p $@ 

% : %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(LIBS) $(ADDITIONAL_SOURCES) $< -o build/$@

clean:
	rm -rf $(DIRECTORIES) 

