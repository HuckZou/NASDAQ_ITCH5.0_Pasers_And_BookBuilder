#include "dataparser.h"


//Here below implements the helper functions for converting
//binary char pointers to different data types
unsigned short twoByteCharToShort(unsigned char* buffer){
	return (unsigned short) ((buffer[0] << 8)| buffer[1]);
}

unsigned int fourByteCharToInt(unsigned char* buffer) {
	unsigned int result = 0;
	for(int i = 0; i < 4; i++) {
		result = (result<<8) + buffer[i];
	}
	return result;
}

unsigned long sixByteCharToLong(unsigned char* timeStamp) {
	unsigned long result = 0;
	for(int i = 0; i < 6; i++) {
		result = (result<<8) + timeStamp[i];
	}
	return result;
}

unsigned long eightByteCharToLong(unsigned char* buffer) {
	unsigned long result = 0;
	for(int i = 0; i < 8; i++) {
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
void stripSpaceForCharArray(unsigned char * charArr, int len) {
	int i = 0;
	while(i < len) {
		if(charArr[i] ==' ')
			break;
		i++;
	}
	std::memset((void*)(charArr+i), 0, len - i);
}

//Here below implements all the parsing methods for different binary messages

//This implements the function for parsing SystemEventMessage binary sequence
void parseSystemEventMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//S
	SystemEventMessage x;
	inputFile.read((char*)x.stockLocate, 2);
	inputFile.read((char*)x.trackingNumber, 2);
	inputFile.read((char*)x.timeStamp, 6);
	inputFile.read((char*)x.eventCode, 1);

	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<
				twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<x.eventCode[0]<<std::endl;
}

//This implements the function for parsing StockDirectory binary sequence
void parseStockDirectory(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//R
	StockDirectory x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.marketCategory), 1);
	inputFile.read((char*)(x.financialStatusIndicator), 1);
	inputFile.read((char*)(x.roundLotSize), 4);
	inputFile.read((char*)(x.roundLotsOnly), 1);
	inputFile.read((char*)(x.issueClassification), 1);
	inputFile.read((char*)(x.issueSubType), 2);
	inputFile.read((char*)(x.authenticity), 1);
	inputFile.read((char*)(x.shortSaleThresholdIndicator), 1);
	inputFile.read((char*)(x.ipoFlag), 1);
	inputFile.read((char*)(x.luldRefPriceTier), 1);
	inputFile.read((char*)(x.etpFlag), 1);
	inputFile.read((char*)(x.etpLeverageFactor), 4);
	inputFile.read((char*)(x.inverseIndicator), 1);
	
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	x.issueSubType[2] = '\0';
	outputFile<<messageType[0]<<","<< twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<x.stock<<","<<x.marketCategory[0]<<","<<
				x.financialStatusIndicator[0]<<","<<fourByteCharToInt((unsigned char *)x.roundLotSize)<<","<<x.roundLotsOnly[0]<<","<<
				x.issueClassification[0]<<","<<x.issueSubType<<","<<x.authenticity[0]<<","<<
				x.shortSaleThresholdIndicator[0]<<","<<x.ipoFlag[0]<<","<<x.luldRefPriceTier[0]<<","<<
				x.etpFlag[0]<<","<<fourByteCharToInt((unsigned char *)x.etpLeverageFactor)<<","<<x.inverseIndicator[0]<<std::endl;
}

//This implements the function for parsing StockTradingAction binary sequence
void parseStockTradingAction(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//H
	StockTradingAction x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.tradingState), 1);
	inputFile.read((char*)(x.reserved), 1);
	inputFile.read((char*)(x.reason), 4);
	// x.reason[4] = '\0';
	stripSpaceForCharArray((unsigned char*)x.reason,5);
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<x.stock<<","<<x.tradingState[0]<<","<<
				x.reserved[0]<<","<<x.reason<<std::endl;
}

//This implements the function for parsing RegShoRestriction binary sequence
void parseRegShoRestriction(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//Y
	RegShoRestriction x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.regSHOAction), 1);
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<x.stock<<","<<
				x.regSHOAction[0]<<std::endl;
}

//This implements the function for parsing MarketParticipantPosition binary sequence
void parseMarketParticipantPosition(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//L
	MarketParticipantPosition x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.MPID), 4);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.primaryMarketMaker), 1);
	inputFile.read((char*)(x.marketMakerMode), 1);
	inputFile.read((char*)(x.marketParticipantState), 1);
	x.MPID[4] = '\0';
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<x.MPID<<","<<x.stock<<","<<
				x.primaryMarketMaker[0]<<","<<x.marketMakerMode[0]<<","<<
				x.marketParticipantState[0]<<std::endl;	
}

//This implements the function for parsing MWCBDeclineLevelMessage binary sequence
void parseMWCBDeclineLevelMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//V
	MWCBDeclineLevelMessage x;	
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.levelOne), 8);
	inputFile.read((char*)(x.levelTwo), 8);
	inputFile.read((char*)(x.levelThree), 8);
	unsigned long levelOne = eightByteCharToLong((unsigned char*)x.levelOne);
	unsigned long levelTwo = eightByteCharToLong((unsigned char*)x.levelTwo);
	unsigned long levelThree = eightByteCharToLong((unsigned char*)x.levelThree);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<
				levelOne/100000000<<"."<<std::setfill('0')<<std::setw(8)<<levelOne%100000000<<
				levelTwo/100000000<<"."<<std::setfill('0')<<std::setw(8)<<levelTwo%100000000<<
				levelThree/100000000<<"."<<std::setfill('0')<<std::setw(8)<<levelThree%100000000<<
				std::endl;
}

//This implements the function for parsing MWCBBreachMessage binary sequence
void parseMWCBBreachMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//W
	MWCBBreachMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.breachedLevel), 1);	
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<x.breachedLevel[0]<<std::endl;
}

//This implements the function for parsing IPOQuotingPeriodUpdate binary sequence
void parseIPOQuotingPeriodUpdate(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType){
	//K
	IPOQuotingPeriodUpdate x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.IPOQuotationReleaseTime), 4);
	inputFile.read((char*)(x.IPOQuotationReleaseQualifier), 1);
	inputFile.read((char*)(x.IPOPrice), 4);
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	unsigned int IPOPrice = fourByteCharToInt((unsigned char*)x.IPOPrice);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<x.stock<<","<<
				fourByteCharToInt((unsigned char*)x.IPOQuotationReleaseTime)<<","<<x.IPOQuotationReleaseQualifier[0]<<","<<
				IPOPrice/10000<<"."<<std::setfill('0')<<std::setw(4)<<IPOPrice%10000<<
				std::endl;
}

//This implements the function for parsing AddOrderMessage binary sequence
void parseAddOrderMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//A
	AddOrderMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.orderReferenceNumber), 8);
	inputFile.read((char*)(x.buySellIndicator), 1);
	inputFile.read((char*)(x.shares), 4);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.price), 4);	
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	unsigned int price = fourByteCharToInt((unsigned char*)x.price);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.orderReferenceNumber)<<","<<
				x.buySellIndicator[0]<<","<<fourByteCharToInt((unsigned char*)x.shares)<<","<<x.stock<<","<<
				price/10000<<"."<<std::setfill('0')<<std::setw(4)<<price%10000<<
				std::endl;
}

//This implements the function for parsing AddOrderMPIDAttributionMessage binary sequence
void parseAddOrderMPIDAttributionMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//F
	AddOrderMPIDAttributionMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.orderReferenceNumber), 8);
	inputFile.read((char*)(x.buySellIndicator), 1);
	inputFile.read((char*)(x.shares), 4);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.price), 4);
	inputFile.read((char*)(x.attribution), 4);
	// x.attribution[4] = '\0';
	stripSpaceForCharArray((unsigned char*)x.attribution,5);
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	unsigned int price = fourByteCharToInt((unsigned char *)x.price);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.orderReferenceNumber)<<","<<
				x.buySellIndicator[0]<<","<<fourByteCharToInt((unsigned char *)x.shares)<<","<<x.stock<<","<<
				price/10000<<"."<<std::setfill('0')<<std::setw(4)<<price%10000<<","<<
				x.attribution<<std::endl;
}


//This implements the function for parsing OrderExecutedMessage binary sequence
void parseOrderExecutedMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType){
	//E
	OrderExecutedMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.orderReferenceNumber), 8);
	inputFile.read((char*)(x.executedShare), 4);
	inputFile.read((char*)(x.matchNumber), 8);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.orderReferenceNumber)<<","<<
				fourByteCharToInt((unsigned char*)x.executedShare)<<","<<eightByteCharToLong((unsigned char*)x.matchNumber)<<std::endl;
}

//This implements the function for parsing OrderExecutedWithPriceMessage binary sequence
void parseOrderExecutedWithPriceMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//C
	OrderExecutedWithPriceMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.orderReferenceNumber), 8);
	inputFile.read((char*)(x.executedShare), 4);
	inputFile.read((char*)(x.matchNumber), 8);
	inputFile.read((char*)(x.printable), 1);
	inputFile.read((char*)(x.executionPrice), 4);
	unsigned int executionPrice = fourByteCharToInt((unsigned char*)x.executionPrice);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.orderReferenceNumber)<<","<<
				fourByteCharToInt((unsigned char*)x.executedShare)<<","<<eightByteCharToLong((unsigned char*)x.matchNumber)<<","<<x.printable[0]<<","<<
				executionPrice/10000<<"."<<std::setfill('0')<<std::setw(4)<<executionPrice%10000<<","<<
				std::endl;
}

//This implements the function for parsing OrderCancelMessage binary sequence
void parseOrderCancelMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType){
	//X
	OrderCancelMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.orderReferenceNumber), 8);
	inputFile.read((char*)(x.canceledShares), 4);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.orderReferenceNumber)<<","<<
				fourByteCharToInt((unsigned char*)x.canceledShares)<<std::endl;
}

//This implements the function for parsing OrderDeleteMessage binary sequence
void parseOrderDeleteMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//D
	OrderDeleteMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.orderReferenceNumber), 8);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.orderReferenceNumber)<<","<<
				std::endl;
}

//This implements the function for parsing OrderReplaceMessage binary sequence
void parseOrderReplaceMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//U
	OrderReplaceMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.orderReferenceNumber), 8);
	inputFile.read((char*)(x.newOrderReferenceNumber), 8);
	inputFile.read((char*)(x.shares), 4);
	inputFile.read((char*)(x.price), 4);	
	unsigned int price = fourByteCharToInt((unsigned char*)x.price);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.orderReferenceNumber)<<","<<
				eightByteCharToLong((unsigned char*)x.newOrderReferenceNumber)<<","<<fourByteCharToInt((unsigned char*)x.shares)<<","<<
				price/10000<<"."<<std::setfill('0')<<std::setw(4)<<price%10000<<","<<
				std::endl;
}

//This implements the function for parsing TradeMessage binary sequence
void parseTradeMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//P
	TradeMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.orderReferenceNumber), 8);
	inputFile.read((char*)(x.buySellIndicator), 1);
	inputFile.read((char*)(x.shares), 4);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.price), 4);
	inputFile.read((char*)(x.matchNumber), 8);
	unsigned int price = fourByteCharToInt((unsigned char*)x.price);
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.orderReferenceNumber)<<","<<
				x.buySellIndicator[0]<<","<<fourByteCharToInt((unsigned char*)x.shares)<<","<<x.stock<<","<<
				price/10000<<"."<<std::setfill('0')<<std::setw(4)<<price%10000<<","<<
				eightByteCharToLong((unsigned char*)x.matchNumber)<<std::endl;
}

//This implements the function for parsing CrossTradeMessage binary sequence
void parseCrossTradeMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//Q
	CrossTradeMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.shares), 8);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.crossPrice), 4);
	inputFile.read((char*)(x.matchNumber), 8);
	inputFile.read((char*)(x.crossType), 1);
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	unsigned int crossPrice = fourByteCharToInt((unsigned char*)x.crossPrice);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.shares)<<","<<x.stock<<","<<
				crossPrice/10000<<"."<<std::setfill('0')<<std::setw(4)<<crossPrice%10000<<","<<
				eightByteCharToLong((unsigned char*)x.matchNumber)<<","<<x.crossType[0]<<std::endl;
}

//This implements the function for parsing BrokenTradeMessage binary sequence
void parseBrokenTradeMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//B
	BrokenTradeMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.matchNumber), 8);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.matchNumber)<<std::endl;
}

//This implements the function for parsing NOIIMessage binary sequence
void parseNOIIMessage(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//I
	NOIIMessage x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.pairedShares), 8);
	inputFile.read((char*)(x.imbalanceShares), 8);
	inputFile.read((char*)(x.imbalanceDirection), 1);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.farPrice), 4);
	inputFile.read((char*)(x.nearPrice), 4);
	inputFile.read((char*)(x.currentReferencePrice), 4);
	inputFile.read((char*)(x.crossType), 1);
	inputFile.read((char*)(x.priceVariationIndicator), 1);
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	unsigned int farPrice = fourByteCharToInt((unsigned char*)x.farPrice);
	unsigned int nearPrice = fourByteCharToInt((unsigned char*)x.nearPrice);
	unsigned int currentReferencePrice = fourByteCharToInt((unsigned char*)x.currentReferencePrice);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<eightByteCharToLong((unsigned char*)x.pairedShares)<<","<<
				eightByteCharToLong((unsigned char*)x.imbalanceShares)<<","<<x.imbalanceDirection[0]<<","<<x.stock<<","<<
				farPrice/10000<<"."<<std::setfill('0')<<std::setw(4)<<farPrice%10000<<","<<
				nearPrice/10000<<"."<<std::setfill('0')<<std::setw(4)<<nearPrice%10000<<","<<
				currentReferencePrice/10000<<"."<<std::setfill('0')<<std::setw(4)<<currentReferencePrice%10000<<","<<
				x.crossType[0]<<","<<x.priceVariationIndicator[0]<<std::endl;
}

//This implements the function for parsing RPII binary sequence
void parseRPII(std::ofstream& outputFile, std::ifstream& inputFile, char* messageType) {
	//N
	RPII x;
	inputFile.read((char*)(x.stockLocate), 2);
	inputFile.read((char*)(x.trackingNumber), 2);
	inputFile.read((char*)(x.timeStamp), 6);
	inputFile.read((char*)(x.stock), 8);
	inputFile.read((char*)(x.interestFlag), 1);
	stripSpaceForCharArray((unsigned char*)x.stock,9);
	outputFile<<messageType[0]<<","<<twoByteCharToShort((unsigned char*)x.stockLocate)<<","<<twoByteCharToShort((unsigned char*)x.trackingNumber)<<","<<
				sixByteCharToLong((unsigned char*)x.timeStamp)<<","<<x.stock<<","<<x.interestFlag[0]<<std::endl;
}