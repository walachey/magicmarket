
#property copyright "Copyright © 2014 David Dormagen"
#property link      "http://www.mql4zmq.org"

// Runtime options to specify.
extern int Bridge_Port = 5005;
extern string Bridge_Address = "127.0.0.1";

extern string leading_currency = "EURUSD";

string socketReceive()
{
	/*if (bridgeSocket == -1) return "";
	while (True)
	{
   	int bufLen = 1; 
   	string message;
   	int errorValue = sock_receive(bridgeSocket, bufLen, message);
   	if (errorValue == 0) return "";
   	
   	if (errorValue < 0)
   	{
   	   socketCleanup();
   	   return "";
   	}
   	
   	// check if messsage is complete (null-terminated)
   	int terminationPosition = -1;
   	for (int i = 0; i < bufLen; ++i)
   	{
   		if (StringGetChar(message, i) != '\0') continue;
   		terminationPosition = i;
   		break;
   	}
   	
   	int oldLength = StringLen(socketReceiveBuffer);
      //Print ("STAT: " + socketReceiveBuffer + " += " + message);
   	StringAdd(socketReceiveBuffer, message);
   	
   	if (terminationPosition != -1)
   	{
   		int position = oldLength + terminationPosition;
   		message = StringSubstr(socketReceiveBuffer, 0, position);
   		socketReceiveBuffer = StringSubstr(socketReceiveBuffer, position + 1, StringLen(socketReceiveBuffer)); 
   		return message;
   	}
	}*/
	return "";
}


// helper
char A1[2048];
char A2[2048];
void S2A(string s, uchar &buffer[])
{
	StringToCharArray(s, buffer);
	//return buffer;
}

string Name()
{
	string acc_name = AccountName();
	acc_name = StringSubstr(acc_name, 0, StringFind(acc_name, " "));
	if (acc_name == 0 || acc_name == "") acc_name = "ERROR";
	return acc_name;
}

// Include the libzmq.dll abstration wrapper.
#include <MetaTraderBridge.mqh>

//+------------------------------------------------------------------+
//| variable definitions                                             |
//+------------------------------------------------------------------+
int server_link;
int isConnected() { return (server_link != 0); }
//+------------------------------------------------------------------+
//| expert initialization function                                   |
//+------------------------------------------------------------------+
void connectBridge()
{
	S2A(Bridge_Address, A1);
	S2A(Symbol(), A2);
	
	server_link = mm_init(A1, Bridge_Port, A2);
	if (server_link == 0)
	{
		Print ("Could not connect to bridge.");
		return;
	}
	if (server_link == 1)
	{
		Print ("Could not init Winsock.");
		server_link = 0;
		return;
	}
	if (server_link == 2)
	{
		Print ("Could not init socket.");
		server_link = 0;
		return;
	}
	if (server_link == 3)
	{
		Print ("Could not set nonblocking option.");
		server_link = 0;
		return;
	}
	PrintFormat("Server Link: %d", server_link);
	mm_sendUp(server_link, TimeCurrent());
}
int OnInit()
{
	Print ("Connecting to bridge for symbol " + Symbol());
	connectBridge();
	if (Symbol() == leading_currency)
	  EventSetMillisecondTimer(100);
	return(0);
}
//+------------------------------------------------------------------+
//| expert deinitialization function                                 |
//+------------------------------------------------------------------+
void OnDeinit(const int reason)
{
	Print("deinit..");
	// Delete all objects from the chart.
	for(int i=ObjectsTotal()-1; i>-1; i--) {
		ObjectDelete(ObjectName(i));
	}
	Comment("");
   
	// Send Notification that bridge is down.
	// Format: bridge|testaccount DOWN
	if (isConnected())
	{
		mm_sendDown(server_link, TimeCurrent());
		mm_cleanup(server_link);
	}
	server_link = 0;

	Print("doen.");
	return;
}
//+------------------------------------------------------------------+
//| expert start function                                            |
//+------------------------------------------------------------------+

void OnTick()
{
	if (!isConnected())
	{
	   Comment("Not connected.");
	   connectBridge();
	   return;
	}
	Comment("Connected!");
	publishStock();
	
	// this needs to be executed only once, find a way to do that..
	if (Symbol() == leading_currency)
		publishGeneralData();
	
	return;
}

void OnTimer()
{
	if (!isConnected())
	{
	   Comment("Not connected!");
	   return;
	}
	//Print("Timering.");
	Comment("Receiving...");
	while (executeCommands());
}

int executeCommands()
{
   int availableCommand = mm_checkCommand(server_link);
   
   if (availableCommand == 0)
   {
   		Comment("Connected.");
   		return 0;
   }
   if (availableCommand == -1)
   {
   		Comment("Reception Problem!");
   		return 0;
   }
   
   Comment("Received message.");
   
   bool selected;
   int ticketID;
   double orderPrice;
   double takeProfitPrice;
   double stopLossPrice;
   double lotSize;
   
   if (availableCommand == MM_NEW_ORDER)
   {
   		int orderType = mm_receiveOrderType(server_link);
   		orderPrice = NormalizeDouble(mm_receiveOrderPrice(server_link), Digits);
   		takeProfitPrice = NormalizeDouble(mm_receiveTakeProfitPrice(server_link), Digits);
   		stopLossPrice = NormalizeDouble(mm_receiveStopLossPrice(server_link), Digits);
   		lotSize = NormalizeDouble(mm_receiveLotSize(server_link), Digits);
   		
   		ticketID = OrderSend(Symbol(),
                           orderType, 
                           lotSize,
                           orderPrice,
                           3,
                           0.0,
                           0.0,
                           NULL,
                           0,
                           TimeCurrent() + 3600,
                           Green); 
		if (ticketID < 0)
		{
			PrintFormat("OrderSend ERROR #%d (Price: %f, S/L: %f, T/P: %f, Bid: %f, Ask: %f)", GetLastError(), orderPrice, stopLossPrice, takeProfitPrice, Bid, Ask);
			return(0);
		}
		else
		{
			double minStopLossLevel = MarketInfo(Symbol(), MODE_STOPLEVEL);
			double price = Bid;
			double sign = -1.0;
			if (orderType == OP_SELL)
			{
				price = Ask;
				sign = 1.0;
			}
			double minStopLoss = NormalizeDouble(price + sign * minStopLossLevel * Point, Digits);
			if ((orderType == OP_BUY) && (minStopLoss < stopLossPrice)) stopLossPrice = minStopLoss;
			else if ((orderType == OP_SELL) && (minStopLoss > stopLossPrice)) stopLossPrice = minStopLoss;
			bool res = OrderModify(ticketID, 0, stopLossPrice, takeProfitPrice, 0);
			if(!res)
			{
				Print("OrderModify Error: ", GetLastError());
    			Print("IMPORTANT: ORDER #", ticketID, " HAS NO STOPLOSS AND TAKEPROFIT");
    		}
		}
		return 1;
   }
   
   if (availableCommand == MM_CLOSE_ORDER)
   {
   		ticketID = mm_receiveTicketID(server_link);

   		selected = OrderSelect(StrToInteger(ticketID), SELECT_BY_TICKET);
      	
      	if (!selected)
      	{
      		Print("Trying to close invalid order.");
      		return 1;
      	}
      	
		// Send the oder close instructions.
		bool close_ticket;
		if (OrderType() == OP_BUY)
		{
			close_ticket = OrderClose(OrderTicket(), OrderLots(), Bid, 3, Red);
		}
		else if (OrderType() == OP_SELL)
		{
			close_ticket = OrderClose(OrderTicket(), OrderLots(), Ask, 3, Red);
		}
		else if (OrderType() == OP_BUYLIMIT || OrderType() == OP_BUYSTOP || OrderType() == OP_SELLLIMIT || OrderType() == OP_SELLSTOP)
		{
			close_ticket = OrderDelete(OrderTicket());
		}
		
		if (close_ticket == false)
		{
			Print("OrderClose failed with error #",GetLastError());
			return 1;
		}
		else
		{
			
			Print("Closed trade: " + ticketID);
		}
		return 1;
   }
   
   if (availableCommand == MM_UPDATE_ORDER)
   {
   		ticketID = mm_receiveTicketID(server_link);
   		takeProfitPrice = NormalizeDouble(mm_receiveTakeProfitPrice(server_link), Digits);
   		stopLossPrice = NormalizeDouble(mm_receiveStopLossPrice(server_link), Digits);
   		
   		selected = OrderSelect(StrToInteger(ticketID), SELECT_BY_TICKET);
   		
   		if (!selected)
      	{
      		Print("Trying to update invalid order.");
      		return 1;
      	}
   		
   		bool update_ticket = OrderModify(OrderTicket(),
								OrderOpenPrice(),
								stopLossPrice,
								takeProfitPrice, 
								0, 
								Blue); 
   		
		if (!update_ticket)
		{
			Print("OrderModify failed with error #",GetLastError());
			return(0);
		}
		else
		{
			Print("New order opened!");
		}
		return 1;
   }
   
   /*string message2 = socketReceive();

   //message2 = "C David|11234 test test";
   if (message2 == "" || message2 == 0) return 0; 
   if (StringLen(message2) <= 3) return 1;
   // all commands need to start with "C accountname|uid"
   if (StringGetChar(message2, 0) != 'C') return 0; 
   //if (StringSubstr(message2, 0, 3) != "cmd") return 0;
   
   
   // always pull out name, uid, and pair which are required to control a trade
   // this allows multiple experts on multiple charts simultaneously
   int token = 0;
   string subscription, accountName, uid;
   int position = 0, lastPosition = 0;
   for (;(token <= 2) &&
   	(((position = StringFind(message2, "|", lastPosition)) != -1)
   	|| ((position = StringFind(message2, " ", lastPosition)) != -1))
   	; ++token)
   {
   	string part = StringSubstr(message2, lastPosition, position - lastPosition);
   	lastPosition = position + 1;
   	
   	switch (token)
   	{
   		case 0:
   			subscription = part; break;
   		case 1:
   			accountName = part; break;
   		case 2:
   			uid = part; break;
   		default:
   			break;
   	};
   }
   
   bool validQuery = token >= 2 && subscription != 0 && accountName != 0 && uid != 0;
   
   if (!validQuery)
   {
   	Print("Received invalid query.");
   	return -1;
   }
   else
   {
      Print("Received message: " + message2);
   	//if(send_response(uid, Symbol()) == false)
		//	Print("ERROR occurred sending response!");
   }
   
   
   // Trade wireframe for sending to MetaTrader specification:
   //
   // For new trade or order:
   // cmd|[account name]|[uid] set [trade_type] [pair] [open price] [take profit price] [stop loss price] [lot size]
   // ex=> 
   //   cmd|testaccount|fdjksalr38wufsd= set 2 EURUSD 1.25 1.2503 1.2450
   //   
   // For updating a trade:
   // cmd|[account name]|[uid] reset [ticket_id] [take profit price] [stop loss price]
   // ex=> 
   //   cmd|testaccount|fdjksalr38wufsd= reset 43916144 1.2515 1.2502
   //
   // For updating an order:
   // cmd|[account name]|[uid] reset [ticket_id] [take profit price] [stop loss price] [open price]
   // ex=> 
   //   cmd|testaccount|fdjksalr38wufsd= reset 43916144 1.2515 1.2502 1.2507
   //   
   // For closing a trade or order:
   // cmd|[account name]|[uid] unset [ticket_id]
   // ex=> 
   //   cmd|testaccount|fdjksalr38wufsd= unset 43916144
   
   // If new trade operation is requested.
   //
   // NOTE: MQL4's order type numbers are as follows:
   // 0 = (MQL4) OP_BUY - buying position,
   // 1 = (MQL4) OP_SELL - selling position,
   // 2 = (MQL4) OP_BUYLIMIT - buy limit pending position,
   // 3 = (MQL4) OP_SELLLIMIT - sell limit pending position,
   // 4 = (MQL4) OP_BUYSTOP - buy stop pending position,
   // 5 = (MQL4) OP_SELLSTOP - sell stop pending position.
   if (StringFind(message2, "reset", 0) != -1)
   {
      // Pull out request uid. Message is formatted: "cmd|[account name]|[uid] reset [ticket_id] [take profit price] [stop loss price] [optional open price]"
      
      // Initialize array to hold the extracted settings. 
      string trade_update_settings[4] = {"ticket_id", "take_profit", "stop_loss", "open_price"};
 
      // Pull out the trade settings.
      keyword = "reset";
      start_position = StringFind(message2, keyword, 0) + StringLen(keyword) + 1;
      end_position = StringFind(message2, " ", start_position + 1);
      
      for(int i = 0; i < ArraySize(trade_update_settings); i++)
      {
         trade_update_settings[i] = StringSubstr(message2, start_position, end_position - start_position);
         
         // Protect against looping back around to the beginning of the string by exiting if the new
         // start position would be a lower index then the current one.
         if(StringFind(message2, " ", end_position) < start_position)
            break;
         else 
         { 
            start_position = StringFind(message2, " ", end_position);
            end_position = StringFind(message2, " ", start_position + 1);
         }
      }

      // Select the requested order.
      OrderSelect(StrToInteger(trade_update_settings[0]),SELECT_BY_TICKET);

      // Since 'open_price' was not received, we know that we're updating a trade.
      bool update_ticket = false;
      if(trade_update_settings[3] == "open_price")
      {
      
         // Send the trade modify instructions.
         update_ticket = OrderModify(OrderTicket(),
                                  OrderOpenPrice(),
                                  NormalizeDouble(StrToDouble(trade_update_settings[2]), Digits),
                                  NormalizeDouble(StrToDouble(trade_update_settings[1]), Digits), 
                                  0, 
                                  Blue); 
      // Since 'open_price' was received, we know that we're updating an order.
      } else {
      Print(NormalizeDouble(StrToDouble(trade_update_settings[3]), Digits));
         // Send the order modify instructions.
         update_ticket = OrderModify(OrderTicket(),
                                  NormalizeDouble(StrToDouble(trade_update_settings[3]), Digits),
                                  NormalizeDouble(StrToDouble(trade_update_settings[2]), Digits),
                                  NormalizeDouble(StrToDouble(trade_update_settings[1]), Digits), 
                                  0, 
                                  Blue);
      }
               
      if(update_ticket == false)
      {
         Print("OrderSend failed with error #",GetLastError());
         return 1;
      }
      else
      {
         if(trade_update_settings[3] == "open_price")
         {
            Print("Trade: " + trade_update_settings[0] + " updated stop loss to: " + trade_update_settings[2] + " and take profit to: " + trade_update_settings[1]);
         }
         else
         {
            Print("Order: " + trade_update_settings[0] + " updated stop loss to: " + trade_update_settings[2] + ", take profit to: " + trade_update_settings[1] + ", and open price to: " + trade_update_settings[3]);
         }
         
         // Send response.
         if(send_response(uid, "Order has been processed.") == false)
            Print("ERROR occurred sending response!");
      }
   } 
   else if (StringFind(message2, "unset", 0) != -1)
   {  
      // Pull out the trade settings.
      keyword = "unset";
      start_position = StringFind(message2, keyword, 0) + StringLen(keyword) + 1;
      end_position = StringFind(message2, " ", start_position + 1);

      string ticket_id = StringSubstr(message2, start_position, end_position - start_position);
         
      // Select the requested order.
      OrderSelect(StrToInteger(ticket_id),SELECT_BY_TICKET);
      
      // Send the oder close instructions.
      bool close_ticket;
      if (OrderType() == OP_BUY)
      {
         close_ticket = OrderClose(OrderTicket(), OrderLots(), Bid, 3, Red);
      }
      else if (OrderType() == OP_SELL)
      {
         close_ticket = OrderClose(OrderTicket(), OrderLots(), Ask, 3, Red);
      }
      else if (OrderType() == OP_BUYLIMIT || OrderType() == OP_BUYSTOP || OrderType() == OP_SELLLIMIT || OrderType() == OP_SELLSTOP)
      {
         close_ticket = OrderDelete(OrderTicket());
      }
      
      if(close_ticket == false)
      {
         Print("OrderSend failed with error #",GetLastError());
         return 1;
      }
      else
      {
      
         Print("Closed trade: " + ticket_id);
         
         // Send response.
         if(send_response(uid, "Order has been processed.") == false)
            Print("ERROR occurred sending response!");
      }
      
   } 
   else if (StringFind(message2, "set", 0) != -1)
   { 
      // Initialize array to hold the extracted settings. 
      string trade_settings[6] = {"type", "pair", "open_price" ,"take_profit", "stop_loss", "lot_size"};
 
      // Pull out the trade settings.
      keyword = "set";
      start_position = StringFind(message2, keyword, 0) + StringLen(keyword) + 1;
      end_position = StringFind(message2, " ", start_position + 1);

      for(i = 0; i < ArraySize(trade_settings); i++)
      {
         trade_settings[i] = StringSubstr(message2, start_position, end_position - start_position);
         
         // Protect against looping back around to the beginning of the string by exiting if the new
         // start position would be a lower index then the current one.
         if(StringFind(message2, " ", end_position) < start_position)
            break;
         else 
         { 
            start_position = StringFind(message2, " ", end_position);
            end_position = StringFind(message2, " ", start_position + 1);
         }
      }
      
      double takeprofit = NormalizeDouble(StrToDouble(trade_settings[3]), Digits);
      double stoploss = NormalizeDouble(StrToDouble(trade_settings[4]), Digits);
      double lots = NormalizeDouble(StrToDouble(trade_settings[5]), Digits);
      double open = NormalizeDouble(StrToDouble(trade_settings[2]), Digits);
      int type = StrToInteger(trade_settings[0]);
      string pair = StringTrimLeft(trade_settings[1]);
      Print("" + type + " '" + pair + "', Open: " + open + ", TP: " + takeprofit + ", SL: " + stoploss + ", Lots: " + lots);
      
      // Open trade.
      Print(NormalizeDouble(StrToDouble(trade_settings[3]), Digits));
      ticket = OrderSend(pair,
                                       type, 
                                       lots,
                                       open,
                                       3,
                                       stoploss,
                                       takeprofit,
                                       NULL,
                                       0,
                                       TimeCurrent() + 3600,
                                       Green); 
      if(ticket<0)
      {
         Print("OrderSend failed with error #",GetLastError());
         return(0);
      }
      else 
      { 
         // Send response.
         if(send_response(uid, "Order has been processed.") == false)
            Print("ERROR occurred sending response!");
      }
   }
   
   // If a new element to be drawen is requested.
   if (StringFind(message2, "Draw", 0) != -1)
   {
      // Initialize array to hold the extracted settings. 
      string object_settings[7] = {"object_type", "window", "open_time", "open_price" ,"close_time", "close_price", "prediction"};
      
      // Pull out the drawing settings.
      keyword = "Draw";
      start_position = StringFind(message2, keyword, 0) + StringLen(keyword) + 1;
      end_position = StringFind(message2, " ", start_position + 1);

      for(i = 0; i < ArraySize(object_settings); i++)
      {
         object_settings[i] = StringSubstr(message2, start_position, end_position - start_position);
         
         // Protect against looping back around to the beginning of the string by exiting if the new
         // start position would be a lower index then the current one.
         if(StringFind(message2, " ", end_position) < start_position)
            break;
         else 
         { 
            start_position = StringFind(message2, " ", end_position);
            end_position = StringFind(message2, " ", start_position + 1);
         }
      }
      
      // ack uid.
      Print("uid: " + uid);
  
      // Generate UID
      double bar_uid = MathRand()%10001/10000.0;
         
      // Draw the rectangle object.
      Print("Drawing: ", object_settings[0], " ", object_settings[1], " ", object_settings[2], " ", object_settings[3], " ", object_settings[4], " ", object_settings[5], " ", object_settings[6]);
      if(!ObjectCreate("bar:" + bar_uid, draw_object_string_to_int(object_settings[0]), StrToInteger(object_settings[1]), StrToInteger(object_settings[2]), StrToDouble(object_settings[3]), StrToInteger(object_settings[4]), StrToDouble(object_settings[5])))
      {
        Print("error: cannot create object! code #",GetLastError());
        // Send response.
        send_response(uid, false);
      }
      else
      {
        // Color the bar based on the predicted direction. If no prediction was sent than the 
        // 'prediction' keyword will still occupy the array element and we need to set to Gray.
        if(StringFind(object_settings[6], "prediction", 0) != -1)
        {
           ObjectSet("bar:" + bar_uid, OBJPROP_COLOR, Gray);
        }
        else if(StrToDouble(object_settings[6]) > 0.5)
        {
           ObjectSet("bar:" + bar_uid, OBJPROP_COLOR, CadetBlue);
        }
        else if(StrToDouble(object_settings[6]) < 0.5)
        {
           ObjectSet("bar:" + bar_uid, OBJPROP_COLOR, IndianRed);
        }
        else
           ObjectSet("bar:" + bar_uid, OBJPROP_COLOR, Gray);
              
        // Send response.
        send_response(uid, true);
      }         
   }
   
   */
   return 1;
}

int publishStock()
{
	int error = mm_sendTick(server_link, Bid, Ask, TimeCurrent());
	if (error == 1)
	{
		Print("Error sending tick!");
		mm_cleanup(server_link);
		server_link = 0;
	}
	else
	{
		
	}
	return 0;
}

int publishGeneralData()
{
	// Publish account info.
	mm_sendAccountInfo(server_link, AccountLeverage(), AccountBalance(), AccountMargin(), AccountFreeMargin());
	
	// Look up the total number of open orders.
	int total_orders = OrdersTotal();
	int max_transmit = total_orders;
	if (max_transmit > 10) max_transmit = 10;
	//if (max_transmit == 0) return (0);
	
	int batch = mm_beginOrderBatch(server_link);
	
	// Build a json-like string for each order and add it to eh current_orders return string.  
	for(int position=0; position < max_transmit; position++)
	{
		if(OrderSelect(position,SELECT_BY_POS)==false) continue;
		mm_addOrder(batch, OrderType(), OrderTicket(), OrderOpenPrice(), OrderTakeProfit(), OrderStopLoss(), OrderOpenTime(), OrderExpiration(), OrderLots(), OrderProfit());
	}
	mm_sendOrderBatch(server_link, batch);
	return(0);
}


//+------------------------------------------------------------------+
//| Sends a response to a command. Messages are fomatted:
//|      => "response|[account name]|[uid] [some command]
//+------------------------------------------------------------------+
bool send_response(string uid, string response)
{
   return (true);
} 

//+------------------------------------------------------------------+
//| Returns the MetaTrader integer value for the string versions of the object types.
//+------------------------------------------------------------------+
int draw_object_string_to_int(string name)
{      

   // Initialize result holder with the error code incase a match is not found.
   int drawing_type_result = -1;
   
   // Initialize array of all of the drawing types for MQL4.
   // NOTE: They are in numerical order. I.E. OBJ_VLINE has
   //       a value of '0' and therefore is array element '0'.
   string drawing_types[24] = {
      "OBJ_VLINE", 
      "OBJ_HLINE", 
      "OBJ_TREND", 
      "OBJ_TRENDBYANGLE", 
      "OBJ_REGRESSION", 
      "OBJ_CHANNEL", 
      "OBJ_STDDEVCHANNEL", 
      "OBJ_GANNLINE", 
      "OBJ_GANNFAN",
      "OBJ_GANNGRID",
      "OBJ_FIBO",
      "OBJ_FIBOTIMES",
      "OBJ_FIBOFAN",
      "OBJ_FIBOARC",
      "OBJ_EXPANSION",
      "OBJ_FIBOCHANNEL",
      "OBJ_RECTANGLE",
      "OBJ_TRIANGLE",
      "OBJ_ELLIPSE",
      "OBJ_PITCHFORK",
      "OBJ_CYCLES",
      "OBJ_TEXT",
      "OBJ_ARROW",
      "OBJ_LABEL"
    };
   
    // Cycle throught the array to find a match to the specified 'name' value.
    // Once a match is found, store it's location within the array. This location
    // corresponds to the int value it should have.
    for(int i = 0; i < ArraySize(drawing_types); i++)
    {
      if(name == drawing_types[i])
      {
         drawing_type_result = i;
         break;
      }
    }
   
    // Return the int value the string would have had if it was a pointer of type int.
    switch(drawing_type_result)                                  
    {           
      case 0 : return(0);          break;               //	   Vertical line. Uses time part of first coordinate.
      case 1 : return(1);          break;               //	   Horizontal line. Uses price part of first coordinate.
      case 2 : return(2);          break;               //		Trend line. Uses 2 coordinates.
      case 3 : return(3);          break;               //		Trend by angle. Uses 1 coordinate. To set angle of line use ObjectSet() function.
      case 4 : return(4);          break;               //		Regression. Uses time parts of first two coordinates.
      case 5 : return(5);          break;               //		Channel. Uses 3 coordinates.
      case 6 : return(6);          break;               //		Standard deviation channel. Uses time parts of first two coordinates.
      case 7 : return(7);          break;               //		Gann line. Uses 2 coordinate, but price part of second coordinate ignored.
      case 8 : return(8);          break;               //		Gann fan. Uses 2 coordinate, but price part of second coordinate ignored.
      case 9 : return(9);          break;               //		Gann grid. Uses 2 coordinate, but price part of second coordinate ignored.
      case 10 : return(10);        break;               //		Fibonacci retracement. Uses 2 coordinates.
      case 11 : return(11);        break;               //		Fibonacci time zones. Uses 2 coordinates.
      case 12 : return(12);        break;               //		Fibonacci fan. Uses 2 coordinates.
      case 13 : return(13);        break;               //		Fibonacci arcs. Uses 2 coordinates.
      case 14 : return(14);        break;               //		Fibonacci expansions. Uses 3 coordinates.
      case 15 : return(15);        break;               //		Fibonacci channel. Uses 3 coordinates.
      case 16 : return(16);        break;               //		Rectangle. Uses 2 coordinates.
      case 17 : return(17);        break;               //		Triangle. Uses 3 coordinates.
      case 18 : return(18);        break;               //		Ellipse. Uses 2 coordinates.
      case 19 : return(19);        break;               //		Andrews pitchfork. Uses 3 coordinates.
      case 20 : return(20);        break;               //		Cycles. Uses 2 coordinates.
      case 21 : return(21);        break;               //		Text. Uses 1 coordinate.
      case 22 : return(22);        break;               //		Arrows. Uses 1 coordinate.
      case 23 : return(23);        break;               //	   Labels.
      default : return(-1);                             //     ERROR. NO MATCH FOUND.
   }
}
  
//+------------------------------------------------------------------+