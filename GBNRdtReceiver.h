#pragma once
#include "RdtReceiver.h"

class GBNRdtReceiver :public RdtReceiver {
private:
	int expectSequenceNumberRcvd;
	Packet lastAckPkt;
public:
	GBNRdtReceiver();
	virtual ~GBNRdtReceiver();
public:
	void receive(Packet &packet);		//recieve will be called NetworkService
};
