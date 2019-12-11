#include "pch.h"
#include "Global.h"
#include "GBNRdtSender.h"

GBNRdtSender::GBNRdtSender(Configuration config) {
	expectSequenceNumberSend = 0;			// 发送序号，初始为0最大为rmax-1
	waitingState = false;									// 是否处于等待Ack的状态
	pPacketWaitingAck = new Packet[config.SEQNUM_MAX];		//全部待发送分组
	windowN = config.WINDOW_N;				// 滑动窗口大小
	base = 0;													// 当前窗口的序号的基址

	rMax = config.SEQNUM_MAX;					// 窗口序号上界，即最大序号为rMax-1（从0开始）
	nowlMax = windowN;	 							// 滑动窗口右边界，最大为rMax，最小为1
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
	if (waitingState)	//发送方处于等待确认状态
		return false;
	//初始化当前处理的分组（编号从0开始）
	Packet *pPacketDel = &pPacketWaitingAck[expectSequenceNumberSend];
	//seq下一个发送的序号，初始为0，下一序号从1开始
	pPacketDel->seqnum = expectSequenceNumberSend;
	//ack确认号
	pPacketDel->acknum = -1;	// 忽略该字段
	//chksum校验和
	pPacketDel->checksum = 0;

	//将信息装入packet，计算校验和
	memcpy(pPacketDel->payload, message.data, sizeof(message.data));
	pPacketDel->checksum = pUtils->calculateCheckSum(*pPacketDel);
	pUtils->printPacket("发送方 发送 原始报文", *pPacketDel);
	
	//窗口的第一个发送报文需要开启定时器，该计时器的编号是base
	if (expectSequenceNumberSend == base) {
		pns->startTimer(SENDER, Configuration::TIME_OUT, base);
	}
	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
	pns->sendToNetworkLayer(RECEIVER, *pPacketDel);
	expectSequenceNumberSend++;

	//处理窗口信息
	//窗口到达最右端且正发送窗口末尾分组
	if (nowlMax == rMax && expectSequenceNumberSend == nowlMax) {
	}
	else
		expectSequenceNumberSend %= rMax;	//正常情况下不变，窗口末尾归0

	//处理窗口
	if (base < nowlMax) {	// 滑动窗口没有分段
		//期待的序号没有超出窗口大小
		if (expectSequenceNumberSend == nowlMax) // 加到上限
			waitingState = true;
	}
	else {
		//下一个序号比base小，且到了边界
		if (expectSequenceNumberSend < base && expectSequenceNumberSend >= nowlMax)
			waitingState = true;
	}

	//控制台输出当前滑动窗口
	cout << "发送方 发送 原始报文后 滑动窗口：";
	cout << "| >";
	for (int i = base; i <= rMax; i++) {
		// 处理窗口分段情况
		if (nowlMax < base && i == rMax)
			i = 0;
		if (i == nowlMax) {
			// 窗口满
			if (expectSequenceNumberSend == nowlMax)
				cout << " <";
			cout << " |" << endl;
			break;
		}
		if (i == expectSequenceNumberSend)
			cout << " <";
		cout << " " << i;
	}

	//滑动窗口输出到文件中
	cout.rdbuf(foutGBN.rdbuf());	// 将输出流设置为保存GBN协议窗口的文件
	cout << "发送方 发送 原始报文后 滑动窗口：";
	cout << "| >";
	for (int i = base; i <= rMax; i++) {
		// 处理窗口分段情况
		if (nowlMax < base && i == rMax)
			i = 0;
		if (i == nowlMax) {
			// 窗口满
			if (expectSequenceNumberSend == nowlMax)
				cout << " <";
			cout << " |" << endl;
			break;
		}
		if (i == expectSequenceNumberSend)
			cout << " <";
		cout << " " << i;
	}
	//？
	cout.rdbuf(backup);	// 将输出流设为控制台

	if (DebugSign) {
		system("pause");
		cout << endl;
		cout << endl;
		cout << endl;
	}
	return true;
}

void GBNRdtSender::receive(Packet &ackPkt) {
	// 如果有报文未确认
	if (expectSequenceNumberSend != base) {
		bool inWindow = false;	// 用于标志确认号是否在窗口内
		bool reSend = false;	// 用于标志是否需要重发报文
		int ackPktNum = ackPkt.acknum;	// 当前报文的确认号
		int checkSum = pUtils->calculateCheckSum(ackPkt);
		// 窗口分段
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
		// 如果该确认号在窗口中
		if (inWindow) {
			// 采用累计确认的方式
			// 校验和正确
			if (checkSum == ackPkt.checksum) {
				pns->stopTimer(SENDER, pPacketWaitingAck[base].seqnum);
				pUtils->printPacket("发送方 收到 确认报文 报文正确", ackPkt);
				// 窗口移动
				base = ackPktNum + 1;
				base %= rMax;
				nowlMax = base + windowN;
				if (nowlMax == rMax) {
					/*刚好等于右边界这种情况下不分段窗口*/
				}
				else {
					// nowlMax取值范围1~rMax
					nowlMax %= rMax;
					// 处理了之前nowlMax == rMax，且expectSequenceNumberSend == nowlMax的情况
					expectSequenceNumberSend %= rMax;
				}
				// 窗口不为空，重启定时器
				if (base != expectSequenceNumberSend)
					pns->startTimer(SENDER, Configuration::TIME_OUT, pPacketWaitingAck[base].seqnum);
				// 窗口未满
				waitingState = false;
			}
			else {
				pUtils->printPacket("发送方 收到 确认报文 校验和错误", ackPkt);
				reSend = true;	// 需要重发报文
			}
		}
		else { // ack没有命中
			// 处理ackPktNum 比base小1的情况
			if ((base + rMax - 1) % rMax == ackPktNum)
				cout << "发送方 收到 确认报文 " << "确认号" << ackPktNum << " 请求重传分组" << base;
			else {
				cout << "发送方 收到 确认报文 没有该确认号";
				reSend = true;
			}
			pUtils->printPacket("", ackPkt);
		}
		if (reSend) {
			// cout << endl << "!!!" << endl<<"!!!" << endl << "!!!" << endl;
			// system("pause");
			cout << "发送方 重发 缓冲区报文：" << endl;
			pns->stopTimer(SENDER, pPacketWaitingAck[base].seqnum);
			cout << endl;
			pns->startTimer(SENDER, Configuration::TIME_OUT, pPacketWaitingAck[base].seqnum);
			for (int i = base; i <= rMax; i++) {
				if (nowlMax < base && i == rMax)
					i = 0;
				if (i == expectSequenceNumberSend)
					break;
				cout << "发送方 重发 报文" << i;
				pUtils->printPacket("", pPacketWaitingAck[i]);
				pns->sendToNetworkLayer(RECEIVER, pPacketWaitingAck[i]);
			}
		}
		// 输出当前滑动窗口
		cout << "发送方 处理 响应报文后 滑动窗口：";
		cout << "| >";
		for (int i = base; i <= rMax; i++) {
			// 处理窗口分段情况
			if (nowlMax < base && i == rMax)
				i = 0;
			if (i == nowlMax) {
				// 窗口满
				if (expectSequenceNumberSend == nowlMax)
					cout << " <";
				cout << " |" << endl;
				break;
			}
			if (i == expectSequenceNumberSend)
				cout << " <";
			cout << " " << i;
		}
		//滑动窗口输出到文件中
		cout.rdbuf(foutGBN.rdbuf());	// 将输出流设置为保存GBN协议窗口的文件
		cout << "发送方 处理 响应报文后 滑动窗口：";
		cout << "| >";
		for (int i = base; i <= rMax; i++) {
			// 处理窗口分段情况
			if (nowlMax < base && i == rMax)
				i = 0;
			if (i == nowlMax) {
				// 窗口满
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
		cout << "发送方 收到 确认报文 窗口为空 不处理";
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
	cout << "发送方 定时器 超时：" << endl;
	cout << "发送方 重发 缓冲区报文：" << endl;
	pns->stopTimer(SENDER, pPacketWaitingAck[base].seqnum);
	cout << endl;
	//重启定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, pPacketWaitingAck[base].seqnum);
	//窗口超时，重发窗口全部已发内容直到expectSeq
	for (int i = base; i <= rMax; i++) {
		//若窗口跨界，回退到0   eg: | 0 1 2 < 3 4 5 6 > 7 | ,base=7,now=2 < 7 ----> when i == 8, goback to 0
		if (nowlMax < base && i == rMax)
			i = 0;
		//直到当前还未发送的报文
		if (i == expectSequenceNumberSend)
			break;
		cout << "发送方 重发 报文" << i;
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