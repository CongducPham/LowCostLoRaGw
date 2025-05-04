from MHDR import MHDR

class Direction:

    UP = 0x00
    DOWN = 0x01
    DIRECTION = {
        MHDR.JOIN_REQUEST: UP,
        MHDR.JOIN_ACCEPT: DOWN,
        MHDR.UNCONF_DATA_UP: UP,
        MHDR.UNCONF_DATA_DOWN: DOWN,
        MHDR.CONF_DATA_UP: UP,
        MHDR.CONF_DATA_DOWN: DOWN,
        MHDR.RFU: UP,
        MHDR.PROPRIETARY: UP }

    def __init__(self, mhdr):
        self.set(mhdr)

    def get(self):
        return self.direction

    def set(self, mhdr):
        self.direction = self.DIRECTION[mhdr.get_mtype()]
