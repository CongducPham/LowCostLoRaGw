Crosscompiling Low-cost LoRa gateway for the Raspberry
====================================


    > apt-get install git rsync cmake libc6-i386 lib32z1 lib32stdc++6

    > mkdir -p ~/raspberrypi
    > cd ~/raspberrypi
    > git clone git://github.com/raspberrypi/tools.git


You wanted to use the following of the 3 ones, gcc-linaro-arm-linux-gnueabihf-raspbian

    > echo 'export PATH=$PATH:$HOME/raspberrypi/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin' >>  ~/.bashrc

	> source ~/.bashrc

Now, verify that you can access the compiler:
 	
    > arm-linux-gnueabihf-gcc -v

Compiling:

    > make CXX=arm-linux-gnueabihf-g++ LD=arm-linux-gnueabihf-ld
