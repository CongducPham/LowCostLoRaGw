#
# lorawan packet: mhdr(1) mac_payload(1..N) mic(4)
#
from MalformedPacketException import MalformedPacketException
from MHDR import MHDR
from Direction import Direction
from MacPayload import MacPayload

class PhyPayload:

    def __init__(self, key):
        self.key = key

    def read(self, packet):
        if len(packet) < 12:
            raise MalformedPacketException("Invalid lorawan packet");

        self.mhdr = MHDR(packet[0])
        self.set_direction()
        self.mac_payload = MacPayload()
        self.mac_payload.read(self.get_mhdr().get_mtype(), packet[1:-4])
        self.mic = packet[-4:]

    def create(self, mhdr, args):
        self.mhdr = MHDR(mhdr)
        self.set_direction()
        self.mac_payload = MacPayload()
        self.mac_payload.create(self.get_mhdr().get_mtype(), self.key, args)
        self.mic = None

    def length(self):
        return len(self.to_raw())

    def to_raw(self):
        phy_payload = [self.get_mhdr().to_raw()]
        phy_payload += self.mac_payload.to_raw()
        phy_payload += self.get_mic()
        return phy_payload

    def get_mhdr(self):
        return self.mhdr;

    def set_mhdr(self, mhdr):
        self.mhdr = mhdr

    def get_direction(self):
        return self.direction.get()

    def set_direction(self):
        self.direction = Direction(self.get_mhdr())

    def get_mac_payload(self):
        return self.mac_payload

    def set_mac_payload(self, mac_payload):
        self.mac_payload = mac_payload

    def get_mic(self):
        if self.mic == None:
            self.set_mic(self.compute_mic())
        return self.mic

    def set_mic(self, mic):
        self.mic = mic

    def compute_mic(self):
        return self.mac_payload.frm_payload.compute_mic(self.key, self.get_direction(), self.get_mhdr())

    def valid_mic(self):
        return self.get_mic() == self.mac_payload.frm_payload.compute_mic(self.key, self.get_direction(), self.get_mhdr())

    def get_payload(self):
        return self.mac_payload.frm_payload.decrypt_payload(self.key, self.get_direction())
