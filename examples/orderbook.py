import pandas as pd 
import numpy as np 
import matplotlib.pyplot as plt 
from datetime import timedelta

class Orderbook():
	'''
	This is an Orderbook object that allows
	users to read from a hdf5 orderbook file
	and read the data into pandas to do data analytics
	'''
	def __init__(self, file_path):
		'''
		Constructor
		'''
		self.file_path = file_path
		self.current_df = None
	
	#This method reads data from 
	#Stock_symbols are stick ticks shown on the exchange
	#AAPL for Apple .Inc, SPY for SP500 ETF
	#Dataset_type has two types: ['order_book', 'record_book']
	#'order_book' stores the entire raw orderbook history for the entire trading day
	#'record_book' stores all the trade messages for the order_book
	def read_data(self, stock_symbol, dataset_type):
		#build the key based on stock_symbol
		key = 'StockSymbols/'+stock_symbol+'/'+dataset_type
		#read from the hdf5 file
		self.current_df = pd.read_hdf(self.file_path, key = key)

	#Since it is possible for our raw orderbook to have multiple
	#execution messages for a given timestamp, we need to roll up our 
	#raw order book
	def rollup_orderbook(self):
		#We keep the last row among the duplicate rows.
		self.current_df.drop_duplicates(subset='Timestamp', keep='last', inplace=True)
		#Redefine the index after drop rows
		self.current_df.index = range(len(self.current_df.index))
		#Check if the last row is zero, if it is, then we drop it
		if(self.current_df['Timestamp'][len(self.current_df)-1] == 0):
			self.current_df = self.current_df[:-1]

	#data visualization components
	def plot_bid_ask_prices(self):
		#we only plot from 9:30 am to 4:00 pm
		temp_df = self.current_df[(self.current_df['Timestamp'] >=34200000000000) & (self.current_df['Timestamp']<=57600000000000)]
		
		#get best bid price 
		bid = temp_df['bidPrice1']
		ask = temp_df['askPrice1']
		plt.plot(bid, label='best bid price')
		plt.plot(ask, label='best ask price')
		plt.xlabel('Time (in nanoseconds)')
		plt.ylabel('Price')
		plt.title('Best Bid-Ask Prices')
		plt.show()

	def plot_bid_ask_spread(self):
		temp_df = self.current_df[(self.current_df['Timestamp'] >=34200000000000) & (self.current_df['Timestamp']<=57600000000000)]		

		#get best bid price 
		bid = temp_df['bidPrice1']
		ask = temp_df['askPrice1']
		plt.plot(ask-bid)
		plt.xlabel('Time (in nanoseconds)')
		plt.ylabel('Spread')
		plt.title('Best Bid-Ask Spread')
		plt.show()

	def plot_interpacket_gap(self):
		temp_df = self.current_df[(self.current_df['Timestamp'] >=34200000000000) & (self.current_df['Timestamp']<=57600000000000)]
		t = temp_df['Timestamp']
		diff_df = t.diff()
		temp = plt.hist(diff_df[diff_df<2000], 200)
		plt.xlabel('Time Gap (in nanoseconds)')
		plt.ylabel('Number of Orders')
		plt.title('Interarrival time distribution')
		plt.show()

	#helper function for converting nanoseconds to time
	def time_convert(self, nanosecond_time):
		return (str(timedelta(seconds=nanosecond_time / 1e+9)))

	def get_daytime(self, time_df):
		return time_df.apply(self.time_convert)
	
	def plot_order_traffic(self, bins):
		temp_df = self.current_df[(self.current_df['Timestamp'] >=34200000000000) & (self.current_df['Timestamp']<=57600000000000)]
		t = temp_df['Timestamp']
		intervals = np.linspace(34200000000000, 57600000000000, bins+1)
		frequency = np.zeros(bins)
		count = 0
		while count < bins:
			frequency[count] = len(t[(t>=intervals[count])&(t<=intervals[count+1])])
			count = count + 1
		df_intervals = pd.DataFrame(intervals[:-1])
		dates = self.get_daytime(df_intervals[0])
		y_pos = np.arange(len(dates))
		plt.bar(y_pos, frequency, align='center', alpha=0.75)
		plt.xticks(y_pos, dates, rotation=45)		
		plt.xlabel('Time of the day')
		plt.ylabel('Number of orders')
		plt.title('matching engine traffic')
		plt.show()

	def get_training_data(self):
		temp_df = self.current_df[(self.current_df['Timestamp'] >=34200000000000) & (self.current_df['Timestamp']<=57600000000000)]
		exe_df = temp_df[temp_df['MessageType']==69]
		exe_df['TimeDiff']=exe_df['Timestamp'].diff()
		exe_df['Imbalance1'] = exe_df['bidSize1']/exe_df['askSize1']
		exe_df['Imbalance2'] = (exe_df['bidSize1'] + exe_df['bidSize2'])/(exe_df['askSize1']+exe_df['askSize2'])
		exe_df['Imbalance3'] = (exe_df['bidSize1'] + exe_df['bidSize2'] + exe_df['bidSize3'])/(exe_df['askSize1']+exe_df['askSize2']+exe_df['askSize3'])
		exe_df['Midprice'] = ((exe_df['askPrice1'] + exe_df['bidPrice1'])/2.0)
		exe_df = exe_df[exe_df['Timestamp'].diff()>300000]
		result = exe_df[['TimeDiff','Imbalance1','Imbalance2','Imbalance3','Midprice']]
		result.to_csv('traning_data.csv')