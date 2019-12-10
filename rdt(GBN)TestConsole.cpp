#include "pch.h"
#include "DataStructure.h"
#include "Global.h"
#include "RdtSender.h"
#include "RdtReceiver.h"
#include "GBNRdtSender.h"
#include "GBNRdtReceiver.h"

bool DebugSign = false;				// 全局变量pch.h中声明，用于控制调试模式的开关
streambuf *backup = cout.rdbuf();	// 全局变量pch.h中声明，用于保存标准输出流
ofstream foutGBN;					// 全局变量pch.h中声明，GBN协议窗口输出流重定向到文件

char signY = 'X';					// 全局变量pch.h中声明，用于SR协议描述该位置缓存被占用
char signN = ' ';					// 全局变量pch.h中声明，用于SR协议描述该位置缓存未被占用

int main(void) {
	char inputFileName[100] = "C:\\Users\\caxus\\Desktop\\input.txt";						// 需要传输的数据文件名
	char outputFileNameGBN[100] = "C:\\Users\\caxus\\Desktop\\output.txt";				// 用该文件保存GBN协议收到的数据
	char outputWindowGBN[100] = "C:\\Users\\caxus\\Desktop\\outputWindow.txt";			// 用该文件保存GBN协议窗口

	RdtSender *ps = new GBNRdtSender();
	RdtReceiver *pr = new GBNRdtReceiver();

	pns->init();
	pns->setRtdSender(ps);
	pns->setRtdReceiver(pr);
	pns->setInputFile(inputFileName);
	pns->setOutputFile(outputFileNameGBN);
	foutGBN.open(outputWindowGBN);
	pns->start();

	foutGBN.close();
	system("pause");
	delete ps;
	delete pr;
	delete pUtils;									//指向唯一的工具类实例，只在main函数结束前delete
	delete pns;									//指向唯一的模拟网络环境类实例，只在main函数结束前delete

	cout << "    发送方发送文件：" << inputFileName << endl;
	cout << "    接收方接收文件：" << outputFileNameGBN << endl;
	cout << endl;
	cout << "发送方窗口描述文件：" << outputWindowGBN << endl;

	return 0;
}