#include "hdf5parser.h"

//h5c++ -std=c++11 -lhdf5 -lhdf5_cpp -o builder hdf5parser_main.cpp hdf5parser.cpp 

int main() {
	auto wcts = std::chrono::system_clock::now();

	stockLocatorMap_init();
	compType_init();
	hdf5GroupHierarchy_init();
	hdf5PopulateData();

	std::chrono::duration<double> wctduration = (std::chrono::system_clock::now() - wcts);
	std::cout << "Finished in " << wctduration.count() << " seconds [Wall Clock]" << std::endl;
	return 0;
}
