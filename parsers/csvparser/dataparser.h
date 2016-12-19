#ifndef __DATAPARSERH__
#define __DATAPARSERH__

#include <sys/stat.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <map>
#include <unordered_map>
#include <ctime>


//This helper function returns the size of the data file in bytes
unsigned long findFileSize(std::string filename);
//This helper function converts char pointers to unsigned short
unsigned short twoByteCharToShort(unsigned char* buffer);
//This helper function converts char pointers to unsigned int
unsigned int fourByteCharToInt(unsigned char* buffer);
//This helper function converts six-byte char pointers to unsigned long
unsigned long sixByteCharToLong(unsigned char* timeStamp);
//This helper function converts eight-byte char pointers to unsigned long
unsigned long eightByteCharToLong(unsigned char* buffer);
//This helper function strips the spaces in a char pointer
void stripSpaceForCharArray(unsigned char * charArr, int len);

//Parsers for each of the trade messages
void parseSystemEventMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseStockDirectory(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseStockTradingAction(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseRegShoRestriction(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseMarketParticipantPosition(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseMWCBDeclineLevelMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseMWCBBreachMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseIPOQuotingPeriodUpdate(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseAddOrderMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseAddOrderMPIDAttributionMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseOrderExecutedMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseOrderExecutedWithPriceMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseOrderCancelMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseOrderDeleteMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseOrderReplaceMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseTradeMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseCrossTradeMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseBrokenTradeMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseNOIIMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);
void parseRPII(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType);

//data format for each kind of trade messages
struct SystemEventMessage {
	//S
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	char eventCode[1];
};

struct StockDirectory {
	//R
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	char stock[9];
	char marketCategory[1];
	char financialStatusIndicator[1];
	unsigned char roundLotSize[4]; //int
	char roundLotsOnly[1];
	char issueClassification[1];
	char issueSubType[3];
	char authenticity[1];
	char shortSaleThresholdIndicator[1];
	char ipoFlag[1];
	char luldRefPriceTier[1];
	char etpFlag[1];
	unsigned char etpLeverageFactor[4]; //int
	char inverseIndicator[1];
}; //39

struct StockTradingAction {
	//H
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	char stock[9];
	char tradingState[1];
	char reserved[1];
	char reason[5];
};

struct RegShoRestriction {
	//Y
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	char stock[9];
	char regSHOAction[1];
}; //20

struct MarketParticipantPosition {
	//L
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	char MPID[5];
	char stock[9];
	char primaryMarketMaker[1];
	char marketMakerMode[1];
	char marketParticipantState[1];
};	//26

struct MWCBDeclineLevelMessage {
	//V
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char levelOne[8];
	unsigned char levelTwo[8];
	unsigned char levelThree[8];
};	//35

struct MWCBBreachMessage {
	//W
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	char breachedLevel[1];
};	//12

struct IPOQuotingPeriodUpdate {
	//K
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	char stock[9];
	unsigned char IPOQuotationReleaseTime[4];
	char IPOQuotationReleaseQualifier[1];
	unsigned char IPOPrice[4];
};	//28

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

struct CrossTradeMessage {
	//Q
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char shares[8];
	char stock[9];
	unsigned char crossPrice[4];
	unsigned char matchNumber[8];
	char crossType[1];
};	//40

struct BrokenTradeMessage {
	//B
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char matchNumber[8];
};	//19

struct NOIIMessage {
	//I
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	unsigned char pairedShares[8];
	unsigned char imbalanceShares[8];
	char imbalanceDirection[1];
	char stock[9];
	unsigned char farPrice[4];
	unsigned char nearPrice[4];
	unsigned char currentReferencePrice[4];
	char crossType[1];
	char priceVariationIndicator[1];
};	//50

struct RPII {
	//N
	unsigned char stockLocate[2];
	unsigned char trackingNumber[2];
	unsigned char timeStamp[6];
	char stock[9];
	char interestFlag[1];
}; 	//20

//here below implements a map that maps different trade message identifiers to
//its corresponding message parsers.
struct FMap {
	typedef void (*ScriptFunction)(std::ofstream&, std::ifstream&, char*);
	typedef std::unordered_map<char, ScriptFunction> functionMap;

	functionMap decoderMap;

	FMap() {
		decoderMap.emplace('S', &parseSystemEventMessage);
		decoderMap.emplace('R', &parseStockDirectory);
		decoderMap.emplace('H', &parseStockTradingAction);
		decoderMap.emplace('Y', &parseRegShoRestriction);
		decoderMap.emplace('L', &parseMarketParticipantPosition);
		decoderMap.emplace('V', &parseMWCBDeclineLevelMessage);
		decoderMap.emplace('W', &parseMWCBBreachMessage);
		decoderMap.emplace('K', &parseIPOQuotingPeriodUpdate);
		decoderMap.emplace('A', &parseAddOrderMessage);
		decoderMap.emplace('F', &parseAddOrderMPIDAttributionMessage);
		decoderMap.emplace('E', &parseOrderExecutedMessage);
		decoderMap.emplace('C', &parseOrderExecutedWithPriceMessage);
		decoderMap.emplace('X', &parseOrderCancelMessage);
		decoderMap.emplace('D', &parseOrderDeleteMessage);
		decoderMap.emplace('U', &parseOrderReplaceMessage);
		decoderMap.emplace('P', &parseTradeMessage);
		decoderMap.emplace('Q', &parseCrossTradeMessage);
		decoderMap.emplace('B', &parseBrokenTradeMessage);
		decoderMap.emplace('I', &parseNOIIMessage);
		decoderMap.emplace('N', &parseRPII);
	}

	void call(const char & pFunction, std::ofstream& outputFile, std::ifstream& inputFile, char* messageType, unsigned char* buffer)
	{
		auto iter = decoderMap.find(pFunction);
		if(iter == decoderMap.end())
		{
			std::cout<<pFunction<<std::endl;
			// std::cout<<twoByteCharToShort((unsigned char*)buffer) <<std::endl;
			// return;
		}
		// std::cout<<twoByteCharToShort((unsigned char*)buffer) <<std::endl;
		(*iter->second)(outputFile, inputFile, messageType);
	}
};

#endif