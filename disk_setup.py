#!/usr/bin/python
# -*- coding: utf-8 -*-
#coding=utf-8

import re
import os
########################################################################################
del_line = 1
########################################################################################
def  init_dev(s):
    setup_info = open(s , "r")
    i = 0
    for i in range(4):
        content = setup_info.readline()
        if(i == 1):
            serial_len    = content.split('#')
        if(i == 2):
            firmware_name = content.split('#')
        if(i == 3):
            sata_speed    = content.split('#')
    setup_info.close()
    return serial_len[1] ,firmware_name[1] , sata_speed[1]

########################################################################################
disk = ("list     disk \n",
        "select   disk 1 \n",
        "convert  mbr \n",
        "create   partition primary \n",
        "active \n",
        "select   partition 1 \n",
        "format   quick \n",
        "assign   letter=L")
clean =(
        "select disk 1 \n",
        "clean \n",
        "list     disk")
########################################################################################
if __name__ == '__main__':
#get default disk number set
    setup_set = open("setup.inf", "r")
    content = setup_set.readline()
    content = content.split('#')
    content[1] = content[1].strip()
    content[1] = content[1].replace('\n', '')
    number = content[1]
    setup_set.close()
    print("disk number: %s" % number)
#set disk.txt and clean.txt
    disk  = list(disk)
    clean = list(clean)
    disk_set   = open("disk.txt", "w")
    disk_clean = open("clean.txt", "w")
    disk[1] = "select   disk " + number + '\n'
    clean[0] ="select   disk " + number + '\n'
#write file
    disk_set.writelines(disk)
    disk_set.flush()
    disk_clean.writelines(clean)
    disk_clean.flush()
    disk_set.close()
    disk_clean.close()