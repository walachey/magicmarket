The MetaTrader stuff needs to go into the user folder:
	%APPDATA%\MetaQuotes\Terminal\E6734F6041F45FBC490D7BD7FB1CB8E3\MQL4\
	
Create a symlink:
	Go to MQL4 user-folder.
	Click File->Open Command Prompt->As Administrator
	
	mklink Experts/MagicMarketBridge.mq4 <repospath>/MQL4/Experts/MagicMarketBridge.mq4
	mklink Include/socket.mqh <repospath>/MQL4/Include/socket.mqh
	mklink Include/WinSock.mqh <repospath>/MQL4/Include/WinSock.mqh
	
	