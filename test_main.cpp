#include "spslib.h"
#include <iostream>

using namespace SPS;
using namespace std;
typedef Message<50> TestMessage;
//typical port
struct MockPort {
	bool available() { return true; }//there are chars to read
	uchar_t read() { return 0x34; }
	void send(uchar_t a) {}
};
MockPort port;

//port handle example
void handle_com_port()
{
	static TestMessage::MsgReader reader;
	char buffer[100];
	int vector[10];
	while (port.available()) {
		if (reader.add_uchar(port.read())) {
			auto m = reader.getMessage();
			switch (m.id) {
			case 1: {
				//message 2 interpretation, get values
				float value = m.read<float>();
				m.read_cstring(buffer, 100);
				m.read_array<int>(vector, 5);
			}break;
			case 2:
				//message 2 interpretation
				//...
				break;
			default:
				//unknown message
				;
			}
		}

	}
}

//typical message construction and transmision
void send_Example() {
	TestMessage test(10); //message of id 10, info = string + int
	test.write_cstring("just an example");
	test.write<int>(1973);
	
	//when finished the message could be sent in this way
	for (int i = 0; i < test.datagram_size(); i++)port.send(test[i]);
}

void main()
{
	//test an use examples
	TestMessage::MsgReader reader;
	TestMessage test(10);
	cout << hex;
	test.write_cstring("ABCDEF ABCDEF");
	test.write<int>(1973);
	for (int i = 0; i < test.datagram_size(); i++)cout << +test[i]<<" ";
	cout << endl;
	uchar_t data[]{ 64, 34,22,3, 0xa3, 0x3a, 0x0f, 0xcc, 0x05, 0x0a, 0xb5, 0x07, 0x00, 0x00 };
	for (auto a : data) {
		if (reader.add_uchar(a)) {
			cout << "Mensaje Leido Correctamente";
			auto m = reader.getMessage();
			cout << "ID: " << +m.id << "   SIZE:" << +m.size << " ";
			cout << dec << "DATO(int):"<<m.read<int>();

		}
		else cout << "agregado: " << +a << endl;
	}

	TestMessage test2(1);
	char buffer[100];
	test2.write_cstring("Miguel Hernando");
	test2.write<int>(194453);
	test2.write<float>(71.675F);
	test2.write_cstring("Madrid");

	for (int i = 0; i < test2.datagram_size(); i++) {
		if (reader.add_uchar(test2.data[i])) {
			cout << "Mensaje Leido Correctamente";
			auto m = reader.getMessage();
			cout << dec << "ID: " << +m.id << "   SIZE:" << +m.size << endl;
			cout << m.read_cstring(buffer, 100) << endl;
			cout << m.read<int>() <<endl;
			cout << m.read<float>() << endl;
			cout << m.read_cstring(buffer, 100) << endl;
		}
		else cout << "agregado: " << test2.data[i] << endl;
	}


}