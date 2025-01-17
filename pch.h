#ifndef PCH_H
#define PCH_H

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <fstream>
#include <windows.h>

// TODO: 在此处引用程序需要的其他头文件
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning(disable:4996)
#pragma warning(disable:4482)
#pragma comment (lib,"C:\\Users\\caxus\\Desktop\\C++\\netsimlib.lib")

#include <iostream>
using namespace std;

// 定义是否开启调试模式
extern bool DebugSign;

// 用于定义输出流
extern streambuf *backup;
extern ofstream foutGBN;
extern ofstream foutSR;
extern ofstream foutSRsender;
extern ofstream foutSRreceiver;
extern ofstream foutTCP;

// 用于定义缓存描述字符
extern char signY;
extern char signN;

#endif //PCH_H
