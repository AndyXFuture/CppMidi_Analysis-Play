#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;
fstream	MIDI;
fstream file;
uint8_t buf[1];

int main() {
	file.open("./result.txt", ios::in | ios::out);
	MIDI.open("./Unravel.mid", ios::in | ios::out | ios::binary);

	cout.setf(ios_base::hex, ios_base::basefield);
	cout.setf(ios::uppercase);
	file.setf(ios_base::hex, ios_base::basefield);
	file.setf(ios::uppercase);

	while (!MIDI.eof()) {
		//while (MIDI.peek() != EOF) {
		MIDI.read((char*)&buf, 1);
		cout << setfill('0') << setw(2) << (int)buf[0] << " ";
		file << setfill('0') << setw(2) << (int)buf[0] << " ";

		//break;
	}
	MIDI.close();
	file.close();

	cout << endl;
	cout << "Done" << endl;

	return 0;
}