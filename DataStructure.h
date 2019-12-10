#pragma once

struct  Configuration {
	//byte size of payload
	static const int PAYLOAD_SIZE = 21;

	//timeout
	static const int TIME_OUT = 20;
};
 
struct  Message {
	//appication layer message content
	char data[Configuration::PAYLOAD_SIZE];
	Message();
	Message(const Message &msg);
	virtual Message & operator=(const Message &msg);
	virtual ~Message();

	virtual void print();
};

//transmiting layer data packet
struct  Packet {
	int seqnum;										//sequence number
	int acknum;										//ACK
	int checksum;									//check sum
	char payload[Configuration::PAYLOAD_SIZE];		//content of message

	Packet();
	Packet(const Packet& pkt);
	virtual Packet & operator=(const Packet& pkt);
	virtual bool operator==(const Packet& pkt) const;
	virtual ~Packet();

	virtual void print();
};