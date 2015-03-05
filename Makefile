CXX=hpxcxx

CXXFLAGS+=-std=c++1y -I. -g
ADDITIONAL_SOURCES=
PROGRAMS=binary_tree unit_tests concurrency_tests 
DIRECTORIES=build

all: directories $(PROGRAMS)

.PHONY: directories
directories: $(DIRECTORIES)/  

$(DIRECTORIES)/:
	mkdir -p $@ 

% : %.cpp
	$(CXX) -DASTM_HPX $(ADDITIONAL_SOURCES) $< --exe=$@ -o build/$@ $(CXXFLAGS) 

clean:
	rm -rf $(DIRECTORIES) 

