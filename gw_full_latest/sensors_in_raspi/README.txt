to use the DHT22 sensor in your Raspberry PI, first enter these commands :

unzip pigpio.zip
cd PIGPIO
make -j4
sudo make install


Then each time your Raspberry PI starts, you have to launch the following command to start the deamon before retrieving data from the DHT22 :

sudo pigpiod