import struct

SPS_HEADER = b'\x3A\xA3'
c_formats = {'char':'c','uint8_t':'B',
             'int16_t':'<h','uint16_t':'<H',
             'int32_t':'<i','int':'<i', 'uint32_t':'<I',
             'float':'<f', 'double':'<d'
             }
class Message:
    def __init__(self, id):
        self.data = bytearray(SPS_HEADER+b'\x00\x00\x01')
        self.data.append(id)
        self.write_crc()
    def __str__(self):
        l=[f'{a:02X}' for a in self.data]
        return ' '.join(l)
    def literal(self):
        l=[f'{chr(92)}x{a:02X}' for a in self.data]
        print(''.join(l))
    def write_crc(self):
        cr = (crc16(self.data[5:])).to_bytes(2,byteorder='little')
        self.data[2:4]=cr
        
    def check_crc(self):
        if self.data[2:4] == (crc16(self.data[5:])).to_bytes(2,byteorder='little'):
            return True
        return False
    def write(self, *args):
        for thing in args:
            if type(thing) is str:
                self.data.extend(thing.encode())
                self.data.append(0)
            elif type(thing) is int:
                self.data.extend(struct.pack("i", thing))
            elif type(thing) is float:
                self.data.extend(struct.pack("f", thing))
            elif type(thing) is list:
                for x in thing:
                    self.writeData(x)
        self.data[4] = len(self.data)-5
        self.write_crc()
    def write_f(self, format, item):
        if format in c_formats:
            self.data.extend(struct.pack(c_formats[format], item))
        else: self.data.extend(struct.pack(format, item))
        self.data[4] = len(self.data)-5
        self.write_crc()
        
    


def init_crc16_tab():
    crc_tab16 = [0] * 256
    for i in range(256):
        crc = 0
        c = i
        for j in range(8):
            if (crc ^ c) & 0x0001:
                crc = (crc >> 1) ^ 0xA001  # CRC_POLY_16
            else:
                crc = crc >> 1
            c = c >> 1
        crc_tab16[i] = crc
    return crc_tab16

def crc16(data):
    global crc_tab16
    if 'crc_tab16' not in globals(): crc_tab16 = init_crc16_tab()
    crc = 0x0000  # CRC_START_16
    for a in data:
        crc = (crc >> 8) ^ crc_tab16[(crc ^ (0x00ff & a)) & 0xff] 
    return crc


if __name__ == "__main__":
    mens = b'\xa3\x3a\x0f\xcc\x05\x0a\xb5\x07\x00\x00'
    cr = (crc16(mens[5:])).to_bytes(2,byteorder='little')
    
    print(f'{cr[0]:02X} {cr[1]:02X}')
    
    m = Message(10)
    
    'a3 3a b4 69 20 1 4d 69 67 75 65 6c 20 48 65 72 6e 61 6e 64 6f 0 95 f7 2 0 9a 59 8f 42 4d 61 64 72 69 64 0'
    m2 = Message(1)
    m2.write('Miguel Hernando', 194453, 71.675, 'Madrid')
