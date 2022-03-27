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

uint16_t TrackNumber = 0;	//��������
uint16_t TickUnit = 0;
uint32_t QuarterNoteLength = 0x06D511;//һ���ķ�������ʱ��(��λ΢��)
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

	//ͷ
	memset(buf, '\0', sizeof(buf));
	MIDI.read((char*)&buf, len = 4);//ͷ
	if (buf[0] == MThd[0] && buf[1] == MThd[1] && buf[2] == MThd[2] && buf[3] == MThd[3]) {
		cout << "ȷ��ΪMIDI�ļ�" << endl;
		fout << "ȷ��ΪMIDI�ļ�" << endl;
	}else {
		cout << "����MIDI�ļ�" << endl;
		fout << "����MIDI�ļ�" << endl;
		return 1;
	}
	outHex();

	MIDI.read((char*)&buf, len = 4);//����
	uint32_t lenght = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
	outHex();
	MIDI.read((char*)&buf, len = lenght);//����
	//��ȡ�ļ������ж�����Ϣ
	//0��1λΪ������Ϣ
	//2��3λΪָ�������
	//4��5λΪ����ʱ�䵥λ
	TickUnit = (buf[4] << 8) + buf[5];
	outHex();


	MIDI.read((char*)&buf, len = 4);//�����
	outHex();
	MIDI.read((char*)&buf, len = 4);//���ݳ���
	lenght = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
	outHex();


	MIDI.read((char*)&buf, len = lenght);//����

	//��ȡ�ļ�ͷ��Ϣ
	for(int i = 0; i<(int)lenght;i++){
	//for (int i : buf) {
		if (buf[i] == 0xFF) {
			//cout << "================" << i << endl;
			uint8_t data[256] = {0};
			switch(buf[i + 1]) {
			case 0x03:
				memcpy(&data, &buf[i + 3], buf[i + 2]*sizeof(uint8_t));
				cout << "��Ŀ����: "<< data << endl;
				fout << "��Ŀ����: "<< data << endl;
				break;
			case 0x01:
				memcpy(&data, &buf[i + 3], buf[i + 2] * sizeof(uint8_t));
				cout << "������Ϣ: " << data << endl;
				fout << "������Ϣ: " << data << endl;
				break;
			case 0x02:
				memcpy(&data, &buf[i + 3], buf[i + 2] * sizeof(uint8_t));
				cout << "��Ȩ��Ϣ: " << data << endl;
				fout << "��Ȩ��Ϣ: " << data << endl;
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
		cout << "����MTrk��" << endl;
		fout << "����MTrk��" << endl;
	}
	else {
		cout << "��ȡ�ļ�����" << endl;
		fout << "��ȡ�ļ�����" << endl;
		return 1;
	}
	outHex();
	MIDI.read((char*)&buf, len = 4);//���ݳ���
	lenght = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
	cout << "���ݳ���"  << lenght << endl;
	fout << "���ݳ���"  << lenght << endl;
	outHex();


	uint16_t DeltaTime = 0;
	uint8_t Event = 0;
	uint8_t Param1 = 0;
	uint8_t Param2 = 0;
	bool SleepFlag = false;

	memset(buf, '\0', sizeof(buf));
	//����
	for (int i = 0; i < (int)lenght; i++) {
		cout << i << " ";

		//�ж���ʱ+��ȡ�¼�����
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


		//�ж�MIDI�¼�����
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
			
			cout << "�¼�:\t" << setfill('0') << setw(2) << (int)Event << "\t";
			fout << "�¼�:\t" << setfill('0') << setw(2) << (int)Event << "\t";
			cout << "����:\t" << setfill('0') << setw(2) << (int)Param1 << "\t";
			fout << "����:\t" << setfill('0') << setw(2) << (int)Param1 << "\t";
			cout << "����:\t" << setfill('0') << setw(2) << (int)Param2 << "\t";
			fout << "����:\t" << setfill('0') << setw(2) << (int)Param2 << "\t";
			
			MIDI.read((char*)&buf, len = Param2);
			outAscii();
			continue;
		}

		if (SleepFlag == true) {//����ʱ����ʱ
			cout << "��ʱ:\t" << setfill('0') << setw(4) << (int)DeltaTime << "\t";
			fout << "��ʱ:\t" << setfill('0') << setw(4) << (int)DeltaTime << "\t";
			/*
			if (DeltaTime > 350) {
				DeltaTime -= 350;
			}
			else {
				DeltaTime = 0;
			}*/
			uusleep(DeltaTime * TickTime*55/100);	//�������������ֻ�ռһ����ʱ�������ʱ������׼ȷ�Դ˳�0.55����
			DeltaTime = 0;
			SleepFlag = false;
		}
		else {
			cout << "��ʱ:\t" << setfill('0') << setw(4) << (int)DeltaTime << "\t";
			fout << "��ʱ:\t" << setfill('0') << setw(4) << (int)DeltaTime << "\t";
		}



		//�����¼�
		midiOutShortMsg(HMidiOut, (Param2 << 16) + (Param1 << 8) + Event);

		cout << "�¼�:\t" << setfill('0') << setw(2) << (int)Event << "\t";
		fout << "�¼�:\t" << setfill('0') << setw(2) << (int)Event << "\t";
		cout << "����:\t" << setfill('0') << setw(2) << (int)Param1 << "\t";
		fout << "����:\t" << setfill('0') << setw(2) << (int)Param1 << "\t";
		cout << "����:\t" << setfill('0') << setw(2) << (int)Param2 << endl;
		fout << "����:\t" << setfill('0') << setw(2) << (int)Param2 << endl;


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

//����������̵߳�����Ҫ����ʱ��ȡ��CPUʹ��Ȩ
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