#
# mac_payload: fhdr(7..23) fport(1) frm_payload(0..N)
#
from MalformedPacketException import MalformedPacketException
from FHDR import FHDR
from MHDR import MHDR
from JoinRequestPayload import JoinRequestPayload
from JoinAcceptPayload import JoinAcceptPayload
from DataPayload import DataPayload

class MacPayload:

    def read(self, mtype, mac_payload):
        if len(mac_payload) < 1:
            raise MalformedPacketException("Invalid mac payload")

        self.fhdr = FHDR()
        self.fhdr.read(mac_payload)
        self.fport = mac_payload[self.fhdr.length()]
        self.frm_payload = None
        if mtype == MHDR.JOIN_REQUEST:
            self.frm_payload = JoinRequestPayload()
            self.frm_payload.read(mac_payload[self.fhdr.length() + 1:])
        if mtype == MHDR.JOIN_ACCEPT:
            self.frm_payload = JoinAcceptPayload()
            self.frm_payload.read(mac_payload[self.fhdr.length() + 1:])
        if mtype == MHDR.UNCONF_DATA_UP or mtype == MHDR.UNCONF_DATA_DOWN or\
                mtype == MHDR.CONF_DATA_UP or mtype == MHDR.CONF_DATA_DOWN:
            self.frm_payload = DataPayload()
            self.frm_payload.read(self, mac_payload[self.fhdr.length() + 1:])

    def create(self, mtype, key, args):
        self.fhdr = FHDR()
        self.fhdr.create(mtype, args)
        self.fport = 0x00
        self.frm_payload = None
        if mtype == MHDR.JOIN_REQUEST:
            self.frm_payload = JoinRequestPayload()
            self.frm_payload.create(args)
        if mtype == MHDR.JOIN_ACCEPT:
            self.frm_payload = JoinAcceptPayload()
            self.frm_payload.create(args)
        if mtype == MHDR.UNCONF_DATA_UP or mtype == MHDR.UNCONF_DATA_DOWN or\
                mtype == MHDR.CONF_DATA_UP or mtype == MHDR.CONF_DATA_DOWN:
            self.frm_payload = DataPayload()
            self.frm_payload.create(self, key, args)

    def length(self):
        return len(self.to_raw())

    def to_raw(self):
        mac_payload = []
        if self.fhdr.get_devaddr() != [0x00, 0x00, 0x00, 0x00]:
            mac_payload += self.fhdr.to_raw()
        if self.frm_payload != None:
            if self.fhdr.get_devaddr() != [0x00, 0x00, 0x00, 0x00]:
                mac_payload += [self.fport]
            mac_payload += self.frm_payload.to_raw()
        return mac_payload

    def get_fhdr(self):
        return self.fhdr

    def set_fhdr(self, fhdr):
        self.fhdr = fhdr

    def get_fport(self):
        return self.fport

    def set_fport(self, fport):
        self.fport = fport

    def get_frm_payload(self):
        return self.frm_payload

    def set_frm_payload(self, frm_payload):
        self.frm_payload = frm_payload
