#------------------------------------------------------------
# Copyright 2019 
# 
# Raphael Couturier, University of Franche Comte, France, raphael.couturier@univ-fcomte.fr
# Congduc Pham, University of Pau, France, Congduc.Pham@univ-pau.fr 
# 
# This file is part of the low-cost LoRa gateway developped at University of Pau
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with the program.  If not, see <http://www.gnu.org/licenses/>.
#
#------------------------------------------------------------

# 


import numpy as np
import time

np.seterr(over='ignore')

LSC_DETERMINISTIC=True
LSC_SKEY=16
LSC_STATIC_KEY=True

def xorshift32(t):
    x=t
    x=x^(x<<np.uint32(13))
    x=x^(x>>np.uint32(17))
    x=x^(x<<np.uint32(5))
    return np.uint32(x)

def rc4key(key, sc, size_DK):

  for i in range(256):
    sc[i]=i


  j0 = 0
  for  i0 in range(256):
    #print((j0 + sc[i0] + key[i0&(size_DK-1)] ))
    j0 = (j0 + sc[i0] + key[i0&(size_DK-1)] )
    tmp = sc[i0]
    sc[np.uint8(i0)] = sc[np.uint8(j0)]
    sc[np.uint8(j0)] = tmp
  
def rc4keyperm(key, lenH, rp, sc, size_DK):

  for i in range(lenH):    
    sc[i]=i
  

  for it in range(rp):
    j0 = 1;
    for i0 in range(lenH):
      j0 = (j0 + sc[i0] + sc[j0] + key[i0%size_DK] )% lenH
      tmp = sc[i0]
      sc[i0] = sc[j0]
      sc[j0] = tmp
    
  
def prga(sc, ldata, r):
  i0=0
  j0=0

  for it in range(ldata):
    i0 = ((i0+1)%255)                                                                                              
    j0 = (j0 + sc[i0])&0xFF
    tmp = sc[i0]
    sc[i0] = sc[j0]
    sc[j0] = tmp

    r[it]=sc[(sc[i0]+sc[j0])&0xFF]

def encrypt_ctr(seq_in, seq_out, lenH, RM1, PboxRM, Sbox1, Sbox2, myrand):

  X = np.empty([h2],dtype=np.uint8)
  fX = np.empty([h2],dtype=np.uint8)  

  ind=0

  for a in range(0,h2,4):

    mm=myrand    
    X[a]=Sbox1[RM1[a]^(mm&255)]           #Warning according to the size of h2, we can be outsize of Sbox1[a]
    mm>>=8
    X[a+1]=Sbox2[RM1[a+1]^(mm&255)]
    mm>>=8
    X[a+2]=Sbox1[RM1[a+2]^(mm&255)]
    mm>>=8
    X[a+3]=Sbox2[RM1[a+3]^(mm&255)]
  
  
  for it in range(lenH):

    for a in range(0,h2,4):
      #if not LSC_DETERMINISTIC:
      myrand=xorshift32(myrand)
      mm=myrand
      X[a]=Sbox2[X[a]^RM1[a]^(mm&255)]
      mm>>=8
      X[a+1]=Sbox1[X[a+1]^RM1[a+1]^(mm&255)]
      mm>>=8
      X[a+2]=Sbox2[X[a+2]^RM1[a+2]^(mm&255)]
      mm>>=8
      X[a+3]=Sbox1[X[a+3]^RM1[a+3]^(mm&255)]
    

    for a in range(h2):
      fX[a]=X[a]^seq_in[ind+a]

    for a in range(h2):
      seq_out[ind+a]=fX[a];

    for a in range(0,h2,4):      
      RM1[a]=Sbox2[RM1[PboxRM[a]]]
      RM1[a+1]=Sbox1[RM1[PboxRM[a+1]]]
      RM1[a+2]=Sbox2[RM1[PboxRM[a+2]]]
      RM1[a+3]=Sbox1[RM1[PboxRM[a+3]]]
    

    ind=ind+h2

h=4
h2=h*h

static_plain=True

if (static_plain):
	s_plain = "HELLO WORLD!!!!!!!!!"
	size_mesg = len(s_plain)
else:	
	size_mesg=36

lenH=np.uint32((size_mesg+h2-1)/h2)

print(lenH)
rp=1    

seed=123 #np.uint32(time.time())
#seed=xorshift32(seed)
#print(seed)
#seed=xorshift32(seed)
#print(seed)
#sys.exit(0)

DK = np.empty([LSC_SKEY],dtype=np.uint8)
Nonce = np.empty([LSC_SKEY],dtype=np.uint8)
sc = np.empty([256],dtype=np.uint8)
PboxRM = np.empty([h2],dtype=np.uint8)
Sbox1 = np.empty([256],dtype=np.uint8)
Sbox2 = np.empty([256],dtype=np.uint8)
RM1 = np.empty([h2],dtype=np.uint8)
RM2 = np.empty([h2],dtype=np.uint8)
RMorig = np.empty([h2],dtype=np.uint8)

for i in range(0,LSC_SKEY,4):
    seed=xorshift32(seed)
    #print(seed)
    val=seed
    DK[i]=val&0xFF
    val>>=8
    DK[i+1]=val&0xFF
    val>>=8
    DK[i+2]=val&0xFF
    val>>=8
    DK[i+3]=val&0xFF
    
#print(DK)

if (LSC_STATIC_KEY):
	if (LSC_SKEY==256):
		Nonce = [ 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C ]	     
	if (LSC_SKEY==64):
		Nonce = [ 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C ]	  
	if (LSC_SKEY==32):
		Nonce = [ 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C, \
				  0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C ]
	if (LSC_SKEY==16):
		Nonce = [ 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C ]					  
else:	
	for i in range(0,LSC_SKEY,4):
		seed=xorshift32(seed)
		val=seed
		Nonce[i]=val&0xFF
		val>>=8
		Nonce[i+1]=val&0xFF
		val>>=8
		Nonce[i+2]=val&0xFF
		val>>=8
		Nonce[i+3]=val&0xFF

for i in range(LSC_SKEY):
    DK[i]=DK[i]^Nonce[i]

rc4key(DK[0:LSC_SKEY/4], sc, LSC_SKEY/4)
#print("sc")
#print(sc)
prga(sc, h2, RM1);
#print("RM1")
#print(RM1)
rc4keyperm(DK[LSC_SKEY/4:2*LSC_SKEY/4], h2, rp, PboxRM, LSC_SKEY/4);
#print("PboxRM")
#print(PboxRM)
rc4key(DK[2*LSC_SKEY/4:3*LSC_SKEY/4], Sbox1, LSC_SKEY/4);
rc4key(DK[3*LSC_SKEY/4:LSC_SKEY], Sbox2, LSC_SKEY/4);
#print("Sbox1")
#print(Sbox1)
#print("Sbox2")
#print(Sbox2)

RM2=np.copy(RM1)
RMorig=np.copy(RM1)

plain = np.empty([lenH*h2],dtype=np.uint8)
cipher = np.empty([lenH*h2],dtype=np.uint8)
check = np.empty([lenH*h2],dtype=np.uint8)

frame = np.empty([64],dtype=np.uint8)
cipher_frame = np.empty([64],dtype=np.uint8)

myrand=np.uint32(0)

for i in range(min(LSC_SKEY,32)):
	myrand=myrand|(DK[i]&1);
	myrand=np.uint32(myrand<<1)

fcount=0

print(fcount)

for i in range(lenH*h2):
    plain[i]=0

if (static_plain):
	print "static plain: %s" % s_plain
	for i in range(size_mesg):   
		plain[i]=ord(s_plain[i])
else:	
	for i in range(size_mesg):
		seed=xorshift32(seed)    
		plain[i]=seed&0xFF
    
print(plain)

encrypt_ctr(plain, cipher, lenH, RM1, PboxRM, Sbox1, Sbox2, fcount)

for i in range(size_mesg,lenH*h2):
    cipher[i]=0

print(cipher)

header = 0

#set the header
frame[header] = 1
frame[header+1] = 0x10
frame[header+2] = 6
frame[header+3] = fcount
header += 4

#copy the ciphered part
for i in range(0,size_mesg):
    frame[i+header]=cipher[i]

#set the new message size
size_mesg += header

#normally, should rebuild the dynamic table according to new message size

print "frame:"
print(frame)

#restore original RM
RM1=np.copy(RMorig)

#encrypt the whole message header+cipher
encrypt_ctr(frame, cipher_frame, lenH, RM1, PboxRM, Sbox1, Sbox2, fcount)

print "cipher frame:"
print(cipher_frame)

#compute MIC at sender
#should implement a better algorithm?
myMIC=np.sum(cipher_frame[:size_mesg])
cipher_frame[0]=xorshift32(myMIC % 7)
cipher_frame[1]=xorshift32(myMIC % 13)
cipher_frame[2]=xorshift32(myMIC % 29)
cipher_frame[3]=xorshift32(myMIC % 57)

print "MIC[0:3]"
print(cipher_frame[:4])

#append MIC into frame
for i in range(0,4):
    frame[size_mesg+i]=cipher_frame[i]

print "final frame:"
print(frame)

#restore original RM
RM1=np.copy(RMorig)

#check MIC on decrypt
#first re-encrypt the whole frame header+cipher part, not taking the MIC part
encrypt_ctr(frame, cipher_frame, lenH, RM1, PboxRM, Sbox1, Sbox2, fcount)

#compare MICs
print "received MIC"
print(frame[size_mesg:size_mesg+4])

#compute MIC at receiver
#should implement a better algorithm?
myMIC=np.sum(cipher_frame[:size_mesg])
cipher_frame[0]=xorshift32(myMIC % 7)
cipher_frame[1]=xorshift32(myMIC % 13)
cipher_frame[2]=xorshift32(myMIC % 29)
cipher_frame[3]=xorshift32(myMIC % 57)

print "computed MIC"
print(cipher_frame[:4])

#remove the header size
size_mesg -= 4

#get the cipher part
for i in range(0,size_mesg):
    cipher[i]=frame[header+i]
    
#decrypt the cipher part, notice that we use RM2
encrypt_ctr(cipher, check, lenH, RM2, PboxRM, Sbox1, Sbox2, fcount)

print(check)

if (static_plain):
	s_plain = check.tostring()
	print s_plain[:size_mesg]	

equal=True
for i in range(size_mesg):
    if(check[i]!=plain[i]):
        equal=False

print("CHECK ",equal)



