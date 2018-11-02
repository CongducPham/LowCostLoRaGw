/******************************************************************************************
* Copyright 2015, 2016 Ideetron B.V.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************************/
/******************************************************************************************
*
* File:        Encrypt_V31.h
* Author:      Gerben den Hartog
* Compagny:    Ideetron B.V.
* Website:     http://www.ideetron.nl/LoRa
* E-mail:      info@ideetron.nl
******************************************************************************************/
/****************************************************************************************
*
* Created on: 			04-02-2016
* Supported Hardware: ID150119-02 Nexus board with RFM95
*
* Firmware Version 2.0
* First version
*
* Firmware Version 2.0
* Works the same is 1.0 using own AES encryption
*
* Firmware Version 3.0
* Included direction in MIC calculation and encryption
*
* Firmware Version 3.1
* Now using AppSkey in Encrypt Payload function
****************************************************************************************/

#ifndef ENCRYPT_V31_H
#define ENCRYPT_V31_H

/*
*****************************************************************************************
* FUNCTION PROTOTYPES
*****************************************************************************************
*/

void Calculate_MIC(unsigned char *Data, unsigned char *Final_MIC, unsigned char Data_Length, unsigned int Frame_Counter, unsigned char Direction);
void Encrypt_Payload(unsigned char *Data, unsigned char Data_Length, unsigned int Frame_Counter, unsigned char Direction);
void Generate_Keys(unsigned char *K1, unsigned char *K2);
void Shift_Left(unsigned char *Data);
void XOR(unsigned char *New_Data,unsigned char *Old_Data);

#endif
