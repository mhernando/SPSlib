# SPSlib -  Small Portable Serializer

SPS stands for Small Portable Serializer. It is a one file template lib designed for easy serialization and message interpretation.

After writing the similiar code in differente projects, and trying not to use other incredible but excessive complex solutions (protobuffer), I've designed this very simple header file lib. 



# Message structure

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

typedef Message<50> TestMessage;
