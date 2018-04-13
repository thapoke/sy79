
#!/bin/bash

startme(){
## Stop the ntp service
#sudo service ntp stop

## Stop the triggerhappy service
#sudo service triggerhappy stop

## Stop the dbus service. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
#sudo service dbus stop

## Stop the console-kit-daemon service. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
#sudo killall console-kit-daemon

## Stop the polkitd service. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
#sudo killall polkitd

## Only needed when Jack2 is compiled with D-Bus support (Jack2 in the AutoStatic RPi audio repo is compiled without D-Bus support)
export DBUS_SESSION_BUS_ADDRESS=unix:path=/run/dbus/system_bus_socket

## Remount /dev/shm to prevent memory allocation errors
sudo mount -o remount,size=128M /dev/shm

## Kill the usespace gnome virtual filesystem daemon. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
#killall gvfsd

## Kill the userspace D-Bus daemon. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
#killall dbus-daemon

## Kill the userspace dbus-launch daemon. Warning: this can cause unpredictable behaviour when running a desktop environment on the RPi
#killall dbus-launch

## Uncomment if you'd like to disable the network adapter completely
#echo -n “1-1.1:1.0” | sudo tee /sys/bus/usb/drivers/smsc95xx/unbind
## In case the above line doesn't work try the following
#echo -n “1-1.1” | sudo tee /sys/bus/usb/drivers/usb/unbind

## Set the CPU scaling governor to performance
echo -n performance | sudo tee /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

#jackd   -d alsa -s -i 8 -o 8  -p 64 -n 2 -r 44100 &
jackd -a A -d alsa -s true -i 8 -o 8 -p 128 -n 2 -r 44100 &
sleep 5 ;
patchage &
sleep 6 ;
#jack_control start ;
#sleep 6 ;
pd-l2ork  -jack  -alsamidi -listdev  ~/Documents/pd_modularorig/opener.pd &
sleep 6 ; 
jack_disconnect pure_data_0:output0 system:playback_1
jack_disconnect pure_data_0:output1 system:playback_2
jack_disconnect pure_data_0:output2 system:playback_3
jack_disconnect pure_data_0:output3 system:playback_4
jack_disconnect pure_data_0:output4 system:playback_5
jack_disconnect pure_data_0:output5 system:playback_6
jack_disconnect pure_data_0:output6 system:playback_7
jack_disconnect pure_data_0:output7 system:playback_8
jack_connect pure_data_0:output0 system:playback_1
jack_connect pure_data_0:output1 system:playback_2
jack_connect pure_data_0:output2 system:playback_3
jack_connect pure_data_0:output3 system:playback_4
jack_connect pure_data_0:output4 system:playback_5
jack_connect pure_data_0:output5 system:playback_6
jack_connect pure_data_0:output6 system:playback_7
jack_connect pure_data_0:output7 system:playback_8
aconnect 20:0 129:0 
}

stopme() {
pkill -f "pd-l2ork"
sleep 1 ;
jack_control stop ;
sleep 1 ;
#pkill -f "jackd"
#sleep 1 ;
#pkill -f "jackd"
#sleep 1 ;
}

case "$1" in 
        start)  startme ;;
        stop)   stopme ;;
        restart) stopme ; startme ;;
*) echo "usage: start|stop|restart" >&2
exit 1
;;
esac


