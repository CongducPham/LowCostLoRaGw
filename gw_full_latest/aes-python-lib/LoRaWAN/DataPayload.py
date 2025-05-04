#
# frm_payload: data(0..N)
#
from AES_CMAC import AES_CMAC
from Crypto.Cipher import AES
import math

class DataPayload:

    def read(self, mac_payload, payload):
        self.mac_payload = mac_payload
        self.payload = payload

    def create(self, mac_payload, key, args):
        self.mac_payload = mac_payload
        self.set_payload(key, 0x00, args['data'])

    def length(self):
        return len(self.payload)

    def to_raw(self):
        return self.payload

    def set_payload(self, key, direction, data):
        self.payload = self.encrypt_payload(key, direction, data)

    def compute_mic(self, key, direction, mhdr):
        mic = [0x49]
        mic += [0x00, 0x00, 0x00, 0x00]
        mic += [direction]
        mic += self.mac_payload.get_fhdr().get_devaddr()
        mic += self.mac_payload.get_fhdr().get_fcnt()
        mic += [0x00]
        mic += [0x00]
        mic += [0x00]
        mic += [1 + self.mac_payload.length()]
        mic += [mhdr.to_raw()]
        mic += self.mac_payload.to_raw()

        cmac = AES_CMAC()
        computed_mic = cmac.encode(str(bytearray(key)), str(bytearray(mic)))[:4]
        return map(int, bytearray(computed_mic))

    def decrypt_payload(self, key, direction):
        k = int(math.ceil(len(self.payload) / 16.0))

        a = []
        for i in range(k):
            a += [0x01]
            a += [0x00, 0x00, 0x00, 0x00]
            a += [direction]
            a += self.mac_payload.get_fhdr().get_devaddr()
            a += self.mac_payload.get_fhdr().get_fcnt()
            a += [0x00] # fcnt 32bit
            a += [0x00] # fcnt 32bit
            a += [0x00]
            a += [i+1]

        cipher = AES.new(str(bytearray(key)))
        s = map(ord, cipher.encrypt(str(bytearray(a))))

        padded_payload = []
        for i in range(k):
            idx = (i + 1) * 16
            padded_payload += (self.payload[idx - 16:idx] + ([0x00] * 16))[:16]

        payload = []
        for i in range(len(self.payload)):
            payload += [s[i] ^ padded_payload[i]]
        return payload

    def encrypt_payload(self, key, direction, data):
        k = int(math.ceil(len(data) / 16.0))

        a = []
        for i in range(k):
            a += [0x01]
            a += [0x00, 0x00, 0x00, 0x00]
            a += [direction]
            a += self.mac_payload.get_fhdr().get_devaddr()
            a += self.mac_payload.get_fhdr().get_fcnt()
            a += [0x00] # fcnt 32bit
            a += [0x00] # fcnt 32bit
            a += [0x00]
            a += [i+1]

        cipher = AES.new(str(bytearray(key)))
        s = map(ord, cipher.encrypt(str(bytearray(a))))

        padded_payload = []
        for i in range(k):
            idx = (i + 1) * 16
            padded_payload += (data[idx - 16:idx] + ([0x00] * 16))[:16]

        payload = []
        for i in range(len(data)):
            payload += [s[i] ^ padded_payload[i]]
        return payload
