#
# frm_payload: appnonce(3) netid(3) devaddr(4) dlsettings(1) rxdelay(1) cflist(0..16)
#
from AES_CMAC import AES_CMAC
from Crypto.Cipher import AES

class JoinAcceptPayload:

    def __init__(self, payload):
        if len(payload) < 16:
            raise MalformedPacketException("Invalid join accept");
        self.encrypted_payload = payload
        self.payload = self.decrypt_payload(payload)

        self.appnonce = self.payload[:3]
        self.netid = self.payload[3:6]
        self.devaddr = self.payload[6:10]
        self.dlsettings = self.payload[10]
        self.rxdelay = self.payload[11]
        self.cflist = None
        if self.payload[12:]:
            self.cflist = self.payload[12:]

    def length(self):
        return len(self.encrypted_payload)

    def to_raw(self):
        return self.encrypted_payload

    def to_clear_raw(self):
        return self.payload

    def get_appnonce(self):
        return self.appnonce

    def get_netid(self):
        return self.netid

    def get_devaddr(self):
        return self.devaddr

    def get_dlsettings(self):
        return self.dlsettings

    def get_rxdelay(self):
        return self.rxdelay

    def get_cflist(self):
        return self.cflist

    def compute_mic(self, key, direction, mhdr):
        mic = []
        mic += self.to_clear_raw()
        mic += [mhdr.to_raw()]

        cmac = AES_CMAC()
        computed_mic = cmac.encode(str(bytearray(key)), str(bytearray(mic)))[:4]
        return map(int, bytearray(computed_mic))

    def decrypt_payload(self, key, direction):
        a = []
        a += self.encrypted_payload
        a += self.mic

        cipher = AES.new(str(bytearray(key)))
        s = map(ord, cipher.encrypt(str(bytearray(a))))
        return s

    def encrypt_payload(self, key):
        a = []
        a += self.to_clear_raw()
        a += self.compute_mic()

        cipher = AES.new(str(bytearray(key)))
        s = map(ord, cipher.decrypt(str(bytearray(a))))
        return s[:-4], s[-4:]
