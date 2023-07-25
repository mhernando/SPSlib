# SPSlib -  Small Portable Serializer

SPS stands for Small Portable Serializer. It is a one file template lib designed for easy serialization and message interpretation.

After writing the similiar code in differente projects, and trying not to use other incredible but excessive complex solutions (protobuffer), I've designed this very simple header file lib. 



### Message structure

| byte |   meaning          |
|:----:|:------------------:|
|   0  |     HEADER1        |
|   1  |     HEADER2        |
|   2  |     CRC16 LS       |
|   3  |     CRC16 HS       |
|   4  |     SIZE           |
|   5  |     ID             |
|   6  |     INFO[0]        |
| ...  |     ...            |

## Typical ussage


To use the library, just include the single, self-contained library header file.

```C++
#include "spslib.h"
```

The namespace used is SPS, so in projects where it does not appear that there may be conflicts, for practicality, it is recommended to use :

```C++
using namespace SPS;
```

Those systems that are going to communicate between themselves must describe the same type of messages that are going to be implemented. To do this, the message template must be instantiated, where the two header bytes and the maximum payload size that messages can transmit (0-254) must be specified. Three different examples of usage are shown below. By default the header 0x3AA3 is used (by little endian encoding, the first byte will be 0xA3 and the second 0x3A), and the maximum size (254) is used.

```C++
typedef Message<50> TestMessage;
typedef Message<64, 0x344E> OtherTypeMessage;
typedef Message<> DefaultMessage; //max info:254 , 0x3AA3 header
```

Although messages are of variable size, the maximum space that messages can occupy is reserved in the read process and in the circular message buffer. This means that for systems where memory may be critical, the message should be set to the maximum possible size.

### Sending messages

The most generic port we can consider is the one that sends and receives single bytes. Messages are therefore sent byte by byte, and received byte by byte. The receiver will collect bytes in order to complete the original message and will then proceed to interpret its content as long as the transmission has been valid.
Therefore, the communication process is generally as follows:

1. **Message creation**. In order to send a message, what we do is to create a message in which we include a message type indicator. This is important so that later different interpretations can be made by the receiver.
```C++
TestMessage test(10); //message of id 10
```

2. **Aggregation of information**. 
Then, by means of the generic functions of write / write_cstring / write_array, we compose the message with the sequence of data corresponding to that type of message.
```C++
test.write_cstring("just an example");
test.write<int>(1973);
int mi_vector[3]{1,2,3};
test.write_array<int>(mi_vector, 3)
```
  In the example we have decided that de message of type 10 is composed by a c string, an integer and a vector of three integers. When this message is received, the information must be extracted in the same order. 
  
3. **Message transmission**. Finally, we proceed to send the complete message byte by byte. 
```C++
for (int i = 0; i < test.datagram_size(); i++)port.send(test[i]);
```
Note that datagram_size(), tells us the size of the complete message, with header, crc, size, id and info. While the message size field only encodes the size of the id + the number of bytes of the message data. 

### Receiving and interpreting messages
To receive messages you can simply make use of a reader that is created and included for each instantiation of the message template. 
The idea is simple. Each time a byte is read from the input port, this byte is passed to the reader. The reader will inform us if a message has been successfully completed thanks to the byte introduced. If so, the message must be picked up and interpreted before inserting more bytes.
```C++
//port handle example
static TestMessage::MsgReader reader; //the reader must be persistent
char buffer[100];
int vector[10];
while (port.available()) {
  if (reader.add_uchar(port.read())) {
    auto m = reader.getMessage();
    switch (m.id) {
      case 10: {
        //message of id 10 interpretation, get values
        m.read_cstring(buffer, 100);
        int value = m.read<int>();
        m.read_array<int>(vector, 3);
        //do something with that values...
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
```
Note that just as there are generic write functions, dual read functions have been implemented.

##Circular buffer
Circular Buffer creation. Generic class that specifies the buffer size as parameter. Default value is 100. 
```C++
	TestMessage::CircularBuffer<> mens_buffer;
	TestMessage::CircularBuffer<50> mens_buffer2;
```
It is possible to push messages. Last in overrides the older messages when buffer size is overflown. 
It is possible to define a single message policy. For example if you are transmiting the current value of a variable, probably you only want to proccess the last value. All the incomming messages sould overwrite the older ones that were not processed. 
By default, if using push_single, messages with the same id are overwritten.
```C++
	mens_buffer.push(test2);
	mens_buffer.push(test2);
	mens_buffer.push(test2);
	mens_buffer.push_single(TestMessage(1)); //this one should change the first appearance of 
	while (mens_buffer.there_is_msg()) {
		auto m = mens_buffer.getMessage();
		cout << "circular buffer: "<< m.id <<"-->"<<m.crc<< endl;
	}
 ```
A different policy could be  easily defined by a lamda or a normal function with the following format:

```C++
//conditions that makes two messages equal (same type of info for the same item)
//this is useful when the messages are precessed at lower rate in order to take only the last command.
bool compare_example(const TestMessage& m1, const TestMessage& m2) {
	if ((m1.id == m2.id) && (m1.size == m2.size))return true; //using size for simplicity
	return false;
}
 ```
and iclude it as the second function parameter:
```C++
mens_buffer.push_single(TestMessage(1), compare_example); //this one should change the first appearance of 
 ```

## some important tips

+ The library for simplicity has been decided to work only with little endian systems. Therefore it should not be used on other systems.

+ Since it is intended to communicate microcontrollers with each other, or with computers or mobile devices, it is important to make use of basic data types with common sizes across platforms. Therefore in the case of integers it is recommended to use ``` <cstdint> ```, with the ``` int16_t int32_t ``` and ``` int64_t ``` types. 

+ Likewise, in the case of structured data transmission, it is important to ensure packing of 1. Since each system will align and pack the aggregates differently otherwise. 



