#include "hdf5_bookbuilder.h"

//h5c++ -std=c++11 -lhdf5 -lhdf5_cpp -o bookbuilder hdf5_bookbuilder_main.cpp hdf5_bookbuilder.cpp   

int main(int argc, char* argv[]){
	if(argc < 2) {
		std::cout<<"Not enough arguments."<<std::endl;
		std::cout<<"Usage: ./program StockSymbol1 [2 ... 5]"<<std::endl;
		return -1;
	}
	if(argc > 5) {
		std::cout<<"Too many arguments"<<std::endl;
		std::cout<<"Only up to five stocks are allowed at one time."<<std::endl;
		return -1;
	}
	//This map uses stock symbol as keys and stocklocate as values
	std::map<std::string, unsigned short> stockSymbolMap;

	auto wcts = std::chrono::system_clock::now();
	for(int i = 1; i < argc; i++ ) {
		//use a map so that we don't how to check duplicates
		stockSymbolMap[argv[i]] = 0;
	}

	stockLocatorMap_init(stockSymbolMap);
	compType_init();
	compTypeO_init();
	compTypeR_init();
	hdf5GroupHierarchy_init();
	hdf5BuildOrderBook(stockSymbolMap);

	std::chrono::duration<double> wctduration = (std::chrono::system_clock::now() - wcts);
	std::cout << "Finished in " << wctduration.count() << " seconds [Wall Clock]" << std::endl;
	return 0;
}
