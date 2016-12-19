#include "dataparser.h"

//g++ -std=c++11 -o output dataparser_main.cpp dataparser.cpp
int main() {
	clock_t begin = clock();
	FMap decoder;
	std::string inputFileName= "../data";
	std::string outputFileName = "parsedData.csv";
	unsigned long filesize = findFileSize(inputFileName);

	//input file stream, reading binary input
	std::ifstream inputFile(inputFileName.c_str(), std::ios::in | std::ios::binary);
	//output file stream, writing ascii characters
	std::ofstream outputFile(outputFileName.c_str(), std::ios::out);
	unsigned char buffer[2];
	while(inputFile.read((char*)buffer, 2)) {
		char typeBuffer[1];
		inputFile.read((char*)typeBuffer, 1);
		decoder.call(typeBuffer[0],outputFile, inputFile, typeBuffer, buffer);
	}
	inputFile.close();
	outputFile.close();
	clock_t end = clock();
	double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	std::cout<<elapsed_secs<<std::endl;
}