import orderbook as ob 

o = ob.Orderbook('new_orderbook.h5')
o.read_data('SPY','order_book')
o.rollup_orderbook()
o.plot_bid_ask_prices()
o.plot_bid_ask_spread()
o.plot_interpacket_gap()
o.plot_order_traffic(40)
