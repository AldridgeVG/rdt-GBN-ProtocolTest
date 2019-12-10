#pragma once
#include "RdtReceiver.h"

class GBNRdtReceiver :public RdtReceiver {
private:
	int expectSequenceNumberRcvd;
	Packet lastAckPkt;
	int seqmax;
public:
	GBNRdtReceiver(Configuration config);
	virtual ~GBNRdtReceiver();
public:
	void receive(Packet &packet);		//recieve will be called NetworkService
};
