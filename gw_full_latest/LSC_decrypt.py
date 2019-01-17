#------------------------------------------------------------
# Copyright 2019 
# 
# Raphael Couturier, University of Franche Comte, France, raphael.couturier@univ-fcomte.fr
# Congduc Pham, University of Pau, France, Congduc.Pham@univ-pau.fr 
#
# The core works on LightWeight Steam Cipher are from:
# 	Hassan Noura, American University of Beirut, Lebanon
#	Raphael Couturier, University of Franche Comte, France
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

import numpy as np
import time
import base64
import sys
import re

#contains the encryption key used by LSC: LSC_Nonce
import LSC_config

np.seterr(over='ignore')

LSC_DETERMINISTIC=True
LSC_SKEY=16
LSC_STATIC_KEY=True
#message has MIC appended so normally there are HEADER+CIPHER+MIC
#using MIC is highly recommended
LSC_WMIC=True
#MIC method
LSC_MICv=2

#MIC size in bytes
LSC_SMIC=4

#------------------------------------------------------------
# our LoRa header packet information
#------------------------------------------------------------

HEADER_SIZE=4
APPKEY_SIZE=4
PKT_TYPE_DATA=0x10
PKT_TYPE_ACK=0x20

PKT_FLAG_ACK_REQ=0x08
PKT_FLAG_DATA_ENCRYPTED=0x04
PKT_FLAG_DATA_WAPPKEY=0x02
PKT_FLAG_DATA_DOWNLINK=0x01

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

def LSC_process_pkt(lorapkt, dst, ptype, src, seq):

	#it seems that there is not a full reset when calling this function from a parent Python program
	#so we need to reset at the beginning of the function
	if LSC_DETERMINISTIC==True:
		RM1=np.copy(RMorig)
		RM2=np.copy(RM1)
	
	size_mesg = len(lorapkt)
	
	if LSC_WMIC:
		lenH=np.uint32((size_mesg+HEADER_SIZE+h2-1)/h2)
	else:		
		lenH=np.uint32((size_mesg+h2-1)/h2)

	plain = np.empty([lenH*h2],dtype=np.uint8)
	cipher = np.empty([lenH*h2],dtype=np.uint8)
	check = np.empty([lenH*h2],dtype=np.uint8)
	
	for i in range(lenH*h2):
		cipher[i]=0

	if LSC_WMIC:

		#add the packet header to compute MIC
		cipher[0]=dst
		cipher[1]=ptype
		cipher[2]=src
		cipher[3]=seq
		
		for i in range(size_mesg):   
			cipher[HEADER_SIZE+i]=lorapkt[i]		
		
		print "?LSC: received MIC: ",
		print (cipher[size_mesg+HEADER_SIZE-LSC_SMIC:size_mesg+HEADER_SIZE])
		
		#encrypt received content: HEADER+CIPHER
		#but with fcount=seq+1
		#here is use the plain buffer
		encrypt_ctr(cipher, plain, lenH, RM1, PboxRM, Sbox1, Sbox2, (seq+1) % 256)
		
		if LSC_MICv==1:
			#skip the first 4 bytes and take the next 4 bytes of encrypted HEADER+CIPHER
			plain[0]=plain[LSC_SMIC]
			plain[1]=plain[LSC_SMIC+1]
			plain[2]=plain[LSC_SMIC+2]
			plain[3]=plain[LSC_SMIC+3]	
		elif LSC_MICv==2:
			#first, compute byte-sum of encrypted HEADER+CIPHER
			myMIC=np.sum(plain[:size_mesg+HEADER_SIZE-LSC_SMIC])

			plain[0]=xorshift32(np.uint32(myMIC % 7))
			plain[1]=xorshift32(np.uint32(myMIC % 13))
			plain[2]=xorshift32(np.uint32(myMIC % 29))
			plain[3]=xorshift32(np.uint32(myMIC % 57))
		elif LSC_MICv==3:
			#should implement a better algorithm?
			#XTEA?: http://code.activestate.com/recipes/496737-python-xtea-encryption/	
			print "todo"
		
		print "?LSC: computed MIC: ",
		print (plain[:LSC_SMIC])
		
		if np.array_equal(plain[:LSC_SMIC], cipher[size_mesg+HEADER_SIZE-LSC_SMIC:size_mesg+HEADER_SIZE]):
	
			print "?LSC: valid MIC"
			
			#print "?LSC: [cipher]: ",
			#print (cipher[HEADER_SIZE:size_mesg+HEADER_SIZE-LSC_SMIC])
			
			#re-index cipher data to get rid of HEADER and MIC
			size_mesg -= LSC_SMIC
			for i in range(size_mesg):   
				cipher[i]=lorapkt[i]
		else:
			return "###BADMIC###"
	else:
		for i in range(size_mesg):   
			cipher[i]=lorapkt[i]		

	#notice the usage of RM2 to decrypt as RM1 has changed
	encrypt_ctr(cipher, check, lenH, RM2, PboxRM, Sbox1, Sbox2, seq)

	#print "?LSC: [plain]: ",
	#print (check)

	s_plain = check.tostring()
	print "?LSC: plain payload is "+replchars.sub(replchars_to_hex, s_plain[:size_mesg])
	return s_plain[:size_mesg]
			

# GLOBAL VARIABLES
# and initialization
####################

h=4
h2=h*h
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

PKT_TYPE_DATA=0x10

#to display non printable characters
replchars = re.compile(r'[\x00-\x1f]')

def replchars_to_hex(match):
	return r'\x{0:02x}'.format(ord(match.group()))

#change your key in LSC_config
#change in the Arduino code as well
#
if (LSC_STATIC_KEY):
	Nonce=np.copy(LSC_config.Nonce) 				  
else:	
	#random key, based on seed. Change the seed if you want but change in the Arduino code as well
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

myrand=np.uint32(0)

for i in range(min(LSC_SKEY,32)):
	myrand=myrand|(DK[i]&1);
	myrand=np.uint32(myrand<<1)

# with following setting:
#
# LSC_DETERMINISTIC=True
# LSC_SKEY=16
# LSC_STATIC_KEY=True
# LSC_WMIC=True
# LSC_MICv=2
#
# when fcount=0
#
# "HELLO WORLD!!!!!!!!!" = [72 69 76 76 79 32 87 79 82 76 68 33 33 33 33 33 33 33 33 33]
# encrypted = [ 254 147 145 154 56 130 222 180 252 194 193 197 194 156 108 48 47 242 109 104 ]
#
# use this Python code to produce base64 encoding of encrypted+MIC:
#
# import base64
# l=[254, 147, 145, 154, 56, 130, 222, 180, 252, 194, 193, 197, 194, 156, 108, 48, 47, 242, 109, 104, 165, 140, 49, 198]
# l1="".join(map(chr, l))
# l64=base64.b64encode(l1)
# print l64
# /pORmjiC3rT8wsHFwpxsMC/ybWiljDHG
#
# or get the first output from the Arduino_Encrypt_LSC_v2.ino example
#
# Then test the decrypt Python code:
#
# python LSC_decrypt.py "/pORmjiC3rT8wsHFwpxsMC/ybWiljDHG" "1,22,6,0,24,8,-45" "125,5,12"
#
# output is:
#
# ?received MIC:  [165 140  49 198]
# ?computed MIC:  [165 140  49 198]
# ?LSC: valid MIC
# ?LSC[plain payload]:  HELLO WORLD!!!!!!!!!
# ?plain payload is : HELLO WORLD!!!!!!!!!
# ^p1,18,6,0,20,8,-45
# ^r125,5,12
# ??HELLO WORLD!!!!!!!!!
#
#
if __name__ == "__main__":
	
	argc=len(sys.argv)
	
	if argc>1:
		#we assume that the input frame is given in base64 format
		lorapktstr_b64=sys.argv[1]
	else:
		sys.exit("LSC-decrypt.py needs at least a base64 encoded string argument")
	
	if argc>2:	
		pdata=sys.argv[2]
		arr = map(int,pdata.split(','))
		dst=arr[0]
		ptype=arr[1]			
		src=arr[2]
		seq=arr[3]
		datalen=arr[4]
		SNR=arr[5]
		RSSI=arr[6]

		#LoRaWAN packet
		if dst==256:
			src_str="%0.8X" % src
		else:
			src_str=str(src)
		
	if argc>3:	
		rdata=sys.argv[3]
	
	plain_payload="###BADMIC###"
	
	try:
		lorapktstr=base64.b64decode(lorapktstr_b64)
		lorapkt=[]
	
		for i in range (0,len(lorapktstr)):
			lorapkt.append(ord(lorapktstr[i]))
	
		plain_payload=LSC_process_pkt(lorapkt, dst, ptype, src, seq)		
		
	except TypeError:
		plain_payload="###BADMIC###"	

	if plain_payload=="###BADMIC###":
		print '?'+plain_payload
	else:	
		#the output is clear data
		ptype = ptype & (~PKT_FLAG_DATA_ENCRYPTED)	
		print "?plain payload is: "+plain_payload
		if argc>2:
			print "^p%d,%d,%d,%d,%d,%d,%d" % (dst,ptype,src,seq,len(plain_payload),SNR,RSSI)
		if argc>3:
			print "^r"+rdata
		print "\xFF\xFE"+plain_payload


