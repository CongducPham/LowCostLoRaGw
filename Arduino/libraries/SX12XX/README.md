SX12XX library
==============

Original library from Stuart Robinson: https://github.com/StuartsProjects/SX12XX-LoRa

It is mainly intended to support SX126X, SX127X and SX128X LoRa chips (SX1261,SX1262,SX1268,SX1272,SX1276,SX1277,SX1278,SX1279,SX1280,SX1281).

See the original repository for examples, READMEs, ... and most recent version.

WARNING: This version has been modified by C. Pham for the LowCostLoRaGw framework. received() and transmit() will probably still be compatible with the original library but receiveAddressed(), transmitAddressed(), receiveReliable() and transmitReliable() will not be compatible because our framework adds an additional byte in the header. Over time, it will probably diverge significantly from the latest version of the original library.

It has also be modified to allow compilation on both Arduino and Raspberry platforms.

A big thank to Stuart Robinson for his great work.

Enjoy,
C. Pham


