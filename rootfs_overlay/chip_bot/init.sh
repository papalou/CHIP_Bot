#!/bin/sh

#determine if we launch bot or controler mode
gpio_mode_number=$(ls /sys/class/gpio/ | grep gpiochip | grep -v gpiochip0 | awk -F"gpiochip" '{print $2}' )
echo "GPIO XIO-P0 is number: $gpio_mode_number"

#Export gpio XIO-P0
if [ ! -d "/sys/class/gpio/gpio${gpio_mode_number}" ]; then
	echo "$gpio_mode_number" > "/sys/class/gpio/export"
fi

#Set gpio direction
echo "in" > "/sys/class/gpio/gpio${gpio_mode_number}/direction"

#Get value of the gpio
mode=$(cat "/sys/class/gpio/gpio${gpio_mode_number}/value")

echo "Gpio mode stat: $mode"

if [ "$mode" == "0" ]; then
	echo "START BOT MODE"

	#Run wifi access point
	#Unlock device
	rfkill unblock all
	
	# Configure Wlan0 ip
	ifconfig wlan0 up 192.168.2.1 netmask 255.255.255.0
	
	#Start Access Point
	hostapd -B /chip_bot/config/hostapd.conf
	
	#start dhcp server
	#TODO
	#touch /var/db/dhcpd.leases
	#dhcpd -cf /chip_bot/config/dhcpd.conf

	#Run chip_bot
	/chip_bot/chip_bot -m 0 &

else
	echo "START CONTROLER MODE"

	#Run chip_bot
	/chip_bot/chip_bot -m 1 &

fi

exit 0
