HPX_LOCATION=/home/wash/install/hpx/gcc-4.8-release
export PKG_CONFIG_PATH=$(HPX_LOCATION)/lib/pkgconfig

CXX=g++-4.9

ifdef DEBUG
	CXXFLAGS+=`pkg-config --cflags --libs hpx_applicationd`
else
	CXXFLAGS+=`pkg-config --cflags --libs hpx_application`
endif


CXXFLAGS+=-std=c++1y -I. 
ADDITIONAL_SOURCES=
PROGRAMS=unit_tests
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

