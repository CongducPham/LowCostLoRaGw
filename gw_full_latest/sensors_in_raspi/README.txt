to use the DHT22 sensor in your Raspberry PI, first enter these commands :

unzip pigpio.zip
cd PIGPIO
make -j4
sudo make install

Then, read_dht22.py will start the deamon (sudo pigpiod) if necessary.

