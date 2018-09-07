#include "CAgent_DQN.h"
#include "CTraderSpi.h"
#include <algorithm>
#include <cmath>
#include <queue>
#include "CThreadPool.h"
#include "pthread.h"
#include "string.h"
#include <sstream> 

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "pthread.h"
#include "sys/types.h"
#include "assert.h"
#include "fstream"

extern CTraderSpi *pTraderSpi;
extern CThostFtdcTraderApi* pTraderApi;
extern CThread_Pool *_ThreadPool;

extern int iRequestID;

extern ThostFtdcParams Citic_Thost_Params;

//¶©µ¥Ïß³Ì¿ØÖÆ×é±äÁ¿
extern vector<int> vcOrderRef;
extern queue<CThostFtdcInputOrderField> qInsertOrderQueue;
extern queue<CThostFtdcInputOrderActionField> qCancelOrderQueue;
extern pthread_mutex_t csInsert;
extern pthread_mutex_t csCancel;
extern pthread_cond_t hInsertEvent;
extern pthread_cond_t hCancelEvent;

extern int PULL_ORDER_LIMIT;
extern bool ORDER_OVER_POSITION;

//void *SliceCallBack(void *pvContext);

CAgent::CAgent(){
	memset(Timer, 0, sizeof(Timer));

	bLongOrderFilled = true;
	bShortOrderFilled = true;

	bLastTradedLong = false;
	bLastTradedShort = false;

	bInitialVolume = true;
	bTickChanged = true;

	bMktClosing = false;
	bStopTrading = false;

	bLongClosingFlagEnable = true;
	bShortClosingFlagEnable = true;

	bCriticalBid = false;
	bCriticalAsk = false;

	mpUnfilledOrder.clear();
	mpUnfilledOrderLots.clear();
	mpUnfilledOrderSizeForCheck.clear();

	Hours = 0;
	Minutes = 0;
	Seconds = 0;

	InitVolume = 0;
	TotalTrades = 0;

	DepthAB.ask_price = 0;
	DepthAB.bid_price = 0;

	LongLimitPrice = 0;
	ShortLimitPrice = 0;
	LastOpenPrice = 0;

	Posi.mp_PriceMap.clear();
	Posi.NetPosition = 0;
	Posi.TotalProfit = 0;
	Posi.TotalValue = 0;
	Posi.Direction_Flag = 0;

	InstrumentTickSize = 0;
	InstrumentLeverage = 0;

	pthread_mutex_init(&csInstrumentInternalLock,NULL);
	pthread_mutex_init(&csInstrumentUnfillFlagLock,NULL);
	pthread_mutex_init(&csInstrumentUnfillOrderLock,NULL);

	Start_ZMQ_Server();
}

CAgent::~CAgent(){
	if (LongLimitPrice != 0)
		PullOrderBack(LongLimitPrice);
	if (ShortLimitPrice != 0)
		PullOrderBack(ShortLimitPrice);
	pthread_mutex_destroy(&csInstrumentInternalLock);
	pthread_mutex_destroy(&csInstrumentUnfillFlagLock);
	pthread_mutex_destroy(&csInstrumentUnfillOrderLock);
}

void CAgent::SliceThreadPool(CThostFtdcDepthMarketDataField *DepthData){
	if (InstrumentID == DepthData->InstrumentID){
		this->SprDepthData = DepthData;
		Adding_Worker(_ThreadPool,SliceCallBack,this);
	};
}


void *CAgent::SliceCallBack(void *arg){
	CAgent *pThis = (CAgent*)arg;

	pthread_mutex_lock(&pThis->csInstrumentInternalLock);

	string tempID = pThis->SprDepthData->InstrumentID;

	//record time stamp
	pThis->TradingDATE = pThis->SprDepthData->TradingDay;
	pThis->TradingTIME = pThis->SprDepthData->UpdateTime;

	string TempTimingStr = pThis->SprDepthData->UpdateTime;
	int pos = 0;

	pos = TempTimingStr.find(":");
	pThis->Hours = atoi(TempTimingStr.substr(0, pos).c_str());
	TempTimingStr.erase(0, pos+1);
	pos = TempTimingStr.find(":");
	pThis->Minutes = atoi(TempTimingStr.substr(0, pos).c_str());
	TempTimingStr.erase(0, pos+1);
	pThis->Seconds = atoi(TempTimingStr.c_str());

	if (!pThis->bMktClosing){
		if ((pThis->Hours == 14 && pThis->Minutes == 48 && pThis->Seconds > 30)
			|| (pThis->Hours == 10 && pThis->Minutes == 13 && pThis->Seconds > 30)
			|| (pThis->Hours == 11 && pThis->Minutes == 27 && pThis->Seconds > 30)
			|| (pThis->Hours == 23 && pThis->Minutes == 27 && pThis->Seconds > 30)
			|| (pThis->Hours == 0 && pThis->Minutes == 55 && pThis->Seconds > 30))
			pThis->bMktClosing = true;
	}
	else{
		if ((pThis->Hours == 9 && pThis->Minutes == 25 && pThis->Seconds > 30)
			|| (pThis->Hours == 10 && pThis->Minutes == 32 && pThis->Seconds > 30)
			|| (pThis->Hours == 13 && pThis->Minutes == 32 && pThis->Seconds > 30)
			|| (pThis->Hours == 23 && pThis->Minutes == 32 && pThis->Seconds > 30)){
			pThis->bMktClosing = false;
			pThis->bShortOrderFilled = true;
			pThis->bLongOrderFilled = true;
		}
	}
	
	if (!pThis->bMktClosing
		&& !pThis->bStopTrading){
		//pThis->CheckLimitOrderQuene(false);
		//pThis->DQN_Action(pThis->SprDepthData);
	}
	else{
		pThis->CleanPosition(pThis->SprDepthData);
	}

	pThis->CalculatePnL(pThis->SprDepthData);

	pthread_mutex_unlock(&pThis->csInstrumentInternalLock);
	return NULL;
}

void CAgent::DeleteLimitOrderFilled(double price, string OrderSysID, char Dir, int _OrderSize)
{
	pthread_mutex_lock(&csInstrumentUnfillOrderLock);

	if (!mpUnfilledOrder.empty()){
		if (mpUnfilledOrder.find(price) != mpUnfilledOrder.end()){
			if (mpUnfilledOrder[price].find(OrderSysID) != mpUnfilledOrder[price].end()
				&& !mpUnfilledOrder[price].empty()){

				mpUnfilledOrder[price].erase(mpUnfilledOrder[price].find(OrderSysID));
			}
		}
	}

	if (!mpUnfilledOrderLots.empty()){
		if (mpUnfilledOrderLots.find(OrderSysID) != mpUnfilledOrderLots.end())
			mpUnfilledOrderLots.erase(mpUnfilledOrderLots.find(OrderSysID));
	}

	if (!mpUnfilledOrderSizeForCheck.empty()){
		mpUnfilledOrderSizeForCheck[price] -= _OrderSize;
	}


	if (Dir == THOST_FTDC_D_Buy)
		LongLimitPrice = 0;
	else if (Dir == THOST_FTDC_D_Sell)
		ShortLimitPrice = 0;

	pthread_mutex_unlock(&csInstrumentUnfillOrderLock);
}

void CAgent::DeleteLimitOrderCancel(double price, string OrderSysID, char Dir, char OffsetFlag, int _OrderSize){
	pthread_mutex_lock(&csInstrumentUnfillOrderLock);

	if (!mpUnfilledOrder.empty()){
		if (mpUnfilledOrder.find(price) != mpUnfilledOrder.end()){
			if (mpUnfilledOrder[price].find(OrderSysID) != mpUnfilledOrder[price].end()
				&& !mpUnfilledOrder[price].empty()){
				mpUnfilledOrder[price].erase(mpUnfilledOrder[price].find(OrderSysID));
			}
		}
	}

	if (!mpUnfilledOrderLots.empty()){
		if (mpUnfilledOrderLots.find(OrderSysID) != mpUnfilledOrderLots.end())
			mpUnfilledOrderLots.erase(mpUnfilledOrderLots.find(OrderSysID));
	}

	if (!mpUnfilledOrderSizeForCheck.empty()){
		mpUnfilledOrderSizeForCheck[price] -= _OrderSize;
	}

	pthread_mutex_unlock(&csInstrumentUnfillOrderLock);
}

void CAgent::SetUnfilledLots(string sysID, int TotalVolume,double price){
	pthread_mutex_lock(&csInstrumentUnfillOrderLock);

	mpUnfilledOrderLots[sysID] = TotalVolume;
	mpUnfilledOrderSizeForCheck[price] = TotalVolume;

	pthread_mutex_unlock(&csInstrumentUnfillOrderLock);
}

void CAgent::DQN_Action(CThostFtdcDepthMarketDataField * DepthData)
{
	pthread_mutex_lock(&csInstrumentUnfillFlagLock);

	char CloseOffSetFlag;
	if (InstrumentExchangeID == "SHFE")
		CloseOffSetFlag = THOST_FTDC_OF_CloseToday;
	else
		CloseOffSetFlag = THOST_FTDC_OF_Close;


	ptAgInfo.set_agent_action(88);

	int i = 1;
	//while(1){
	ptAgInfo.set_agent_action(i++);
	string proto_buffer;
	ptAgInfo.SerializeToString(&proto_buffer);

	printf("waiting\n");

	zmq_msg_t request;
	zmq_msg_init_size(&request,100);
	int size = zmq_msg_recv(&request,ZMQ_Responder,0);
	char buff[100];
	memset(buff,0,100*sizeof(char));
	memcpy(buff,zmq_msg_data(&request),100);
	ptAgInfo.ParseFromArray(buff,100);
	cerr<<"Agent Action: "<<ptAgInfo.agent_action()<<endl;
	memset(buff,0,100*sizeof(char));

	zmq_msg_t reply;
	int reply_size = proto_buffer.size();
	int ret = zmq_msg_init_size(&reply,100);
	memcpy(zmq_msg_data(&reply),proto_buffer.c_str(),reply_size);
	zmq_msg_send(&reply,ZMQ_Responder,0);
	//}

	zmq_close(ZMQ_Responder);
	zmq_ctx_destroy(ZMQ_Context);

	pthread_mutex_unlock(&csInstrumentUnfillFlagLock);
}

void CAgent::CleanPosition(CThostFtdcDepthMarketDataField * DepthData)
{
	pthread_mutex_lock(&csInstrumentUnfillFlagLock);

	char CloseOffSetFlag;
	if (InstrumentExchangeID == "SHFE")
		CloseOffSetFlag = THOST_FTDC_OF_CloseToday;
	else
		CloseOffSetFlag = THOST_FTDC_OF_Close;

	//if (bLongOrderFilled)
	//{
	//	if (ShortInventory > 0)
	//	{
	//		CheckLimitOrderQuene(true);
	//		PendingOrder(DepthData, DepthData->AskPrice1, ShortInventory, CloseOffSetFlag, THOST_FTDC_D_Buy);;
	//		bShortOrderFilled = false;
	//	}
	//}

	//if (bShortOrderFilled)
	//{
	//	if (LongInventory > 0)
	//	{
	//		CheckLimitOrderQuene(true);
	//		PendingOrder(DepthData, DepthData->BidPrice1, LongInventory, CloseOffSetFlag, THOST_FTDC_D_Sell);
	//		bShortOrderFilled = false;
	//	}
	//}



	pthread_mutex_unlock(&csInstrumentUnfillFlagLock);
}

void CAgent::PendingOrder(CThostFtdcDepthMarketDataField *DepthData, double price, int lots, char offlag, char direction)
{
	CThostFtdcInputOrderField req;
	memset(&req, 0, sizeof(req));

	//if (offlag == THOST_FTDC_OF_Close
	//	|| offlag == THOST_FTDC_OF_CloseToday){
	//	if (direction == THOST_FTDC_D_Buy){
	//		if (lots <= LongAvailable)
	//			req.CombOffsetFlag[0] = offlag;
	//		else
	//			req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	//	}

	//	if (direction == THOST_FTDC_D_Sell){
	//		if (lots <= ShortAvailable)
	//			req.CombOffsetFlag[0] = offlag;
	//		else
	//			req.CombOffsetFlag[0] = THOST_FTDC_OF_Open;
	//	}
	//}	

	strcpy(req.BrokerID,Citic_Thost_Params.BROKER_ID);
	strcpy(req.InvestorID, Citic_Thost_Params.INVESTOR_ID);
	strcpy(req.InstrumentID, DepthData->InstrumentID);
	strcpy(req.OrderRef, Citic_Thost_Params.ORDER_REF);
	//	TThostFtdcUserIDType	UserID;
	req.OrderPriceType = THOST_FTDC_OPT_LimitPrice;

	req.Direction = direction;

	req.CombOffsetFlag[0] = offlag;

	req.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
	req.LimitPrice = price;
	req.VolumeTotalOriginal = abs(lots);
	req.TimeCondition = THOST_FTDC_TC_GFD;
	//	TThostFtdcDateType	GTDDate;
	req.VolumeCondition = THOST_FTDC_VC_AV;
	req.MinVolume = 1;
	req.ContingentCondition = THOST_FTDC_CC_Immediately;
	//	TThostFtdcPriceType	StopPrice;
	req.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
	req.IsAutoSuspend = 0;
	//	TThostFtdcBusinessUnitType	BusinessUnit;
	//	TThostFtdcRequestIDType	RequestID;
	req.UserForceClose = 0;

	req.CombOffsetFlag[0] = offlag;

	if (direction == THOST_FTDC_D_Buy)
		LongLimitPrice = price;
	else if (direction == THOST_FTDC_D_Sell)
		ShortLimitPrice = price;

	pthread_mutex_lock(&csInsert);

	qInsertOrderQueue.push(req);

	pthread_cond_signal(&hInsertEvent);
	pthread_mutex_unlock(&csInsert);

	//SetEvent(hInsertEvent);
}

void CAgent::PullOrderBack(double price){
	pthread_mutex_lock(&csInstrumentUnfillOrderLock);
	pthread_mutex_lock(&csCancel);

	if (mpUnfilledOrder.find(price) != mpUnfilledOrder.end()
		&& !mpUnfilledOrder[price].empty()){
		for (map<string, CThostFtdcInputOrderActionField>::iterator mp_it = mpUnfilledOrder[price].begin();
			mp_it != mpUnfilledOrder[price].end();){

			qCancelOrderQueue.push(mp_it->second);
			mpUnfilledOrder[price].erase(mp_it++);
		}
	}
	//SetEvent(hCancelEvent);
	pthread_cond_signal(&hCancelEvent);

	pthread_mutex_unlock(&csCancel);
	pthread_mutex_unlock(&csInstrumentUnfillOrderLock);
}

void CAgent::SetInstrumentID(char *ID){
	InstrumentID = ID;
}

void CAgent::SetPositionStatus(CThostFtdcTradeField * pTrade){
	//Output
	string OffSetFlag, Direction, Date, OrderSysID;
	Date = pTrade->TradeDate;
	string sz_path = "./TradeLog_" + Posi.InstrumentID + "_" + Date + ".csv";
	ofstream TradeLog(sz_path.c_str(), ios::app);

	int tempLots = 0;
	int tempTotalLots = 0;
	Posi.InstrumentID = pTrade->InstrumentID;
	OrderSysID = pTrade->OrderSysID;
	if (pTrade->Direction == '0'){
		tempLots = pTrade->Volume;
		//LongLimitPrice = 0;
	}
	else{
		tempLots = -1 * pTrade->Volume;
		//ShortLimitPrice = 0;
	}

	if (Posi.mp_PriceMap.find(pTrade->Price) == Posi.mp_PriceMap.end())
		Posi.mp_PriceMap[pTrade->Price] = tempLots;
	else
		Posi.mp_PriceMap[pTrade->Price] = Posi.mp_PriceMap[pTrade->Price] + tempLots;

	for (map<double, int>::iterator mpit = Posi.mp_PriceMap.begin(); mpit != Posi.mp_PriceMap.end(); mpit++)
	{
		//cerr <<Posi.InstrumentID << " " << mpit->first << " " << mpit->second<<endl;
		tempTotalLots += mpit->second;
	}
	Posi.NetPosition = tempTotalLots;

	if (pTrade->OffsetFlag == '0')
	{
		OffSetFlag = "OPEN";
		LastOpenPrice = pTrade->Price;

		//DiretionType is a char '0'=buy '1'=sell
		//OffsetFlag is a char '0'=open '1'=close
		if (pTrade->Direction == '0'){
			Direction = "LONG";
			bLastTradedLong = true;
			bLastTradedShort = false;
		}
		else{
			Direction = "SHORT";
			bLastTradedLong = false;
			bLastTradedShort = true;
		}

		if (Posi.mp_PositionDistribution.find(pTrade->Price) == Posi.mp_PositionDistribution.end())
			Posi.mp_PositionDistribution[pTrade->Price] = tempLots;
		else
			Posi.mp_PositionDistribution[pTrade->Price] += tempLots;

		double tempTotalPosiValue = 0;
		double tempTotolLots = 0;
		for (map<double, int>::iterator mpit = Posi.mp_PositionDistribution.begin(); 
		     mpit != Posi.mp_PositionDistribution.end();
		     mpit++){
			tempTotalPosiValue += mpit->first*mpit->second;
			tempTotolLots += mpit->second;
		}
		Posi.AveragePositionEntry = tempTotalPosiValue / tempTotolLots;

		if (pTrade->Volume != 0){
			TotalTrades += pTrade->Volume;
			if (TradeLog.is_open())
				TradeLog << pTrade->InstrumentID << "," << OffSetFlag << "," << Direction << "," << pTrade->TradeTime << "," << pTrade->Price << "," << pTrade->Volume << "," << TotalTrades << endl;
			TradeLog.close();
		}
	}
	else{
		OffSetFlag = "CLOSE";
		LastOpenPrice = 0;

		//DiretionType is a char '0'=buy '1'=sell
		//OffsetFlag is a char '0'=open '1'=close
		if (pTrade->Direction == '0'){
			Direction = "LONG";
			bLastTradedLong = true;
			bLastTradedShort = false;
		}
		else{
			Direction = "SHORT";
			bLastTradedLong = false;
			bLastTradedShort = true;
		}

		Posi.mp_PositionDistribution.clear();
		Posi.AveragePositionEntry = 0;


		if (pTrade->Volume != 0){
			if (TradeLog.is_open())
				TradeLog << pTrade->InstrumentID << "," << OffSetFlag << "," << Direction << "," << pTrade->TradeTime << "," << pTrade->Price << "," << pTrade->Volume << "," << "," << Posi.TotalProfit << endl << endl;
			TradeLog.close();
		}

	}
}

void CAgent::SetCancelField(CThostFtdcOrderField * pOrder,int OriginVolume){
	CThostFtdcInputOrderActionField req;
	//char tmpDirection = pOrder->Direction;
	double OrderPrice;
	string sysID;

	pthread_mutex_lock(&csInstrumentUnfillOrderLock);

	memset(&req, 0, sizeof(req));
	strcpy(req.BrokerID, pOrder->BrokerID);
	strcpy(req.InvestorID, pOrder->InvestorID);
	strcpy(req.OrderRef, pOrder->OrderRef);
	strcpy(req.InstrumentID, pOrder->InstrumentID);

	strcpy(req.ExchangeID, pOrder->ExchangeID);
	strcpy(req.OrderSysID, pOrder->OrderSysID);
	req.ActionFlag = THOST_FTDC_AF_Delete;
	req.LimitPrice = pOrder->LimitPrice;

	OrderPrice = pOrder->LimitPrice;
	sysID = pOrder->OrderSysID;

	mpUnfilledOrder[OrderPrice][sysID]= req;
	mpUnfilledOrderLots[sysID] = OriginVolume;
	if (!mpUnfilledOrderSizeForCheck.empty()){
		if (mpUnfilledOrderSizeForCheck.find(OrderPrice) != mpUnfilledOrderSizeForCheck.end())
			mpUnfilledOrderSizeForCheck[OrderPrice] += OriginVolume;
		else
			mpUnfilledOrderSizeForCheck[OrderPrice] = OriginVolume;
	}

	pthread_mutex_unlock(&csInstrumentUnfillOrderLock);
}

void CAgent::ResetOrderDoneFlag(CThostFtdcOrderField * pOrder){
	pthread_mutex_lock(&csInstrumentUnfillFlagLock);

	if (InstrumentID == pOrder->InstrumentID){
		if (pOrder->Direction == '1'){
			bShortOrderFilled = true;
		}
		
		if (pOrder->Direction == '0'){
			bLongOrderFilled = true;
		}
	}

	pthread_mutex_unlock(&csInstrumentUnfillFlagLock);
}

void CAgent::ResetOrderDoneFlag(string& _ID, char _direction){
	pthread_mutex_lock(&csInstrumentUnfillFlagLock);

	if (InstrumentID == _ID){
		if (_direction == '1'){
			bShortOrderFilled = true;
		}

		if (_direction == '0'){
			bLongOrderFilled = true;
		}
	}

	pthread_mutex_unlock(&csInstrumentUnfillFlagLock);
}

void CAgent::CalculatePnL(CThostFtdcDepthMarketDataField *DepthData){
	Posi.InstrumentID = DepthData->InstrumentID;
	Posi.TotalValue = 0;
	double total_lots = 0;
	for (map<double, int>::iterator mpit = Posi.mp_PriceMap.begin(); mpit != Posi.mp_PriceMap.end(); mpit++){
		//cerr <<Posi.InstrumentID << " " << mpit->first << " " << mpit->second<<endl;
		Posi.TotalValue = Posi.TotalValue + mpit->first*mpit->second;
		total_lots += mpit->second;
	}
	Posi.TotalProfit = (DepthData->LastPrice*Posi.NetPosition - Posi.TotalValue) / InstrumentTickSize;
	Posi.CostLine = Posi.TotalValue / total_lots;
}

double CAgent::Factorial(double fc)
{
	if (fc < 0)
		return 1;
	else if (fc == 0 || fc == 1)
		return 1;
	else if (fc > 1)
		return fc*Factorial(fc - 1);
	else
		return -1;
}

string CAgent::GetTimer(){
	return SprDepthData->UpdateTime;
}

void CAgent::Start_ZMQ_Server(){
	ZMQ_Context = zmq_ctx_new();
	ZMQ_Responder = zmq_socket(ZMQ_Context, ZMQ_REP);
	int rc = zmq_bind(ZMQ_Responder, "ipc:///tmp/dqnagent");
	assert(rc == 0);

	// need set the machinesm well in this section
	int i = 1;
	while(1){
		char buff[100];
		zmq_msg_t request;
		zmq_msg_t reply;

		ptAgInfo.set_agent_action(i++);
		string proto_buffer;
		ptAgInfo.SerializeToString(&proto_buffer);

		printf("waiting\n");

		zmq_msg_init_size(&request,100);
		int size = zmq_msg_recv(&request,ZMQ_Responder,0);
		memset(buff,0,100 * sizeof(char));
		memcpy(buff,zmq_msg_data(&request),100);
		ptAgInfo.ParseFromArray(buff,100);
		cerr<<"InstrumentID: "<<ptAgInfo.agent_trading_instrument()<<endl;
		cerr<<"Agent Action: "<<ptAgInfo.agent_action()<<endl;

		//using strftime in PY to get the action timing and compare
		if(Previous_Action_Timestamp != ptAgInfo.action_timestamp())
			Previous_Action_Timestamp = ptAgInfo.action_timestamp();
		else
			cerr<<"Same Action \n";

		cerr<<"Timestamp   : "<<ptAgInfo.action_timestamp()<<endl;

		int reply_size = proto_buffer.size();
		int ret = zmq_msg_init_size(&reply,100);
		memcpy(zmq_msg_data(&reply),proto_buffer.c_str(),reply_size);
		zmq_msg_send(&reply,ZMQ_Responder,0);
	}

	zmq_close(ZMQ_Responder);
	zmq_ctx_destroy(ZMQ_Context);
}













