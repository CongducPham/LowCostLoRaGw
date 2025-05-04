#////////////////////////////////////////////////////////////
# ADD HERE APP KEYS THAT YOU WANT TO ALLOW FOR YOUR GATEWAY
#////////////////////////////////////////////////////////////
# NOTE: the format of the application key list has changed from 
# a list of list, to a list of string that will be process as 
# a byte array. Doing so wilL allow for dictionary construction
# using the appkey to retrieve information such as encryption key,...

app_key_list = [
	#change/add here your application keys
	'\x01\x02\x03\x04',
	'\x05\x06\x07\x08' 
]