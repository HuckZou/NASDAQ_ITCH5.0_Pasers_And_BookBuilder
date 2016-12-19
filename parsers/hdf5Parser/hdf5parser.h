#ifndef __HDF5PARSERH__
#define __HDF5PARSERH__
/* 
Add Order "A"
MessageType, StockLocate, TrackingNumber, Timestamp, 
OrderReferenceNumber, BuySellIndicator, Shares, Stock, 
Price
{0, 1, 2, 3, 4, 5, 6, 7, 8]

Add Order "F"
MessageType, StockLocate, TrackingNumber, Timestamp, 
OrderReferenceNumber, BuySellIndicator, Shares, Stock, 
Price, Attribution
{0, 1, 2, 3, 4, 5, 6, 7, 8, 9]

Modify Order "E"
MessageType, StockLocate, TrackingNumber, Timestamp, 
OrderReferenceNumber, ExecutedShares, MatchNumber
{0, 1, 2, 3, 4, 10, 11]

Executed Order "C"
MessageType, StockLocate, TrackingNumber, Timestamp, 
OrderReferenceNumber, ExecutedShares, MatchNumber, 
Printable, ExecutionPrice
{0, 1, 2, 3, 4, 10, 11, 12, 13]

Cancel Order "X"
MessageType, StockLocate, TrackingNumber, Timestamp, 
OrderReferenceNumber, CanceledShares
{0, 1, 2, 3, 4, 14]

Delete Order "D"
MessageType, StockLocate, TrackingNumber, Timestamp, 
OrderReferenceNumber
{0, 1, 2, 3, 4]

Replace Order "U"
MessageType, StockLocate, TrackingNumber, Timestamp, 
OriginalOrderReferenceNumber(use OrderReferenceNumber spot), 
NewOrderReferenceNumber, Shares, Price
{0, 1, 2, 3, 4, 5, 15, 6, 8]

Total
MessageType 0, StockLocate 1, TrackingNumber 2, Timestamp 3, 
OrderReferenceNumber 4, BuySellIndicator 5, Shares 6, Stock 7, 
Price 8, Attribution 9, ExecutedShares 10, MatchNumber 11, 
Printable 12, ExecutionPrice 13, CanceledShares 14, 
NewOrderReferenceNumber 15
*/

//h5c++ -std=c++11 -lhdf5 -lhdf5_cpp hdf5Builder.cpp -o builder
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <ctime>
#include <chrono>
#include "H5Cpp.h"

struct compound_t {
// 	MessageType 0, StockLocate 1, TrackingNumber 2, Timestamp 3, 
// OrderReferenceNumber 4, BuySellIndicator 5, Shares 6, Stock 7, 
// Price 8, Attribution 9, ExecutedShares 10, MatchNumber 11, 
// Printable 12, ExecutionPrice 13, CanceledShares 14, 
// NewOrderReferenceNumber 15
	char MessageType;
	unsigned short StockLocate;
	unsigned short TrackingNumber;
	unsigned long Timestamp;
	unsigned long OrderReferenceNumber;
	char BuySellIndicator;
	unsigned int Shares;
	char Stock[9];
	float Price;
	char Attribution[5];
	unsigned int ExecutedShares;
	unsigned long MatchNumber;
	char Printable;
	float ExecutionPrice;
	unsigned int CanceledShares;
	unsigned long NewOrderReferenceNumber;
};

/*
Helper function declarations
*/
unsigned short twoByteCharToShort(unsigned char* buffer);
unsigned int fourByteCharToInt(unsigned char* buffer);
unsigned long sixByteCharToLong(unsigned char* buffer);
unsigned long eightByteCharToLong(unsigned char* buffer);
void stripSpaceForCharArray(unsigned char * charArr, int len);

//Helper function for extending HDF5 dataset space
void extendDataSetSpace(unsigned short stockLocator);
//Helper function for writing to HDF5 dataset or just increasing the counter
void increaseCounterOrWrite(unsigned short stockLocator);
//Helper function for writing the last chunk and clean up memories
void writeLastChunkAndCleanUp();

//Helper functions for initializing data structures
void stockLocatorMap_init();
void hdf5GroupHierarchy_init();
void compType_init();
int hdf5PopulateData();
void testStockLocatorMap();
/*
End of helper function declarations
*/

void parseAddOrderMessage(std::ifstream& inputFile, char* messageType);
void parseAddOrderMPIDAttributionMessage(std::ifstream& inputFile, char* messageType);
void parseOrderExecutedMessage(std::ifstream& inputFile, char* messageType);
void parseOrderExecutedWithPriceMessage(std::ifstream& inputFile, char* messageType);
void parseOrderCancelMessage(std::ifstream& inputFile, char* messageType);
void parseOrderDeleteMessage(std::ifstream& inputFile, char* messageType);
void parseOrderReplaceMessage(std::ifstream& inputFile, char* messageType);
void parseTradeMessage(std::ifstream& inputFile, char* messageType);


struct AddOrderMessage {
	//A
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char orderReferenceNumber[8];
	char buySellIndicator[1];
	char shares[4];
	char stock[9];
	unsigned char price[4];
};	//36

struct AddOrderMPIDAttributionMessage {
	//F
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char orderReferenceNumber[8];
	char buySellIndicator[1];
	unsigned char shares[4];
	char stock[9];
	unsigned char price[4];
	char attribution[5];
};	//40

struct OrderExecutedMessage {
	//E
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char orderReferenceNumber[8];	
	unsigned char executedShare[4];
	unsigned char matchNumber[8];
};	//31

struct OrderExecutedWithPriceMessage{
	//C
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char orderReferenceNumber[8];	
	unsigned char executedShare[4];
	unsigned char matchNumber[8];
	char printable[1];
	unsigned char executionPrice[4];
};  //36

struct OrderCancelMessage {
	//X
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char orderReferenceNumber[8];	
	unsigned char canceledShares[4];
};	//23

struct OrderDeleteMessage {
	//D
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char orderReferenceNumber[8];		
};	//19

struct OrderReplaceMessage {
	//U
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char orderReferenceNumber[8];	
	unsigned char newOrderReferenceNumber[8];
	unsigned char shares[4];
	unsigned char price[4];
};	//35

struct TradeMessage {
	//P
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char orderReferenceNumber[8];	
	char buySellIndicator[1];
	unsigned char shares[4];
	char stock[9];
	unsigned char price[4];
	unsigned char matchNumber[8];
};	//44

struct FMap {
	typedef void (*ScriptFunction)(std::ifstream&, char*);
	typedef std::unordered_map<char, ScriptFunction> functionMap;

	functionMap decoderMap;

	FMap() {
		decoderMap.emplace('A', &parseAddOrderMessage);
		decoderMap.emplace('F', &parseAddOrderMPIDAttributionMessage);
		decoderMap.emplace('E', &parseOrderExecutedMessage);
		decoderMap.emplace('C', &parseOrderExecutedWithPriceMessage);
		decoderMap.emplace('X', &parseOrderCancelMessage);
		decoderMap.emplace('D', &parseOrderDeleteMessage);
		decoderMap.emplace('U', &parseOrderReplaceMessage);
		decoderMap.emplace('P', &parseTradeMessage);
	}

	void call(const char & pFunction, std::ifstream& inputFile, char* messageType, unsigned char* buffer)
	{
		auto iter = decoderMap.find(pFunction);
		if(iter == decoderMap.end())
		{
			inputFile.seekg(twoByteCharToShort(buffer)-1, std::ios::cur);
			return;
		}
		(*iter->second)(inputFile, messageType);
	}

};

#endif