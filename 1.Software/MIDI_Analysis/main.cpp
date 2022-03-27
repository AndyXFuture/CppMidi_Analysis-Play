#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <Windows.h>
#include <vector>
#pragma comment(lib,"winmm.lib")
using namespace std;
using namespace chrono;
fstream	MIDI;
fstream fout;
uint8_t	MThd[4] = { 'M','T','h','d' };
uint8_t MTrk[4] = { 'M','T','r','k' };
uint8_t buf[256];

uint16_t TrackNumber = 0;	//音轨数量
uint16_t TickUnit = 0;
uint32_t QuarterNoteLength = 0x06D511;//一个四分音符的时长(单位微秒)
uint32_t TickTime;

int len;
void outHex();
void outAscii();
void usleep(int len);
void uusleep(int len);
void uuusleep(int len);
HMIDIOUT HMidiOut;

int main() {
	midiOutOpen(&HMidiOut, 0, 0, 0, CALLBACK_NULL);

	fout.open("./result.txt", ios::in | ios::out);
	MIDI.open("./Unravel.mid", ios::in | ios::out | ios::binary);

	//头
	memset(buf, '\0', sizeof(buf));
	MIDI.read((char*)&buf, len = 4);//头
	if (buf[0] == MThd[0] && buf[1] == MThd[1] && buf[2] == MThd[2] && buf[3] == MThd[3]) {
		cout << "确认为MIDI文件" << endl;
		fout << "确认为MIDI文件" << endl;
	}else {
		cout << "不是MIDI文件" << endl;
		fout << "不是MIDI文件" << endl;
		return 1;
	}
	outHex();

	MIDI.read((char*)&buf, len = 4);//长度
	uint32_t lenght = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
	outHex();
	MIDI.read((char*)&buf, len = lenght);//内容
	//读取文件内容判断谱信息
	//0、1位为音轨信息
	//2、3位为指定轨道数
	//4、5位为基本时间单位
	TickUnit = (buf[4] << 8) + buf[5];
	outHex();


	MIDI.read((char*)&buf, len = 4);//音轨块
	outHex();
	MIDI.read((char*)&buf, len = 4);//数据长度
	lenght = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
	outHex();


	MIDI.read((char*)&buf, len = lenght);//数据

	//读取文件头信息
	for(int i = 0; i<(int)lenght;i++){
	//for (int i : buf) {
		if (buf[i] == 0xFF) {
			//cout << "================" << i << endl;
			uint8_t data[256] = {0};
			switch(buf[i + 1]) {
			case 0x03:
				memcpy(&data, &buf[i + 3], buf[i + 2]*sizeof(uint8_t));
				cout << "曲目名字: "<< data << endl;
				fout << "曲目名字: "<< data << endl;
				break;
			case 0x01:
				memcpy(&data, &buf[i + 3], buf[i + 2] * sizeof(uint8_t));
				cout << "作者信息: " << data << endl;
				fout << "作者信息: " << data << endl;
				break;
			case 0x02:
				memcpy(&data, &buf[i + 3], buf[i + 2] * sizeof(uint8_t));
				cout << "版权信息: " << data << endl;
				fout << "版权信息: " << data << endl;
				break;
			case 0x51:
				memcpy(&data, &buf[i + 3], buf[i + 2] * sizeof(uint8_t));
				QuarterNoteLength = (data[0] << 16) + (data[1] << 8) + data[2];
				TickTime = QuarterNoteLength / TickUnit;
				cout << "TickTime:" << dec << (int)TickTime << endl;
				fout << "TickTime:" << dec << (int)TickTime << endl;
				break;
			case 0x2F:
				goto exit;
			}
		}
	}
exit:
	outHex();

	fout << endl << endl;
	cout << endl << endl;

	//MTrk
	MIDI.read((char*)&buf, len = 4);
	if (buf[0] == MTrk[0] && buf[1] == MTrk[1] && buf[2] == MTrk[2] && buf[3] == MTrk[3]) {
		cout << "进入MTrk块" << endl;
		fout << "进入MTrk块" << endl;
	}
	else {
		cout << "读取文件错误" << endl;
		fout << "读取文件错误" << endl;
		return 1;
	}
	outHex();
	MIDI.read((char*)&buf, len = 4);//数据长度
	lenght = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
	cout << "数据长度"  << lenght << endl;
	fout << "数据长度"  << lenght << endl;
	outHex();


	uint16_t DeltaTime = 0;
	uint8_t Event = 0;
	uint8_t Param1 = 0;
	uint8_t Param2 = 0;
	bool SleepFlag = false;

	memset(buf, '\0', sizeof(buf));
	//主菜
	for (int i = 0; i < (int)lenght; i++) {
		cout << i << " ";

		//判断延时+读取事件类型
		MIDI.read((char*)&buf, len = 1);
		if (buf[0] >= 0x80) {
			DeltaTime = (buf[0] - 0x80) << 8;
			MIDI.read((char*)&buf, len = 1);
			DeltaTime += buf[0];
			SleepFlag = true;
		}
		else if (buf[0] != 0) {
			DeltaTime = buf[0];
			SleepFlag = true;
		}
		else {
			DeltaTime = buf[0];
		}


		//判断MIDI事件类型
		MIDI.read((char*)&buf, len = 1);
		Event = buf[0];
		if ((Event & 0xF0) == 0xC0 || (Event & 0xF0) == 0xD0) {
			MIDI.read((char*)&buf, len = 1);
			Param1 = buf[0];
			Param2 = 0;
		} else {
			MIDI.read((char*)&buf, len = 2);
			Param1 = buf[0];
			Param2 = buf[1];
		}



		if (Event == 0xFF) {
			if (Param1 == 0x2F) {
				break;
			}
			
			cout << "事件:\t" << setfill('0') << setw(2) << (int)Event << "\t";
			fout << "事件:\t" << setfill('0') << setw(2) << (int)Event << "\t";
			cout << "类型:\t" << setfill('0') << setw(2) << (int)Param1 << "\t";
			fout << "类型:\t" << setfill('0') << setw(2) << (int)Param1 << "\t";
			cout << "长度:\t" << setfill('0') << setw(2) << (int)Param2 << "\t";
			fout << "长度:\t" << setfill('0') << setw(2) << (int)Param2 << "\t";
			
			MIDI.read((char*)&buf, len = Param2);
			outAscii();
			continue;
		}

		if (SleepFlag == true) {//有延时就延时
			cout << "延时:\t" << setfill('0') << setw(4) << (int)DeltaTime << "\t";
			fout << "延时:\t" << setfill('0') << setw(4) << (int)DeltaTime << "\t";
			/*
			if (DeltaTime > 350) {
				DeltaTime -= 350;
			}
			else {
				DeltaTime = 0;
			}*/
			uusleep(DeltaTime * TickTime*55/100);	//可能是其他部分会占一定的时间或者延时函数不准确对此乘0.55修正
			DeltaTime = 0;
			SleepFlag = false;
		}
		else {
			cout << "延时:\t" << setfill('0') << setw(4) << (int)DeltaTime << "\t";
			fout << "延时:\t" << setfill('0') << setw(4) << (int)DeltaTime << "\t";
		}



		//处理事件
		midiOutShortMsg(HMidiOut, (Param2 << 16) + (Param1 << 8) + Event);

		cout << "事件:\t" << setfill('0') << setw(2) << (int)Event << "\t";
		fout << "事件:\t" << setfill('0') << setw(2) << (int)Event << "\t";
		cout << "音符:\t" << setfill('0') << setw(2) << (int)Param1 << "\t";
		fout << "音符:\t" << setfill('0') << setw(2) << (int)Param1 << "\t";
		cout << "力度:\t" << setfill('0') << setw(2) << (int)Param2 << endl;
		fout << "力度:\t" << setfill('0') << setw(2) << (int)Param2 << endl;


		memset(buf, '\0', sizeof(buf));
	}

	//usleep(10000000);
	midiOutClose(HMidiOut);
	cout << "over" << endl;
	fout << "over" << endl;

	return 0;
}

void outHex() {
	fout.setf(ios_base::hex, ios_base::basefield);
	fout.setf(ios::uppercase);
	cout.setf(ios_base::hex, ios_base::basefield);
	cout.setf(ios::uppercase);
	for (int i = 0; i < len; i++) {
		cout << setfill('0') << setw(2) << (int)buf[i] << " ";
		fout << setfill('0') << setw(2) << (int)buf[i] << " ";
	}
	memset(buf, '\0', sizeof(buf));
	cout << endl;
	cout << std::defaultfloat;
	fout << std::defaultfloat;
}

void outAscii() {
	for (int i = 0; i < len; i++) {
		cout << buf[i];
		fout << buf[i];
	}
	memset(buf, '\0', sizeof(buf));
	cout << endl;
	fout << endl;
}

//这个会休眠线程导致需要额外时间取得CPU使用权
void usleep(int len) {
	std::this_thread::sleep_for(std::chrono::microseconds(len));
}

void uusleep(int len) {
	steady_clock::time_point time_begin = steady_clock::now();
	//SleepSelectUS(s, Interval_Microseconds);
	while (1) {
		steady_clock::time_point time_end = steady_clock::now();
		//char tmp[128] = { 0 };
		uint64_t used = duration_cast<microseconds>(time_end - time_begin).count();
		if (used > len) {
			return;
		}
	}
	return;
}
/*
void uuusleep(int len) {
	struct timeval tv;
	fd_set dummy;
	FD_ZERO(&dummy);
	FD_SET(s, &dummy);
	tv.tv_sec = len / 1000000L;
	tv.tv_usec = len % 1000000L;
	select(0, 0, 0, &dummy, &tv);
	DWORD err = GetLastError();
	if (err != 0)
		printf("Error : %d", err);
	return;
}
*/

void uuusleep(int len)
{
	LARGE_INTEGER perfCnt, start, now;

	QueryPerformanceFrequency(&perfCnt);
	QueryPerformanceCounter(&start);

	do {
		QueryPerformanceCounter((LARGE_INTEGER*)&now);
	} while ((now.QuadPart - start.QuadPart) / float(perfCnt.QuadPart) * 1000 * 1000 < (DWORD)len);
}