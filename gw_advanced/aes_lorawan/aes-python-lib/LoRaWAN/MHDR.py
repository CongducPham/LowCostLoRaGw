from MalformedPacketException import MalformedPacketException

class MHDR:

    LORAWAN_V1 = 0x00;

    MHDR_TYPE = 0xE0;
    MHDR_RFU = 0x1C;
    MHDR_MAJOR = 0x03;

    JOIN_REQUEST = 0x00
    JOIN_ACCEPT = 0x20
    UNCONF_DATA_UP = 0x40
    UNCONF_DATA_DOWN = 0x60
    CONF_DATA_UP = 0x80
    CONF_DATA_DOWN = 0xA0
    RFU = 0xC0 # rejoin for roaming
    PROPRIETARY = 0xE0

    def __init__(self, mhdr):
        self.mhdr = mhdr
        mversion = mhdr & self.MHDR_MAJOR
        if mversion != self.LORAWAN_V1:
            raise MalformedPacketException("Invalid major version")

    def to_raw(self):
        return self.mhdr

    def get_mversion(self):
        return self.mhdr & self.MHDR_MAJOR

    def get_mtype(self):
        return self.mhdr & self.MHDR_TYPE
