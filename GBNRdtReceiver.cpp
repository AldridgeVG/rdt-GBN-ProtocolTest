#pragma once
#include "pch.h"
#include "Global.h"
#include "GBNRdtReceiver.h"

// ���캯������ʼ��ȷ�ϱ���
GBNRdtReceiver::GBNRdtReceiver(Configuration config) {
	expectSequenceNumberRcvd = 0;
	//������д���
	seqmax = config.SEQNUM_MAX;
	//��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ7��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ7
	lastAckPkt.acknum = config.SEQNUM_MAX - 1;
	// У���
	lastAckPkt.checksum = 0;
	// ���
	lastAckPkt.seqnum = -1;
	for (int i = 0; i < Configuration::PAYLOAD_SIZE; i++) {
		lastAckPkt.payload[i] = '.';// ��������
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
}

// ��������
GBNRdtReceiver::~GBNRdtReceiver() {}

// ���ձ��ģ�����NetworkService����
void GBNRdtReceiver::receive(Packet &packet) {
	int checkSum = pUtils->calculateCheckSum(packet);	// У���
	// ����յ��ı�����ŵ��ڽ��շ��ڴ��յ��ı�����ţ���У�����ȷ
	if (expectSequenceNumberRcvd == packet.seqnum && checkSum == packet.checksum) {
		pUtils->printPacket("���շ� �յ� ��ȷ����", packet);
		// ȡ��Message�����ϵݽ���Ӧ�ò�
		Message msg;
		memcpy(msg.data, packet.payload, sizeof(packet.payload));
		pns->delivertoAppLayer(RECEIVER, msg);	//���Ľ���
		//�����µ�ack������
		lastAckPkt.acknum = packet.seqnum;	//ȷ�Ϻ�
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ� ���� ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//ȷ�ϱ��ĵķ���
		//��seqmaxȡ��ʵ�������н�
		expectSequenceNumberRcvd = (expectSequenceNumberRcvd + 1) % seqmax;
	}
	//����յ������жϴ�������
	else {
		//У��Ͳ���
		if (checkSum != packet.checksum) {
			pUtils->printPacket("���շ� �յ� У�������", packet);
		}
		//��Ų���
		else {
			pUtils->printPacket("���շ� �յ� ��Ŵ�����", packet);
		}
		//�����ϸ�ack
		pUtils->printPacket("���շ� �ط� ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	// �ϴ�ȷ�ϱ��ĵ��ط�
	}
	if (DebugSign) {
		system("pause");
		cout << endl;
		cout << endl;
		cout << endl;
	}
}