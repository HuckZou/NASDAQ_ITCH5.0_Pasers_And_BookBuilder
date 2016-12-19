#ifndef __HDF5BOOKBUILDERH__
#define __HDF5BOOKBUILDERH__

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <map>
#include <ctime>
#include <chrono>
#include <vector>
#include "H5Cpp.h"

//change this variable to the orderbook depth you want.
const hsize_t ORDERBOOK_DEPTH = 10;


struct compound_t {
	char messageType;
	unsigned short stockLocate;
	unsigned short trackingNumber;
	unsigned long timeStamp;
	unsigned long orderReferenceNumber;
	char buySellIndicator;
	unsigned int shares;
	char stock[9];
	float price;
	char attribution[5];
	unsigned int executedShares;
	unsigned long matchNumber;
	char printable;
	float executionPrice;
	unsigned int canceledShares;
	unsigned long newOrderReferenceNumber;
};

struct order_pool_t
{
	unsigned long orderReferenceNumber;
	float price;
	unsigned int shares;
	char buySellIndicator;
};

struct order_book_t
{
	unsigned long timeStamp;
	char messageType;
	float bidPrice[ORDERBOOK_DEPTH];
	float askPrice[ORDERBOOK_DEPTH];
	unsigned int bidSize[ORDERBOOK_DEPTH];
	unsigned int askSize[ORDERBOOK_DEPTH];
};

struct record_book_t
{
	unsigned long timeStamp;
	char messageType;
	char buySellIndicator;
	float price;
	unsigned int shares;
};

order_pool_t* extractOrderPoolData(compound_t* data);
void writeBuffer_init();

//Order type handlers
void addOrder_AF(order_pool_t* data);
void executedOrder_EC(order_pool_t* data, unsigned int executedShares);
void cancelOrder_X(order_pool_t* data, unsigned int canceledShares);
void deleteOrder_D(order_pool_t* data);
void replaceOrder_U(order_pool_t* data, unsigned long newOrderReferenceNumber);

//HDF5 APIs 
void extendRecordDataSetSpace();
void extendOrderDataSetSpace();
void writeRecordBookBuffer();
void writeOrderBookBuffer();
void updateRecordBookBuffer(compound_t* data);
void updateOrderBookBuffer(compound_t* data);
void writeLastChunkAndCleanUp();

//initialize data structures
void compType_init();
void compTypeR_init();
void compTypeO_init();
void stockLocatorMap_init(std::map<std::string, unsigned short> stockSymbolMap);
void hdf5GroupHierarchy_init();

//main loop for building the orderbook
int hdf5BuildOrderBook(std::map<std::string, unsigned short> stockSymbolMap);

//helper functions
unsigned short twoByteCharToShort(unsigned char* buffer);
void stripSpaceForCharArray(unsigned char * charArr, int len);

#endif