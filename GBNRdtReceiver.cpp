#pragma once
#include "pch.h"
#include "Global.h"
#include "GBNRdtReceiver.h"
#include "GBNRdtSender.h"

// ���캯������ʼ��ȷ�ϱ���
GBNRdtReceiver::GBNRdtReceiver() {
	expectSequenceNumberRcvd = 0;
	// ȷ�Ϻ�
	//��ʼ״̬�£��ϴη��͵�ȷ�ϰ���ȷ�����Ϊ7��ʹ�õ���һ�����ܵ����ݰ�����ʱ��ȷ�ϱ��ĵ�ȷ�Ϻ�Ϊ7
	lastAckPkt.acknum = GBNRdtSender::SEQNUM_MAX - 1;
	// У���
	lastAckPkt.checksum = 0;
	// ���
	lastAckPkt.seqnum = -1;	//���Ը��ֶΡ�
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
		pns->delivertoAppLayer(RECEIVER, msg);	// ���Ľ���
		lastAckPkt.acknum = packet.seqnum;	// ȷ�Ϻ�
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("���շ� ���� ȷ�ϱ���", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	// ȷ�ϱ��ĵķ���
		// ͨ����������ֵȡ��ʵ�������н�
		expectSequenceNumberRcvd = (expectSequenceNumberRcvd + 1) % GBNRdtSender::SEQNUM_MAX;
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("���շ� �յ� У�������", packet);
		}
		else {
			pUtils->printPacket("���շ� �յ� ��Ŵ�����", packet);
		}
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