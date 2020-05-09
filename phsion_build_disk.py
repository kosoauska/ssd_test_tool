#!/usr/bin/python
# -*- coding: utf-8 -*-
#coding=utf-8

import re
import os
import time
import Tkinter
import tkMessageBox
import string



debug = 0

def  init_dev(s):
    setup_info = open(s , "r")
    i = 0
    for i in range(8):
        content = setup_info.readline()
        if(i == 1):
            serial_len    = content.split('#')
        if(i == 2):
            firmware_name = content.split('#')
        if(i == 3):
            sata_speed    = content.split('#')
        if(i == 4):
           atto_path = content.split('#')
        if(i == 5):
            disk_letter  = content.split('#')
        if(i == 6):
            diskpart_letter = content.split('#')
        if(i == 7):
            smart_delay = content.split('#')
    setup_info.close()
    return serial_len[1] ,firmware_name[1] , sata_speed[1] , atto_path[1] , disk_letter[1] , diskpart_letter[1] , smart_delay[1]

def  read_sata_info(s):
    disk_info = open(s , "r")
    i = 0
    for i in range(6):
        content = disk_info.readline()
        if(i == 0):
            dev_serial_len    = content.split(':')
        if(i == 1):
            dev_firmware_name = content.split(':')
        if(i == 3):
            dev_sata_speed    = content.split(':')
    disk_info.close()
    return dev_serial_len[1] , dev_firmware_name[1] , dev_sata_speed[1]



if __name__ == '__main__':
#diskpart：get default setup
    serial_len ,firmware_name , sata_speed , atto_path , disk_letter , diskpart_letter , smart_delay = init_dev("setup.inf");
    diskpart_letter = diskpart_letter.replace('\n' ,'')
    smart_delay = smart_delay.replace(' ' ,'')
    smart_delay = string.atoi(smart_delay)
#get atto path
#formate character
    serial_len = serial_len.strip()
    firmware_name = firmware_name.strip()
    firmware_name = firmware_name.replace('\n' ,'')
    sata_speed = sata_speed.strip()
    if(debug == 1):
        print(serial_len ,firmware_name , sata_speed)
#diskpart：build disk
    main = "C:\\Windows\\System32\\diskpart.exe /s disk.txt"
#diskpart：get sata serial\firmware\speed
    r = os.system(main)
    handler = open("sata.txt" , "w+")
    main = "sata.exe"
    r = os.popen(main)
    data = r.readlines()
    handler.writelines(data)
    handler.flush()
    handler.close()
#formate character
    dev_serial_len , dev_firmware_name , dev_sata_speed = read_sata_info("sata.txt");
    dev_serial_len = dev_serial_len.strip()
    dev_firmware_name = dev_firmware_name.strip()
    dev_firmware_name = dev_firmware_name.replace('\n' ,'')
    dev_sata_speed = dev_sata_speed.strip()
    if(debug == 1):
        print(dev_serial_len ,dev_firmware_name , dev_sata_speed)
#compare result
    if(0 == cmp(firmware_name , dev_firmware_name)):
       print("********************firmware ok************************")
    else:
       print("###############firmware error:")
       tkMessageBox.showinfo(title='firmware ', message='firmware error')
       os.system("pause")


    if(len(serial_len) == len(dev_serial_len)):
       print("********************serial len ok**********************")
    else:
       print("###############serial len error:")
       tkMessageBox.showinfo(title='serial ', message='serial error')
       os.system("pause")

    if(0 == cmp(sata_speed , dev_sata_speed)):
       print("********************sata mode ok********************")
    else:
       print("###############sata mode error: %s" % dev_sata_speed)
       tkMessageBox.showinfo(title='sata speed ', message='sata speed error')
       os.system("pause")

    print("\n\n\n")
    main = "smart_change.exe"
    try:
       handler = open(main, "r")
       if (handler != None):
           r = os.popen(main)
           time.sleep(smart_delay)
           try:
               r = os.system('taskkill /im smart_change.exe /f')
               r = os.system('taskkill /im smart.exe /f')
           except:
               r = 0;
       handler.close()
    except:
        r = 0;

    print("\n\n\n")
    main = "test.exe"
    r = os.system(main)

    try:
        s  = input("press enter to clean disk")
    except:
        s = 0;

#clean disk
    try:
        r = os.system('taskkill /im Bench32.exe /f')
    except:
        r = 0;
    try:
        r = os.system('taskkill /im HDTunePro.exe')
    except:
        r = 0;
    main = "C:\\Windows\\System32\\diskpart.exe /s clean.txt"
    r = os.system(main)


