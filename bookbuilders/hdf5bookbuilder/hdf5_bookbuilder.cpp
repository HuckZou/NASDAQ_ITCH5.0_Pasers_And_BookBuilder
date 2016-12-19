#include "hdf5_bookbuilder.h"

const hsize_t READ_CHUNK_SIZE = 3000;
const hsize_t WRITE_CHUNK_SIZE = 3000;
const hsize_t INIT_DIMS[] = {WRITE_CHUNK_SIZE};
const hsize_t MAX_DIMS[] = {H5S_UNLIMITED};
const hsize_t CHUNK_DIMS[] = {WRITE_CHUNK_SIZE};
const int DATASET_RANK = 1;
hsize_t ORDERBOOK_OFFSET[] = {0};
hsize_t RECORDBOOK_OFFSET[] = {0};
hsize_t INPUTFILE_OFFSET[] = {0};

const std::string BINARY_SOURCE_FILE = "../week1/data";
const std::string HDF5_INPUT_FILE = "../week1/hdf5parser_result/hdf5_data.h5";
const std::string HDF5_OUTPUT_FILE = "hdf5_orderbook.h5";
const std::string MAIN_GROUP_NAME = "/StockSymbols";
const std::string memberStrings[] = {"MessageType", "StockLocate", "TrackingNumber","Timestamp", 
										"OrderReferenceNumber", "BuySellIndicator", "Shares", "Stock", 
										"Price", "Attribution", "ExecutedShares", "MatchNumber", 
										"Printable", "ExecutionPrice", "CanceledShares", 
										"NewOrderReferenceNumber"
									};

H5::CompType COMP_TYPE(sizeof(compound_t)); 
H5::CompType COMP_TYPER(sizeof(record_book_t));
H5::CompType COMP_TYPEO(sizeof(order_book_t));


//This map uses stockLocate as values and stock symbols as keys
std::map<std::string, unsigned short> stockLocatorMap;

//This is a map to hold <bid price, share size> information 
//Use reverse iterator to find N largest keys in the map
std::map<float, unsigned int> bidMap;
//This is a map to hold <ask price, share size> information 
//Use normal iterator to find N smallest keys in the map
std::map<float, unsigned int> askMap;
//This is an unordered map used to track all the order information
//about a given order. <ref#, order_pool_t*>
std::map<unsigned long, order_pool_t*> orderPoolMap;
//This is a pointer that points to an array of order_book_t[WRITE_CHUNK_SIZE]
order_book_t* orderBookWriteBuffer;
//This is a pointer that points to an array of record_book_t[WRITE_CHUNK_SIZE]
record_book_t* recordBookWriteBuffer;
//This is a pointer that points to an array of compound_t[chunk_dim]
compound_t* inputReadBuffer;
//This is a variable to keep track of the current order buffer usage
int orderBufferTracker;
//This is a variable to keep track of the current record buffer usage
int recordBufferTracker;
//This is a variable to keep track of the current input dataset buffer usage
int inputBufferTracker;
//This is a variable to keep track of the number of rows of input dataset that has 
//been read
int numOfChunksRead;

//Following set of pointers are for writing to the order book
H5::DataSet* bookOrderDataSetPointer = NULL;
H5::DataSpace* bookOrderDataSpacePointer= NULL;
H5::DataSpace* bookOrderFileSpacePointer= NULL;

//Following set of pointers are for writing to the record book
H5::DataSet* recordDataSetPointer= NULL;
H5::DataSpace* recordDataSpacePointer= NULL;
H5::DataSpace* recordFileSpacePointer= NULL;

//Following set of pointers are for reading from the input dataset
H5::DataSet* inputDataSetPointer= NULL;
H5::DataSpace* inputDataSpacePointer= NULL;
H5::DataSpace* inputFileSpacePointer= NULL;

//Following set of dimensions are holding the value of the dataset dimension
//the program is currently working with
hsize_t bookOrderDataSetDimension;
hsize_t recordDataSetDimension;
hsize_t inputDataSetDimension;

//Following set of variables tracks the chunk properties of the input file dataset
hsize_t chunk_dim[1];
int chunk_rank;

order_pool_t* extractOrderPoolData(compound_t* data) {
	order_pool_t* result = new order_pool_t;
	result->orderReferenceNumber = data->orderReferenceNumber;
	result->price = data->price;
	result->shares = data->shares;
	result->buySellIndicator = data->buySellIndicator;
	return result;
}

void writeBuffer_init() {
	orderBufferTracker = 0;
	recordBufferTracker = 0;
	orderBookWriteBuffer = new order_book_t[WRITE_CHUNK_SIZE];
	recordBookWriteBuffer = new record_book_t[WRITE_CHUNK_SIZE];
}

//we update our orderbook the same way for A and F
void addOrder_AF(order_pool_t* data) {
	//add this order into our order pool
	orderPoolMap[data->orderReferenceNumber] = data;
	//update bid-ask maps
	if(data->buySellIndicator == 'B') {
		if(bidMap.find(data->price) == bidMap.end()) {
			bidMap[data->price] = data->shares;
		} else {
			bidMap[data->price] += data->shares;
		}
	} else {
		if(askMap.find(data->price) == askMap.end()) {
			askMap[data->price] = data->shares;
		} else {
			askMap[data->price] += data->shares;
		}
	}	
}

//we update our orderbook the same way for E and C
void executedOrder_EC(order_pool_t* data, unsigned int executedShares) {
	//find the corresponding order in the order pool
	order_pool_t* old_order = orderPoolMap[data->orderReferenceNumber];
	if(old_order->buySellIndicator == 'B') {
		if(bidMap[old_order->price] > executedShares)
			bidMap[old_order->price] -= executedShares;
		else {
			bidMap.erase(old_order->price);
		}
	}
	else {
		if(askMap[old_order->price] > executedShares)
			askMap[old_order->price] -= executedShares;
		else {
			askMap.erase(old_order->price);
		}	
	}
	if(old_order->shares > executedShares)
		old_order->shares = old_order->shares - executedShares;
	else {
		delete old_order;
		orderPoolMap.erase(data->orderReferenceNumber);
	}
	delete data;
}

void cancelOrder_X(order_pool_t* data, unsigned int canceledShares) {
	order_pool_t* old_order = orderPoolMap[data->orderReferenceNumber];
	if(old_order->buySellIndicator == 'B') {
		if(bidMap[old_order->price] > canceledShares)
			bidMap[old_order->price] -= canceledShares;
		else {
			bidMap.erase(old_order->price);
		}
	}
	else {
		if(askMap[old_order->price] > canceledShares)
			askMap[old_order->price] -= canceledShares;
		else {
			askMap.erase(old_order->price);
		}	
	}
	if(old_order->shares > canceledShares)
		old_order->shares = old_order->shares - canceledShares;
	else {
		delete old_order;
		orderPoolMap.erase(data->orderReferenceNumber);
	}
	delete data;
}

void deleteOrder_D(order_pool_t* data) {
	order_pool_t* old_order = orderPoolMap[data->orderReferenceNumber];
	if(old_order->buySellIndicator == 'B') {
		if(bidMap[old_order->price] > old_order->shares) {
			bidMap[old_order->price] = bidMap[old_order->price] - old_order->shares;
		} else {
			bidMap.erase(old_order->price);	
		}
	} else {
		if(askMap[old_order->price] > old_order->shares) {
			askMap[old_order->price] = askMap[old_order->price] - old_order->shares;
		} else {
			askMap.erase(old_order->price);	
		}
	}
	delete old_order;
	orderPoolMap.erase(data->orderReferenceNumber);
	delete data;
}

void replaceOrder_U(order_pool_t* data, unsigned long newOrderReferenceNumber) {
	order_pool_t* old_order = orderPoolMap[data->orderReferenceNumber];
	char buySellIndicator = old_order->buySellIndicator;
	unsigned int new_shares = data->shares;
	float new_price = data->price;
	deleteOrder_D(data);
	data = new order_pool_t;
	data->shares = new_shares;
	data->orderReferenceNumber = newOrderReferenceNumber;
	data->price = new_price;
	data->buySellIndicator = buySellIndicator;
	addOrder_AF(data);
}

//Extend the dataspace for the corresponding stock data set of the stock locator
//this function will be called everytime after writing a chunk to the data set
void extendRecordDataSetSpace() {
	try{
		/* Initialize all the HDF5 parameters needed*/
		// Turn off the auto-printing when failure occurs so that we can
		// handle the errors appropriately
		H5::Exception::dontPrint();

		//update new dimension of the dataset
		RECORDBOOK_OFFSET[0] = recordDataSetDimension;
		recordDataSetDimension = recordDataSetDimension + WRITE_CHUNK_SIZE;
		// std::cout<<"The extended size is: "<<dataSetDimensionMap[stockLocator]<<std::endl;
		//set the new size of dataset 
		recordDataSetPointer->extend(&recordDataSetDimension);
		
		//delete the old file data space pointer, we will allocate a new file dataspace pointer
		//to write to new part of the dataset
		delete recordFileSpacePointer;
		//select a hyperslab in extended portion of the data set
		recordFileSpacePointer = new H5::DataSpace(recordDataSetPointer->getSpace());	
		recordFileSpacePointer->selectHyperslab(H5S_SELECT_SET, &WRITE_CHUNK_SIZE, RECORDBOOK_OFFSET);
		//delete the old data space pointer, we will need to allocate a new one for the 
		//new data.
		delete recordDataSpacePointer;
		//allocated new data space and store it in the data space map
		recordDataSpacePointer = new H5::DataSpace(DATASET_RANK, &WRITE_CHUNK_SIZE, NULL);
    }
    // catch failure caused by the H5File operations 
    catch(H5::FileIException error)
    {
		error.printError();
		return;
    }

    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException error)
    {
		error.printError();
		return;
    }

    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException error)
    {
		error.printError();
		return;
    }
}

void extendOrderDataSetSpace() {
	try{
		/* Initialize all the HDF5 parameters needed*/
		// Turn off the auto-printing when failure occurs so that we can
		// handle the errors appropriately
		H5::Exception::dontPrint();
		//update new dimension of the dataset
		ORDERBOOK_OFFSET[0] = bookOrderDataSetDimension;
		bookOrderDataSetDimension = bookOrderDataSetDimension + WRITE_CHUNK_SIZE;
		// std::cout<<"The extended size is: "<<dataSetDimensionMap[stockLocator]<<std::endl;
		//set the new size of dataset 
		bookOrderDataSetPointer->extend(&bookOrderDataSetDimension);
		//delete the old file data space pointer, we will allocate a new file dataspace pointer
		//to write to new part of the dataset
		delete bookOrderFileSpacePointer;
		//select a hyperslab in extended portion of the data set
		bookOrderFileSpacePointer = new H5::DataSpace(bookOrderDataSetPointer->getSpace());	
		bookOrderFileSpacePointer->selectHyperslab(H5S_SELECT_SET, &WRITE_CHUNK_SIZE, ORDERBOOK_OFFSET);
		//delete the old data space pointer, we will need to allocate a new one for the 
		//new data.
		delete bookOrderDataSpacePointer;
		//allocated new data space and store it in the data space map
		bookOrderDataSpacePointer = new H5::DataSpace(DATASET_RANK, &WRITE_CHUNK_SIZE, NULL);
    }
    // catch failure caused by the H5File operations 
    catch(H5::FileIException error)
    {
		error.printError();
		return;
    }
    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException error)
    {
		error.printError();	
		return;
    }
    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException error)
    {
		error.printError();	
		return;
    }
}

void writeRecordBookBuffer() {
	try{
		/* Initialize all the HDF5 parameters needed*/
		// Turn off the auto-printing when failure occurs so that we can
		// handle the errors appropriately
		H5::Exception::dontPrint();
		recordBufferTracker = 0;
		recordDataSetPointer->write(recordBookWriteBuffer, COMP_TYPER, *recordDataSpacePointer, *recordFileSpacePointer);
		extendRecordDataSetSpace();
		std::memset(recordBookWriteBuffer, 0, sizeof(record_book_t)*WRITE_CHUNK_SIZE);
		    // catch failure caused by the H5File operations 
    }
    catch(H5::FileIException error)
    {
		error.printError();
		return;
    }
    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException error)
    {
		error.printError();
		return;
    }
    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException error)
    {
		error.printError();
		return;
    }
}

void writeOrderBookBuffer() {
	try{
		/* Initialize all the HDF5 parameters needed*/
		// Turn off the auto-printing when failure occurs so that we can
		// handle the errors appropriately
		H5::Exception::dontPrint();
		orderBufferTracker = 0;
		bookOrderDataSetPointer->write(orderBookWriteBuffer, COMP_TYPEO, *bookOrderDataSpacePointer, *bookOrderFileSpacePointer);
		extendOrderDataSetSpace();
		std::memset(orderBookWriteBuffer, 0, sizeof(order_book_t)*WRITE_CHUNK_SIZE);
		// catch failure caused by the H5File operations 
    }
    catch(H5::FileIException error)
    {
		error.printError();	
		return;
    }
    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException error)
    {
		error.printError();
		return;
    }
    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException error)
    {
		error.printError();
		return;
    }
}

void updateRecordBookBuffer(compound_t* data) {
	recordBookWriteBuffer[recordBufferTracker].timeStamp = data->timeStamp;
	recordBookWriteBuffer[recordBufferTracker].messageType = data->messageType;
	recordBookWriteBuffer[recordBufferTracker].buySellIndicator = data->buySellIndicator;
	recordBookWriteBuffer[recordBufferTracker].price = data->price;
	recordBookWriteBuffer[recordBufferTracker].shares = data->shares;
	recordBufferTracker++;
	if(recordBufferTracker == WRITE_CHUNK_SIZE)
		writeRecordBookBuffer();
}

void updateOrderBookBuffer(compound_t* data) {
	orderBookWriteBuffer[orderBufferTracker].timeStamp = data->timeStamp;
	orderBookWriteBuffer[orderBufferTracker].messageType = data->messageType;
	int count = 0;
	if(!bidMap.empty()) {
		std::map<float, unsigned int>::reverse_iterator rit;
		for(rit=bidMap.rbegin(); rit!=bidMap.rend(); rit++){
			if(count >= ORDERBOOK_DEPTH)
				break;
			orderBookWriteBuffer[orderBufferTracker].bidPrice[count] = rit->first;
			orderBookWriteBuffer[orderBufferTracker].bidSize[count] = rit->second;
			count++;
		}
	}
	while(count < ORDERBOOK_DEPTH) {
		orderBookWriteBuffer[orderBufferTracker].bidPrice[count] = 0.0;
		orderBookWriteBuffer[orderBufferTracker].bidSize[count] = 0;
		count++;
	}

	//reset the counter and start populating ask side
	count = 0;
	if(!askMap.empty()) {
		std::map<float, unsigned int>::iterator it;
		for(it=askMap.begin(); it!=askMap.end(); it++){
			if(count >= ORDERBOOK_DEPTH)
				break;
			orderBookWriteBuffer[orderBufferTracker].askPrice[count] = it->first;
			orderBookWriteBuffer[orderBufferTracker].askSize[count] = it->second;
			count++;	
		}
	}
	while(count < ORDERBOOK_DEPTH) {
		orderBookWriteBuffer[orderBufferTracker].askPrice[count] = 0.0;
		orderBookWriteBuffer[orderBufferTracker].askSize[count] = 0;
		count++;	
	}

	orderBufferTracker++;
	if (orderBufferTracker == WRITE_CHUNK_SIZE)
		writeOrderBookBuffer();
}

void compType_init() {
	// Create char array type
	hid_t stockType = H5Tcopy(H5T_C_S1);
	hid_t attributionType = H5Tcopy(H5T_C_S1);
	H5Tset_size(stockType, 9);
	H5Tset_size(attributionType, 5);

	COMP_TYPE.insertMember(memberStrings[0], HOFFSET(compound_t, messageType), H5::PredType::NATIVE_CHAR);
	COMP_TYPE.insertMember(memberStrings[1], HOFFSET(compound_t, stockLocate), H5::PredType::NATIVE_USHORT);
	COMP_TYPE.insertMember(memberStrings[2], HOFFSET(compound_t, trackingNumber), H5::PredType::NATIVE_USHORT);
	COMP_TYPE.insertMember(memberStrings[3], HOFFSET(compound_t, timeStamp), H5::PredType::NATIVE_ULONG);
	COMP_TYPE.insertMember(memberStrings[4], HOFFSET(compound_t, orderReferenceNumber), H5::PredType::NATIVE_ULONG);
	COMP_TYPE.insertMember(memberStrings[5], HOFFSET(compound_t, buySellIndicator), H5::PredType::NATIVE_CHAR);
	COMP_TYPE.insertMember(memberStrings[6], HOFFSET(compound_t, shares), H5::PredType::NATIVE_UINT);
	COMP_TYPE.insertMember(memberStrings[7], HOFFSET(compound_t, stock), stockType);
	COMP_TYPE.insertMember(memberStrings[8], HOFFSET(compound_t, price), H5::PredType::NATIVE_FLOAT);
	COMP_TYPE.insertMember(memberStrings[9], HOFFSET(compound_t, attribution), attributionType);
	COMP_TYPE.insertMember(memberStrings[10], HOFFSET(compound_t, executedShares), H5::PredType::NATIVE_UINT);
	COMP_TYPE.insertMember(memberStrings[11], HOFFSET(compound_t, matchNumber), H5::PredType::NATIVE_ULONG);
	COMP_TYPE.insertMember(memberStrings[12], HOFFSET(compound_t, printable), H5::PredType::NATIVE_CHAR);
	COMP_TYPE.insertMember(memberStrings[13], HOFFSET(compound_t, executionPrice), H5::PredType::NATIVE_FLOAT);
	COMP_TYPE.insertMember(memberStrings[14], HOFFSET(compound_t, canceledShares), H5::PredType::NATIVE_UINT);
	COMP_TYPE.insertMember(memberStrings[15], HOFFSET(compound_t, newOrderReferenceNumber), H5::PredType::NATIVE_ULONG);
}

void compTypeR_init() {
	COMP_TYPER.insertMember(memberStrings[3],HOFFSET(record_book_t, timeStamp), H5::PredType::NATIVE_ULONG);
	COMP_TYPER.insertMember(memberStrings[0], HOFFSET(record_book_t, messageType), H5::PredType::NATIVE_CHAR);
	COMP_TYPER.insertMember(memberStrings[5], HOFFSET(record_book_t, buySellIndicator), H5::PredType::NATIVE_CHAR);
	COMP_TYPER.insertMember(memberStrings[8], HOFFSET(record_book_t, price), H5::PredType::NATIVE_FLOAT);
	COMP_TYPER.insertMember(memberStrings[6], HOFFSET(record_book_t, shares), H5::PredType::NATIVE_UINT);
}

void compTypeO_init() {
	COMP_TYPEO.insertMember(memberStrings[3],HOFFSET(order_book_t, timeStamp), H5::PredType::NATIVE_ULONG);
	COMP_TYPEO.insertMember(memberStrings[0], HOFFSET(order_book_t, messageType), H5::PredType::NATIVE_CHAR);
	size_t bidPriceOffset = HOFFSET(order_book_t, bidPrice);
	size_t askPriceOffset = HOFFSET(order_book_t, askPrice);
	size_t bidSizeOffset = HOFFSET(order_book_t, bidSize);
	size_t askSizeOffset = HOFFSET(order_book_t, askSize);
	std::stringstream bidPriceStr;
	std::stringstream askPriceStr;
	std::stringstream bidSizeStr;
	std::stringstream askSizeStr;
	for(size_t i = 0; i < (size_t)ORDERBOOK_DEPTH; i++) {
			bidPriceStr.clear();
			bidPriceStr.str(std::string());
			bidPriceStr << "bidPrice" << i + 1;
			COMP_TYPEO.insertMember(bidPriceStr.str(), bidPriceOffset + 4*i, H5::PredType::NATIVE_FLOAT);
	}

	for(size_t i = 0; i < (size_t)ORDERBOOK_DEPTH; i++) {
			askPriceStr.clear();
			askPriceStr.str(std::string());
			askPriceStr << "askPrice" << i + 1;
			COMP_TYPEO.insertMember(askPriceStr.str(), askPriceOffset + 4*i, H5::PredType::NATIVE_FLOAT);
	}
	for(size_t i = 0; i < (size_t)ORDERBOOK_DEPTH; i++) {
			bidSizeStr.clear();
			bidSizeStr.str(std::string());
			bidSizeStr << "bidSize" << i + 1;
			COMP_TYPEO.insertMember(bidSizeStr.str(), bidSizeOffset + 4*i, H5::PredType::NATIVE_UINT);
	}
	for(size_t i = 0; i < (size_t)ORDERBOOK_DEPTH; i++) {
			askSizeStr.clear();
			askSizeStr.str(std::string());
			askSizeStr << "askSize" << i + 1;
			COMP_TYPEO.insertMember(askSizeStr.str(), askSizeOffset + 4*i, H5::PredType::NATIVE_UINT);
	}
}


unsigned short twoByteCharToShort(unsigned char* buffer) {
	unsigned short result = 0;
	result += buffer[0];
	result = (result << 8) + buffer[1];
	return result;
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

//This function initialize the stockLocatorMap by reading
//all "R" message on the given day. It stops when it reads
//the first "L" message of the day, which is issued after 
//all "R" messages are out. Note: each "R" message contains
//a stock that is trading on NASDAQ on the given day
void stockLocatorMap_init(std::map<std::string, unsigned short> stockSymbolMap) {
	//input file stream, reading binary input
	std::ifstream inputFile(BINARY_SOURCE_FILE.c_str(), std::ios::in | 
							std::ios::binary);
	unsigned char buffer[2];
	char typeBuffer[1];
	while(inputFile.read((char*)buffer, 2)) {
		inputFile.read((char*)typeBuffer, 1);
		if(typeBuffer[0] == 'L')
			break;
		//If the message type is 'R', we add the (stockLocate, symbol)
		//pair to our stockLocatorMap
		if(typeBuffer[0] == 'R') {
			unsigned char stockSymbol[9];
			unsigned char stockLocator[2];
			inputFile.read((char*)stockLocator, 2);
			inputFile.seekg(8, std::ios::cur);
			inputFile.read((char*)stockSymbol, 8);
			unsigned short stockLocator_s= twoByteCharToShort((unsigned char*)stockLocator);
			stripSpaceForCharArray((unsigned char *)stockSymbol, 9);
			std::stringstream ss;
			ss<<stockSymbol;
			if(stockSymbolMap.find(ss.str()) != stockSymbolMap.end())
				stockLocatorMap[ss.str()] = stockLocator_s;
			inputFile.seekg(20, std::ios::cur);
		}
		//If else, then we skip this message 
		else {
			inputFile.seekg(twoByteCharToShort((unsigned char*)buffer) - 1, std::ios::cur);
		}
	}
	inputFile.close();
}

//This method initialize the heirarchy of the HDF5_OUTPUT_FILE
//under group "/stocks/", for each stock that exists in the stockLocatorMap
//we create a new group and name it with the corresponding stock symbol name
void hdf5GroupHierarchy_init() {
	try {
		// Turn off the auto-printing when failure occurs so that we can
		// handle the errors appropriately
		H5::Exception::dontPrint();

		H5::H5File file(HDF5_OUTPUT_FILE, H5F_ACC_TRUNC);
		H5::Group mainGroup(file.createGroup(MAIN_GROUP_NAME));
		for(auto it = stockLocatorMap.cbegin(); it != stockLocatorMap.cend(); it++) {
			H5::Group stockGroup(mainGroup.createGroup(it->first));
		}
	}
	catch (H5::FileIException error) {
		error.printError();
	}
	catch (H5::GroupIException error) {
		error.printError();
    }
}

void writeLastChunkAndCleanUp() {
	try{
		/* Initialize all the HDF5 parameters needed*/
		// Turn off the auto-printing when failure occurs so that we can
		// handle the errors appropriately
		H5::Exception::dontPrint();
		//write all the remaining data in the write buffers into dataset
		bookOrderDataSetPointer->write(orderBookWriteBuffer, COMP_TYPEO, *bookOrderDataSpacePointer, *bookOrderFileSpacePointer);
		recordDataSetPointer->write(recordBookWriteBuffer, COMP_TYPER, *recordDataSpacePointer, *recordFileSpacePointer);
		delete bookOrderDataSpacePointer;
		delete recordDataSpacePointer;
		delete bookOrderDataSetPointer;
		delete recordDataSetPointer;
		delete bookOrderFileSpacePointer;
		delete recordFileSpacePointer;
		std::memset(orderBookWriteBuffer, 0, sizeof(order_book_t)*WRITE_CHUNK_SIZE);
		std::memset(recordBookWriteBuffer, 0, sizeof(record_book_t)*WRITE_CHUNK_SIZE);
		delete inputDataSetPointer;
		delete inputFileSpacePointer;
		delete inputDataSpacePointer;
		delete inputReadBuffer;
		//clear all maps and free memory if necessary
		bidMap.clear();
		askMap.clear();
		for(auto it = orderPoolMap.begin(); it != orderPoolMap.end(); it++) {
			delete it->second;
		}
		orderPoolMap.clear();
	}
    catch(H5::FileIException error)
    {
		error.printError();
		return;
    }
    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException error)
    {
		error.printError();	
		return;
    }
    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException error)
    {
		error.printError();
		return;
    }
}


int hdf5BuildOrderBook(std::map<std::string, unsigned short> stockSymbolMap) {
	try{
		/* Initialize all the HDF5 parameters needed*/
		// Turn off the auto-printing when failure occurs so that we can
		// handle the errors appropriately
		H5::Exception::dontPrint();
		// Create a new file using the default property lists. 
		H5::H5File outfile(HDF5_OUTPUT_FILE, H5F_ACC_RDWR);		
		// Create a file pointer for reading the parsed data file.
		H5::H5File infile(HDF5_INPUT_FILE, H5F_ACC_RDONLY);
		// Modify dataset creation property to enable chunking
		H5::DSetCreatPropList prop;
		prop.setChunk(DATASET_RANK, CHUNK_DIMS);
		bookOrderDataSetDimension = WRITE_CHUNK_SIZE;
		recordDataSetDimension = WRITE_CHUNK_SIZE;
		orderBookWriteBuffer = new order_book_t[WRITE_CHUNK_SIZE];
		recordBookWriteBuffer = new record_book_t[WRITE_CHUNK_SIZE];
		std::memset(orderBookWriteBuffer, 0, sizeof(order_book_t)*WRITE_CHUNK_SIZE);
		std::memset(recordBookWriteBuffer, 0, sizeof(record_book_t)*WRITE_CHUNK_SIZE);
		orderBufferTracker = 0;
		recordBufferTracker = 0;

		// Initialize datasets for each stock and write to them.
		for(auto it = stockSymbolMap.cbegin(); it != stockSymbolMap.cend(); it++) {
			// if(it->second != "SPY") continue;
			if(stockLocatorMap.find(it->first) == stockLocatorMap.end()) {
				std::cout<<"Stock symbol: "<<it->first<<" is omitted, because it is not found in the dataset."<<std::endl;
				continue;
			}
			//Get the corresponding group for this stock
			H5::Group group(outfile.openGroup(MAIN_GROUP_NAME+"/"+it->first));

			//Allocate space for this dataset.
			bookOrderDataSpacePointer = new H5::DataSpace(DATASET_RANK, INIT_DIMS, MAX_DIMS);
			recordDataSpacePointer = new H5::DataSpace(DATASET_RANK, INIT_DIMS, MAX_DIMS);
			//Craete a pointer to the dataset, if the dataset is full, we will need to extend the dataset
			bookOrderDataSetPointer = new H5::DataSet(group.createDataSet("order_book",COMP_TYPEO, *bookOrderDataSpacePointer, prop));
			recordDataSetPointer = new H5::DataSet(group.createDataSet("record_book",COMP_TYPER, *recordDataSpacePointer, prop));
			//Create file pointers for both of the datasets
			bookOrderFileSpacePointer = new H5::DataSpace(bookOrderDataSetPointer->getSpace());
			recordFileSpacePointer = new H5::DataSpace(recordDataSetPointer->getSpace());

			//while not the end of the hdf5 file (to do this, we need to find the dimension of the 
			//dataset and then keep track of where we are in the dataset)
			//Open the input file group for this stock, open a pointer to its dataset
			H5::Group inputFileGroup(infile.openGroup(MAIN_GROUP_NAME+"/"+it->first));
			inputDataSetPointer = new H5::DataSet(inputFileGroup.openDataSet(std::to_string(stockLocatorMap[it->first])));
			inputFileSpacePointer = new H5::DataSpace(inputDataSetPointer->getSpace());
			inputFileSpacePointer->getSimpleExtentDims(&inputDataSetDimension);
			//Get creation properties list for the dataset
			H5::DSetCreatPropList inputFileCparms = inputDataSetPointer->getCreatePlist();
			//Check if the dataset is chunked

			if(H5D_CHUNKED != inputFileCparms.getLayout()) {
				std::cout<<"The input dataset should be chunked."<<
				"Something is wrong here at: line"<<__LINE__<<std::endl;
				return -1;
			}
			chunk_rank = inputFileCparms.getChunk(1, chunk_dim);
			inputReadBuffer = new compound_t[chunk_dim[0]];
			INPUTFILE_OFFSET[0] = 0;
			numOfChunksRead = 0;
			//for each chunk of data that is read
			while(numOfChunksRead*chunk_dim[0] < inputDataSetDimension) {
				inputDataSpacePointer = new H5::DataSpace(chunk_rank, chunk_dim);
				inputFileSpacePointer->selectHyperslab(H5S_SELECT_SET, chunk_dim, INPUTFILE_OFFSET);
				inputDataSetPointer->read(inputReadBuffer, COMP_TYPE, *inputDataSpacePointer, *inputFileSpacePointer);
				inputBufferTracker = 0;
				//for each message in the chunk of data
				while(inputBufferTracker < chunk_dim[0]) {
					compound_t data = inputReadBuffer[inputBufferTracker];
					inputBufferTracker++;
					char messageType = data.messageType;
					//This memory is freed in the order functions
					order_pool_t* order_data = new order_pool_t;
					order_data->orderReferenceNumber = data.orderReferenceNumber;
					order_data->price = data.price;
					order_data->shares = data.shares;
					order_data->buySellIndicator = data.buySellIndicator;
					if(messageType == 'A' || messageType == 'F') {
						addOrder_AF(order_data);
					} else if (messageType == 'E' || messageType == 'C'){
						executedOrder_EC(order_data, data.executedShares);
					} else if (messageType == 'X') {
						cancelOrder_X(order_data, data.canceledShares);
					} else if (messageType == 'D') {
						deleteOrder_D(order_data);
					} else if (messageType == 'U') {
						replaceOrder_U(order_data, data.newOrderReferenceNumber);
					} else if (messageType == 0) {
						/*	if the message is all zeros, it means that it is the end of message feeds
						  	write all the remaining buffers into the hdf5 file
							end of the dataset, break the loop and write whatever in the buffer 
							into the dataset. */
						break;
					} else {
						//do nothing and continue						
					}
					updateOrderBookBuffer(&data);
					updateRecordBookBuffer(&data);
				}
				std::memset(inputReadBuffer, 0, sizeof(compound_t)*chunk_dim[0]);
				INPUTFILE_OFFSET[0] = INPUTFILE_OFFSET[0] + chunk_dim[0];
				numOfChunksRead++;
			}
			//memset all memory allocated to zero
			//free memory if needed.
			writeLastChunkAndCleanUp();
		}
		delete orderBookWriteBuffer;
		delete recordBookWriteBuffer;
		infile.close();
		outfile.close();
		prop.close();
	}
	// catch failure caused by the H5File operations
    catch(H5::FileIException error)
    {
		error.printError();
		return -1;
    }

    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException error)
    {
		error.printError();
		return -1;
    }

    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException error)
    {
		error.printError();
		return -1;
    }
    return 0;  // successfully terminated
}
