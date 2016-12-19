#include <sys/stat.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <unordered_map>
#include <string>
#include <vector>

unsigned short twoByteCharToShort(unsigned char* buffer){
	return (unsigned short) ((buffer[0] << 8)| buffer[1]);
}
unsigned long twoByteCharToLong(unsigned char* buffer) {
	unsigned long result = 0;
	for(int i = 0; i < 2; i++) {
		result = (result<<8) + buffer[i];
	}
	return result;
}


unsigned long findFileSize(std::string filename){
	struct stat result;
	unsigned long succeed = stat(filename.c_str(), &result);
	if(succeed == 0)
		return result.st_size;
	else
		return -1;
}
struct MessageFormat {
	std::unordered_map<char, unsigned short> formatMap;

	MessageFormat() {
		formatMap.emplace('S', 12);
		formatMap.emplace('R', 39);
		formatMap.emplace('H', 25);
		formatMap.emplace('Y', 20);
		formatMap.emplace('L', 26);
		formatMap.emplace('V', 35);
		formatMap.emplace('W', 12);
		formatMap.emplace('K', 28);
		formatMap.emplace('A', 36);
		formatMap.emplace('F', 40);
		formatMap.emplace('E', 31);
		formatMap.emplace('C', 36);
		formatMap.emplace('X', 23);
		formatMap.emplace('D', 19);
		formatMap.emplace('U', 35);
		formatMap.emplace('P', 44);
		formatMap.emplace('Q', 40);
		formatMap.emplace('B', 19);
		formatMap.emplace('I', 50);
		formatMap.emplace('N', 20);
	}

	unsigned short getMessageLength(char & messageType) {
		auto iter = formatMap.find(messageType);
		if(iter == formatMap.end()) {
			// std::cout<<messageType<<std::endl;
			// return ;
			return 0;
		}
		return (iter->second);
	}
};

struct ExistTest {
	std::unordered_map<char, int> formatMap;

	ExistTest() {
		formatMap.emplace('S', 1);
		formatMap.emplace('R', 1);
		formatMap.emplace('H', 1);
		formatMap.emplace('Y', 1);
		formatMap.emplace('L', 1);
		formatMap.emplace('V', 1);
		formatMap.emplace('W', 1);
		formatMap.emplace('K', 1);
		formatMap.emplace('A', 1);
		formatMap.emplace('F', 1);
		formatMap.emplace('E', 1);
		formatMap.emplace('C', 1);
		formatMap.emplace('X', 1);
		formatMap.emplace('D', 1);
		formatMap.emplace('U', 1);
		formatMap.emplace('P', 1);
		formatMap.emplace('Q', 1);
		formatMap.emplace('B', 1);
		formatMap.emplace('I', 1);
		formatMap.emplace('N', 1);
	}

	int isExist(char & messageType) {
		auto iter = formatMap.find(messageType);
		if(iter == formatMap.end()) {
			// std::cout<<messageType<<std::endl;
			return 0;
		}
		return (iter->second);
	}
};



unsigned long findNextStartIndex(unsigned long startSearchPosition, unsigned long fileSize, 
							std::ifstream & inputFile,MessageFormat & formatMap,
							ExistTest & existMap, int lookback) {
	unsigned char* lenBuffer = new unsigned char[2 * lookback];
	char* typeBuffer = new char[lookback];
	int* foundBuffer = new int[lookback];
	int found = 0;
	unsigned long currentOffset;
	while(!found && inputFile.tellg() < fileSize) {
		currentOffset = startSearchPosition;
		memset((void*) foundBuffer, 0, 12);
		for(int i = 0; i < lookback; i++) {
			inputFile.read((char *)&(lenBuffer[2*i]), 2);
			inputFile.read((char *)&(typeBuffer[i]), 1);
			if(!(existMap.isExist(typeBuffer[i])&&
				twoByteCharToShort((unsigned char *)(&lenBuffer[2*i]))==formatMap.getMessageLength(typeBuffer[i]))) {
				startSearchPosition++;
				inputFile.seekg(startSearchPosition);
				break;
			}
			if(i == lookback -1) {
				found = 1;
				break;
			}
			currentOffset += twoByteCharToLong((unsigned char *)(&lenBuffer[2*i])) + 2;
			inputFile.seekg(currentOffset);
		}
	}
	delete[] lenBuffer;
	delete[] typeBuffer;
	delete[] foundBuffer;
	return startSearchPosition;
}


int main() {
	MessageFormat formatMap;
	ExistTest existMap;
	std::string inputFileName= "partialData";
	std::string outputFileName = "startIndex.txt";
	unsigned long filesize = findFileSize(inputFileName);

	//input file stream, reading binary input
	std::ifstream inputFile(inputFileName.c_str(), std::ios::in | std::ios::binary);
	std::ofstream outputFile(outputFileName.c_str(), std::ios::out);
	unsigned char buffer[2];

	while(inputFile.read((char*)buffer, 2)) {
		outputFile<<((unsigned long)inputFile.tellg() - 2)<<std::endl;
		inputFile.seekg((unsigned long)inputFile.tellg() + twoByteCharToLong((unsigned char *)buffer));
	}
	outputFile.close();
	inputFile.clear();
	inputFile.seekg(0);
	unsigned long startSearchPosition = 320;
	//end index means the start of a new message
	unsigned long endIndex = findNextStartIndex(startSearchPosition, filesize, inputFile, formatMap, existMap, 3);
	std::cout<<endIndex<<std::endl;
	return 0;
}