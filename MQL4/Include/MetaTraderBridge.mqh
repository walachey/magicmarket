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

#define MM_NEW_ORDER 7
#define MM_CLOSE_ORDER 8
#define MM_UPDATE_ORDER 9

int mm_checkCommand(int link);
int mm_receiveTicketID(int link);
int mm_receiveOrderType(int link);
double mm_receiveOrderPrice(int link);
double mm_receiveTakeProfitPrice(int link);
double mm_receiveStopLossPrice(int link);
double mm_receiveLotSize(int link);
#import
