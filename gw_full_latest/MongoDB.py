#-------------------------------------------------------------------------------
# Copyright 2016 Congduc Pham, University of Pau, France.
# 
# Congduc.Pham@univ-pau.fr
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
#-------------------------------------------------------------------------------
 
# written by N. Bertuol under C. Pham supervision 
#
#

import pymongo
from pymongo import MongoClient
import datetime
from datetime import timedelta

_max_months_stored = 0

#set from post_processing_gw.py
def mongodb_set_max_months(max_months):

	global _max_months_stored
	_max_months_stored = max_months

#timedelta does not have month attribute, then create our monthdelta
def monthdelta(date, delta):
    m, y = (date.month+delta) % 12, date.year + ((date.month)+delta-1) // 12
    if not m: m = 12
    d = min(date.day, [31,
        29 if y%4==0 and not y%400==0 else 28,31,30,31,30,31,31,30,31,30,31][m-1])
		
	#returns date given in argument plus the delta (months)
    return date.replace(day=d, month=m, year=y)

# add a document in the collection of 'messages' database
def add_document(document):
	try:
		# client MongoDB
		client = MongoClient()

		# open database messages
		db = client.messages
		
		# adding the data in the collection
		db.ReceivedData.insert_one(document)
	except Exception as e:
		print("MongoDB: can not add document in MongoDB: " + str(e))
	
	
# remove in the collection documents which are older than _max_months_stored
def remove_if_new_month(now):
	try:
		# client MongoDB
		client = MongoClient()

		# open database messages
		db = client.messages
		
		# check if collection isn't empty
		if(db.ReceivedData.count() > 0) :
			print("MongoDB: deleting data older than "+str(_max_months_stored)+" month(s)...")

			deleted_docs = 0
			# retrieving documents older than _max_months_stored month(s) data
			for msg in db.ReceivedData.find({"time": {"$lt" : monthdelta(now, -_max_months_stored)}}) :	
				#delete all these retrieved documents
				db.ReceivedData.remove(msg)
				deleted_docs += 1
			
			print("MongoDB: "+str(deleted_docs)+" documents deleted")
	
	except Exception as e:
		print("MongoDB: error while deleting documents: " + str(e))
			
# get all documents added since the date passed in argument
def get_documents_since_date(date):

	# client MongoDB
	client = MongoClient()

	# open database messages
	db = client.messages

	#recover corresponding documents and return it
	return db.ReceivedData.find({ "time": {"$gte" : date}}).sort("time", pymongo.ASCENDING)
	
# get documents corresponding to the nodes and the dates (for the Android application)
def get_documents(nodeArray, beginDate, endDate):
	# client MongoDB
	client = MongoClient()

	# open database messages
	db = client.messages
		
	#no preferences were set, preferences page has not been visited yet (returns all data from all nodes)
	if(nodeArray is None and beginDate is None and endDate is None):
		#get utc date of now substracted by 8 days
		date = datetime.datetime.utcnow() - timedelta(days=8)
		date = date.replace(hour=0, minute=0, second=0, microsecond=0)
		
		#recover documents
		documents = db.ReceivedData.find({"time": {"$gte" : date}}).sort("time", pymongo.ASCENDING)
		docArray = []
		
		for doc in documents :
			docArray.append(doc)
			
		return docArray
		
	#only nodes names in preferences (returns all data from nodes checked in the Android app)
	elif(beginDate is None and endDate is None):
		#get utc date of now substracted by 8 days
		date = datetime.datetime.utcnow() - timedelta(days=8)
		date = date.replace(hour=0, minute=0, second=0, microsecond=0)
		
		arrayDocuments = []
		
		documents = db.ReceivedData.find({"time": {"$gte" : date}}).sort("time", pymongo.ASCENDING)
		docArray = []
		
		for doc in documents :
			if(str(doc['node_eui']) in nodeArray):
				docArray.append(doc)
			
		return docArray
		
	#all preferences were set (returns documents for the specified nodes and between the specified dates)
	else:
		documentsArray = []
		
		documents = db.ReceivedData.find({"time": {"$gte" : beginDate, "$lte" : endDate}}).sort("time", pymongo.ASCENDING)
		docArray = []
		
		for doc in documents :
			if(str(doc['node_eui']) in nodeArray):
				docArray.append(doc)
			
		return docArray
	
# return nodes names of the documents (for the Android application)
def get_nodes_names():

	# client MongoDB
	client = MongoClient()

	# open database messages
	db = client.messages

	#recover documents and recover from it the nodes names (sort by time then by node_eui to retrieve first and last date of the current node)
	documents = db.ReceivedData.find().sort("time", pymongo.ASCENDING).sort("node_eui", pymongo.ASCENDING)
	
	#the number of documents retrieved
	number_of_documents = documents.count()
	#contains the nodes names with their begin date and end date
	nodes_names = ""
	#contains begin date and end date
	dates_current_node = ""
	#to know the date of last doc if node_eui has changed
	last_date = ""
	#to compare with de node name of the current doc
	last_node_name = ""
	#to know if we are at the last document
	iteration = 0
	
	#filling nodes_names
	for doc in documents:
	
		#first doc retrieved
		if(last_node_name == ""):
			last_node_name = str(doc['node_eui'])
			nodes_names = last_node_name
			
			#getting date in format "YYYY-MM-DD"
			dates_current_node = (doc["time"]).date().isoformat()
			last_date = dates_current_node
			
			iteration += 1
		
		#the current doc has a different node_eui than the last one
		elif(last_node_name != str(doc['node_eui'])):
			last_node_name = str(doc['node_eui'])
			dates_current_node += "_"+last_date
			nodes_names += "#"+dates_current_node+";"+last_node_name
			
			#getting date in format "YYYY-MM-DD"
			dates_current_node = (doc["time"]).date().isoformat()
			last_date = dates_current_node
			
			iteration += 1
			
		#nothing particular, change last_date and increment iteration
		else:
			#getting date in format "YYYY-MM-DD"
			last_date = (doc["time"]).date().isoformat()
			iteration += 1
			
		#last doc retrieved
		if(iteration == number_of_documents):
			dates_current_node += "_"+(doc["time"]).date().isoformat()
			nodes_names += "#"+dates_current_node
			
	return nodes_names
	
#to know if "str" represents an integer or not
def represents_integer(str):
    try: 
        int(str)
        return True
    except ValueError:
        return False