#include "pch.h"
#include "Global.h"
#include "GBNRdtSender.h"

GBNRdtSender::GBNRdtSender(Configuration config) {
	/*
	Configuration::SEQNUM_BIT = 3;
	Configuration::SEQNUM_MAX = 8;
	Configuration::WINDOW_N = 4;

	cout << "input bits of seqnum_max:" << endl;
	cin >> Configuration::SEQNUM_BIT;
	Configuration::SEQNUM_MAX = 1;
	for (int i = 0; i < Configuration::SEQNUM_BIT; i++)	Configuration::SEQNUM_MAX *= 2;

	cout << "sequence max: " << Configuration::SEQNUM_MAX << endl;
	cout << "input window size" << endl;
	cin >> Configuration::WINDOW_N;
	cout << "window size: " << Configuration::WINDOW_N;
	*/
	expectSequenceNumberSend = 0;			// ��һ��������� 
	waitingState = false;									// �Ƿ��ڵȴ�Ack��״̬
	pPacketWaitingAck = new Packet[config.SEQNUM_MAX];
	windowN = config.WINDOW_N;	// �������ڴ�С
	base = 0;													// ��ǰ���ڵ���ŵĻ�ַ����receive����ά�� 
	rMax = config.SEQNUM_MAX;		// ��������Ͻ磬��������ΪrMax-1
	nowlMax = windowN;	 							// ���������ұ߽磬���ΪrMax����СΪ1,��receive����ά��
}

GBNRdtSender::~GBNRdtSender() {
	if (pPacketWaitingAck) {
		delete[] pPacketWaitingAck;
		pPacketWaitingAck = 0;
	}
}

bool GBNRdtSender::getWaitingState() {
	return waitingState;
}

bool GBNRdtSender::send(Message &message) {
	if (waitingState)	//���ͷ����ڵȴ�ȷ��״̬
		return false;
	// ��ǰ����ķ���
	Packet *pPacketDel = &pPacketWaitingAck[expectSequenceNumberSend];
	// ��ţ���һ�����͵���ţ���ʼΪ0
	pPacketDel->seqnum = expectSequenceNumberSend;
	// ȷ�Ϻ�
	pPacketDel->acknum = -1;	// ���Ը��ֶ�
	// У���
	pPacketDel->checksum = 0;

	memcpy(pPacketDel->payload, message.data, sizeof(message.data));
	pPacketDel->checksum = pUtils->calculateCheckSum(*pPacketDel);
	pUtils->printPacket("���ͷ� ���� ԭʼ����", *pPacketDel);
	// ���ڵĵ�һ�����ͱ�����Ҫ������ʱ�����ü�ʱ���ı����base
	if (base == expectSequenceNumberSend) {
		pns->startTimer(SENDER, Configuration::TIME_OUT, base);
		// cout << endl;
	}
	//����ģ�����绷����sendToNetworkLayer��ͨ������㷢�͵��Է�
	pns->sendToNetworkLayer(RECEIVER, *pPacketDel);
	expectSequenceNumberSend++;
	// �п��ܻ����nowlMax == rMax����expectSequenceNumberSend == rMax�����
	if (nowlMax == rMax && expectSequenceNumberSend == nowlMax) {

	}
	else
		expectSequenceNumberSend %= rMax;
	// ������
	if (base < nowlMax) {	// ��������û�зֶ�
		// �ڴ������û�г������ڴ�С
		if (expectSequenceNumberSend == nowlMax) // �ӵ�����
			waitingState = true;
	}
	else {
		// ��һ����ű�baseС���ҵ��˱߽�
		if (expectSequenceNumberSend < base && expectSequenceNumberSend >= nowlMax)
			waitingState = true;
	}
	// �����ǰ��������
	cout << "���ͷ� ���� ԭʼ���ĺ� �������ڣ�";
	cout << "| >";
	for (int i = base; i <= rMax; i++) {
		// �����ڷֶ����
		if (nowlMax < base && i == rMax)
			i = 0;
		if (i == nowlMax) {
			// ������
			if (expectSequenceNumberSend == nowlMax)
				cout << " <";
			cout << " |" << endl;
			break;
		}
		if (i == expectSequenceNumberSend)
			cout << " <";
		cout << " " << i;
	}
	//��������������ļ���
	cout.rdbuf(foutGBN.rdbuf());	// �����������Ϊ����GBNЭ�鴰�ڵ��ļ�
	cout << "���ͷ� ���� ԭʼ���ĺ� �������ڣ�";
	// �����ǰ��������
	cout << "| >";
	for (int i = base; i <= rMax; i++) {
		// �����ڷֶ����
		if (nowlMax < base && i == rMax)
			i = 0;
		if (i == nowlMax) {
			// ������
			if (expectSequenceNumberSend == nowlMax)
				cout << " <";
			cout << " |" << endl;
			break;
		}
		if (i == expectSequenceNumberSend)
			cout << " <";
		cout << " " << i;
	}
	cout.rdbuf(backup);	// ���������Ϊ����̨

	if (DebugSign) {
		system("pause");
		cout << endl;
		cout << endl;
		cout << endl;
	}
	return true;
}

void GBNRdtSender::receive(Packet &ackPkt) {
	// ����б���δȷ��
	if (expectSequenceNumberSend != base) {
		bool inWindow = false;	// ���ڱ�־ȷ�Ϻ��Ƿ��ڴ�����
		bool reSend = false;	// ���ڱ�־�Ƿ���Ҫ�ط�����
		int ackPktNum = ackPkt.acknum;	// ��ǰ���ĵ�ȷ�Ϻ�
		int checkSum = pUtils->calculateCheckSum(ackPkt);
		// ���ڷֶ�
		if (nowlMax < base) {
			if (expectSequenceNumberSend < base) {
				if (ackPktNum >= base && ackPktNum < rMax)
					inWindow = true;
				else if (ackPktNum < base && ackPktNum >= 0 && ackPktNum < expectSequenceNumberSend)
					inWindow = true;
			}
			else {
				if (ackPktNum >= base && ackPktNum < expectSequenceNumberSend)
					inWindow = true;
			}
		}
		else {

			if (ackPktNum >= base && ackPktNum < expectSequenceNumberSend)
				inWindow = true;
		}
		// �����ȷ�Ϻ��ڴ�����
		if (inWindow) {
			// �����ۼ�ȷ�ϵķ�ʽ
			// У�����ȷ
			if (checkSum == ackPkt.checksum) {
				pns->stopTimer(SENDER, pPacketWaitingAck[base].seqnum);
				pUtils->printPacket("���ͷ� �յ� ȷ�ϱ��� ������ȷ", ackPkt);
				// �����ƶ�
				base = ackPktNum + 1;
				base %= rMax;
				nowlMax = base + windowN;
				if (nowlMax == rMax) {
					/*�պõ����ұ߽���������²��ֶδ���*/
				}
				else {
					// nowlMaxȡֵ��Χ1~rMax
					nowlMax %= rMax;
					// ������֮ǰnowlMax == rMax����expectSequenceNumberSend == nowlMax�����
					expectSequenceNumberSend %= rMax;
				}
				// ���ڲ�Ϊ�գ�������ʱ��
				if (base != expectSequenceNumberSend)
					pns->startTimer(SENDER, Configuration::TIME_OUT, pPacketWaitingAck[base].seqnum);
				// ����δ��
				waitingState = false;
			}
			else {
				pUtils->printPacket("���ͷ� �յ� ȷ�ϱ��� У��ʹ���", ackPkt);
				reSend = true;	// ��Ҫ�ط�����
			}
		}
		else { // ackû������
			// ����ackPktNum ��baseС1�����
			if ((base + rMax - 1) % rMax == ackPktNum)
				cout << "���ͷ� �յ� ȷ�ϱ��� " << "ȷ�Ϻ�" << ackPktNum << " �����ش�����" << base;
			else {
				cout << "���ͷ� �յ� ȷ�ϱ��� û�и�ȷ�Ϻ�";
				reSend = true;
			}
			pUtils->printPacket("", ackPkt);
		}
		if (reSend) {
			// cout << endl << "!!!" << endl<<"!!!" << endl << "!!!" << endl;
			// system("pause");
			cout << "���ͷ� �ط� ���������ģ�" << endl;
			pns->stopTimer(SENDER, pPacketWaitingAck[base].seqnum);
			cout << endl;
			pns->startTimer(SENDER, Configuration::TIME_OUT, pPacketWaitingAck[base].seqnum);
			for (int i = base; i <= rMax; i++) {
				if (nowlMax < base && i == rMax)
					i = 0;
				if (i == expectSequenceNumberSend)
					break;
				cout << "���ͷ� �ط� ����" << i;
				pUtils->printPacket("", pPacketWaitingAck[i]);
				pns->sendToNetworkLayer(RECEIVER, pPacketWaitingAck[i]);
			}
		}
		// �����ǰ��������
		cout << "���ͷ� ���� ��Ӧ���ĺ� �������ڣ�";
		cout << "| >";
		for (int i = base; i <= rMax; i++) {
			// �����ڷֶ����
			if (nowlMax < base && i == rMax)
				i = 0;
			if (i == nowlMax) {
				// ������
				if (expectSequenceNumberSend == nowlMax)
					cout << " <";
				cout << " |" << endl;
				break;
			}
			if (i == expectSequenceNumberSend)
				cout << " <";
			cout << " " << i;
		}
		//��������������ļ���
		cout.rdbuf(foutGBN.rdbuf());	// �����������Ϊ����GBNЭ�鴰�ڵ��ļ�
		cout << "���ͷ� ���� ��Ӧ���ĺ� �������ڣ�";
		cout << "| >";
		for (int i = base; i <= rMax; i++) {
			// �����ڷֶ����
			if (nowlMax < base && i == rMax)
				i = 0;
			if (i == nowlMax) {
				// ������
				if (expectSequenceNumberSend == nowlMax)
					cout << " <";
				cout << " |" << endl;
				break;
			}
			if (i == expectSequenceNumberSend)
				cout << " <";
			cout << " " << i;
		}
		cout.rdbuf(backup);
	}
	else {
		cout << "���ͷ� �յ� ȷ�ϱ��� ����Ϊ�� ������";
		pUtils->printPacket("", ackPkt);
	}
	if (DebugSign) {
		system("pause");
		cout << endl;
		cout << endl;
		cout << endl;
	}
}

void GBNRdtSender::timeoutHandler(int seqNum) {
	cout << "���ͷ� ��ʱ�� ��ʱ��" << endl;
	cout << "���ͷ� �ط� ���������ģ�" << endl;
	pns->stopTimer(SENDER, pPacketWaitingAck[base].seqnum);
	cout << endl;
	pns->startTimer(SENDER, Configuration::TIME_OUT, pPacketWaitingAck[base].seqnum);
	for (int i = base; i <= rMax; i++) {
		if (nowlMax < base && i == rMax)
			i = 0;
		if (i == expectSequenceNumberSend)
			break;
		cout << "���ͷ� �ط� ����" << i;
		pUtils->printPacket("", pPacketWaitingAck[i]);
		pns->sendToNetworkLayer(RECEIVER, pPacketWaitingAck[i]);
	}
	if (DebugSign) {
		system("pause");
		cout << endl;
		cout << endl;
		cout << endl;
	}
}