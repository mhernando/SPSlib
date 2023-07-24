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

# Typical ussage



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