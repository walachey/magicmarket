string MetaTraderBridge="MetaTraderBridge.mqh Ver 1.0, David D.";
#import "MetaTraderBridge.dll"
int mm_init(char &server[], int port, char &currencyPair[]);
int mm_cleanup(int link);
int mm_sendUp(int link, int timestamp);
int mm_sendDown(int link, int timestamp);
int mm_sendTick(int link, double bid, double ask, int timestamp);
int mm_sendAccountInfo(int link, double leverage, double balance, double margin, double freeMargin);
int mm_sendError(int link, string message);
int mm_beginOrderBatch(int link);
int mm_addOrder(int batch, int type, int ticketID, double openPrice, double takeProfit, double stopLoss, int timestampOpen, int timestampExpire, double lots, double profit);
int mm_sendOrderBatch(int link, int batch);
#import
