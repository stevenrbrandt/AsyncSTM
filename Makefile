HPX_LOCATION=/home/wash/install/hpx/gcc-4.8-release
export PKG_CONFIG_PATH=$(HPX_LOCATION)/lib/pkgconfig

CXX=g++-4.9

ifdef DEBUG
	CXXFLAGS+=`pkg-config --cflags --libs hpx_applicationd` -DASTM_HPX
else
	CXXFLAGS+=`pkg-config --cflags --libs hpx_application` -DASTM_HPX
endif


CXXFLAGS+=-std=c++1y -I. -g
ADDITIONAL_SOURCES=
PROGRAMS=binary_tree
DIRECTORIES=build

all: directories $(PROGRAMS)

.PHONY: directories
directories: $(DIRECTORIES)/  

$(DIRECTORIES)/:
	mkdir -p $@ 

% : %.cpp
	$(CXX) $(ADDITIONAL_SOURCES) $< -o build/$@ $(CXXFLAGS) 

clean:
	rm -rf $(DIRECTORIES) 

