from Crypto.Cipher import AES
from struct import pack, unpack

class AES_CMAC:
    def gen_subkey(self, K):
        AES_128 = AES.new(K)

        L = AES_128.encrypt('\x00'*16)

        LHigh = unpack('>Q',L[:8])[0]
        LLow  = unpack('>Q',L[8:])[0]

        K1High = ((LHigh << 1) | ( LLow >> 63 )) & 0xFFFFFFFFFFFFFFFF
        K1Low  = (LLow << 1) & 0xFFFFFFFFFFFFFFFF

        if (LHigh >> 63):
            K1Low ^= 0x87

        K2High = ((K1High << 1) | (K1Low >> 63)) & 0xFFFFFFFFFFFFFFFF
        K2Low  = ((K1Low << 1)) & 0xFFFFFFFFFFFFFFFF

        if (K1High >> 63):
            K2Low ^= 0x87

        K1 = pack('>QQ', K1High, K1Low)
        K2 = pack('>QQ', K2High, K2Low)

        return K1, K2

    def xor_128(self, N1, N2):
        J = ''
        for i in range(len(N1)):
            J = J + chr(ord(N1[i]) ^ ord(N2[i]))
        return J

    def pad(self, N):
        const_Bsize = 16
        padLen = 16-len(N)
        return  N + '\x80' + '\x00'*(padLen-1)

    def encode(self, K, M):
        const_Bsize = 16
        const_Zero  = '\x00'*16

        AES_128= AES.new(K)
        K1, K2 = self.gen_subkey(K)
        n      = len(M)/const_Bsize

        if n == 0:
            n = 1
            flag = False
        else:
            if (len(M) % const_Bsize) == 0:
                flag = True
            else:
                n += 1
                flag = False

        M_n = M[(n-1)*const_Bsize:]
        if flag is True:
            M_last = self.xor_128(M_n,K1)
        else:
            M_last = self.xor_128(self.pad(M_n),K2)

        X = const_Zero
        for i in range(n-1):
            M_i = M[(i)*const_Bsize:][:16]
            Y   = self.xor_128(X, M_i)
            X   = AES_128.encrypt(Y)
        Y = self.xor_128(M_last, X)
        T = AES_128.encrypt(Y)

        return T
