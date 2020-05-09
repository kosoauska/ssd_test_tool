#!/usr/bin/python
# -*- coding: utf-8 -*-
# coding=utf-8

import re
import os
import time
import string
import Tkinter
import sys
import tkMessageBox

# start debug
print_debug = 0
init_debug = 0
sata_test_debug = 0
disk_part_debug = 0
hd_test_debug = 0
log_test_debug = 0
# end debug
################################################################################################################
################################################################################################################
################################################################################################################
INVALID_ITEM = 999
setup_file_name = "setup.inf"
sata_file_name = "sata.txt"
hdtune_scan_name = "scan.txt"
hdtune_health_name = "health.txt"
log_result_file = "test_disk_cnt.txt"
reverse_file_name = ["setup.inf", "sata.txt", "scan.txt", "health.txt", "test_disk_cnt.txt",
                     "clean.txt", "disk.txt", "health.txt", "phsion_disk_check.py", "phsion_setup.py",
                     "pskill.exe", "pskill64.exe", "resource.bat", "sata.exe", "SSDAutoTestTool.exe"
                     ]
# add constant here: disk_tital = [XXXX , XXXX, XXXX ,XXXX]
disk_tital = ["disknum", "serial", "firmware",  "module" , "sataType", "letter", "diskpart", "hdtest", "capacity"]
# add sata test item: sata_tital = [XXXX , XXXX, XXXX ,XXXX]
sata_tital = ["serial", "firmware", "module" , "sataType"]
# add sata test error: sata_error_msg = [XXXX , XXXX, XXXX ,XXXX]
sata_error_msg = [u'序列号错误', u'固件版本错误', u'厂商信息错误' , u'SATA3不识别']
# add hdtune test item here: hd_tital = [XXXX , XXXX, XXXX ,XXXX]
hd_tital = [u"损坏块", u"健康状态", u"HD Tune Pro:"]
# add hdtune test standard: sata_error_msg = [XXXX , XXXX, XXXX ,XXXX]
hd_stand = [u"0.0%", u"正常"]
tital_cnt = len(disk_tital)
sata_tital_cnt = len(sata_tital)
hdtune_tital_cnt = len(hd_tital)
disk_stand = [None] * tital_cnt
sata_result = [None] * sata_tital_cnt
hd_result = [None] * hdtune_tital_cnt


################################################################################################################
################################################################################################################
################################################################################################################
def formate_item_str(header, content):
    replace_str = ['\r', '\n', ' ']
    for i in range(0, len(replace_str)):
        header = header.replace(replace_str[i], '')
        content = content.replace(replace_str[i], '')
    return header, content


# *********************************search special item string****************************************************
def search_item(item, serach_arr):
    if (serach_arr == None):
        print("invalid result")
        return INVALID_ITEM
    if print_debug == 1:
        print("tital cnt = %d" % tital_cnt)
        print("search item =  %s" % item)
    for i in range(0, len(serach_arr)):
        if cmp(item, serach_arr[i]) == 0:
            if print_debug == 1:
                print("search success offset = %d" % i)
            return i
    return INVALID_ITEM


# *********************************split string     ************************************************************
def split_string(line_str, split_str):
    try:
        head_str, content_str = line_str.split(str(split_str))
    except:
        if print_debug == 1:
            print("split error")
        return None, None
    return head_str, content_str


# *********************************get special item standard****************************************************
def get_info(line_str, split_str, search_arr, result_arr):
    # spare line by str
    head_str, content_str = split_string(line_str, split_str)
    # replace uselss char
    head_str, content_str = formate_item_str(head_str, content_str)
    item_offset = search_item(head_str, search_arr)
    if item_offset != INVALID_ITEM:
        result_arr[item_offset] = content_str
        if print_debug == 1:
            print("get info success offset = %d , str = %s" % (item_offset , result_arr[item_offset]))
        return True
    if print_debug == 1:
        print("get info fail header = %s , content = %s" % (head_str , content_str))
    return False


# *********************************get setup info***************************************************************
def init_dev(file_name):
    with open(file_name, "r") as setup_info:
        file_len = len(setup_info.readlines())
        setup_info.seek(0)
        if init_debug == 1:
            print("setup.txt file len = %d " % file_len)
        for i in range(0, file_len):
            read_file = setup_info.readline()
            get_info(read_file, '#', disk_tital, disk_stand)
    if init_debug == 1:
        print("*************************standard result**********************")
        print(disk_tital)
        print(disk_stand)


# *********************************get sata result**************************************************************
def read_sata_info(file_name):
    with open(file_name, "r") as sata_info:
        file_len = len(sata_info.readlines())
        sata_info.seek(0)
        if sata_test_debug == 1:
            print("sata.txt file len = %d " % file_len)
        for i in range(0, file_len):
            read_file = sata_info.readline()
            get_info(read_file, ':', sata_tital, sata_result)
    if sata_test_debug == 1:
        print("*************************sata result*************************")
        print(sata_tital)
        print(sata_result)


# *********************************compare result**************************************************************
def compare_sata_test_result():
    for i in range(0, sata_tital_cnt):
        stand_offset = search_item(sata_tital[i], disk_tital)
        sata_offset  = search_item(sata_tital[i], sata_tital)
        if ((INVALID_ITEM == stand_offset) or (INVALID_ITEM == sata_offset)):
            if sata_test_debug == 1:
                print("invalid str %s" % sata_tital[i])
            os._exit(0)
        # serial compare len
        if disk_tital[stand_offset] == "serial":
            if (len(disk_stand[stand_offset]) != len(sata_result[sata_offset])):
                return False, i
        # other compare str
        else:
            if cmp(disk_stand[stand_offset], sata_result[sata_offset]) != 0:
                if sata_test_debug == 1:
                    print("sata not satisfy stand[%d] = %s , read[%d] = %s" % (stand_offset,
                                                                               disk_stand[stand_offset], sata_offset,
                                                                               sata_result[sata_offset]))
                return False, i
    return True, None

# *********************************call sata error print********************************************************
def sata_test_error(error_offset):
    tkMessageBox.showinfo(title=sata_tital[error_offset], message=sata_error_msg[error_offset])
    os._exit(0)


# *********************************hdtune test*******************************************************************
def hdtune_test(disk_letter):
    # HDTunePro
    health_cmd = "HDTunePro.exe " + "/DISK:" + disk_letter + " /FUNCTION:Health  /START  /LOG:health.txt "
    scan_cmd = "HDTunePro.exe " + "/DISK:" + disk_letter + "/FUNCTION:Errorscan /QUICKSCAN /START /LOG:scan.txt /QUIT"
    # bad block check
    r = os.system(scan_cmd)
    if (hd_test_debug == 1):
        print(scan_cmd)
    time.sleep(1)
    # read bad block
    with open(hdtune_scan_name, "r+") as handler:
        lines = handler.readlines();
        if(len(lines) < 1):
            tkMessageBox.showinfo(title='scan ', message=u'无任何数据')
            handler.seek(0)
            handler.truncate()
            os._exit(0)
        for read_line in lines:
            read_line = read_line.decode("gb2312")
            if (read_line.find(hd_tital[0]) != -1):
                header, status = split_string(read_line, u':')
                header, status = formate_item_str(header, status)
                item_offset = search_item(hd_tital[0], hd_tital)
                if (hd_test_debug == 1):
                    print("header = %s , satus = %s " % (header, status))
                if cmp(status, hd_stand[item_offset]) != 0:
                    tkMessageBox.showinfo(title='scan ', message=u'坏块错误')
                    handler.seek(0)
                    handler.truncate()
                    os._exit(0)
        handler.seek(0)
        handler.truncate()
    # health check
    r = os.popen(health_cmd)
    time.sleep(2)
    if (hd_test_debug == 1):
        print(health_cmd)
    disk_cap = disk_stand[search_item("capacity", disk_tital)]
    with open(hdtune_health_name, "r+") as handler:
        lines = handler.readlines();
        if(len(lines) < 1):
            tkMessageBox.showinfo(title='health ', message=u'无任何数据')
            handler.seek(0)
            handler.truncate()
            os._exit(0)
        for read_line in lines:
            read_line = read_line.decode("gb2312")
            if(disk_cap != u'0GB'):
                if (read_line.find(hd_tital[2]) != -1) :
                    header, status = split_string(read_line, u':')
                    status = status.split();
                    if (hd_test_debug == 1):
                        print("header = %s , satus = %s " % (header, status))
                    if status[0].find(disk_cap) == -1:
                    #if cmp(disk_cap, status[0]) != 0:
                        tkMessageBox.showinfo(title='capacity ', message=u'容量不一致错误')
                        handler.seek(0)
                        handler.truncate()
                        os._exit(0)
            elif (read_line.find(hd_tital[1]) != -1):
                header, status = split_string(read_line, u':')
                header, status = formate_item_str(header, status)
                item_offset = search_item(hd_tital[1], hd_tital)
                if (hd_test_debug == 1):
                    print("header = %s , satus = %s " % (header, status))
                if cmp(status, hd_stand[item_offset]) != 0:
                    tkMessageBox.showinfo(title='health ', message=u'健康状态错误')
                    handler.seek(0)
                    handler.truncate()
                    os._exit(0)
            if (read_line.find("attention") != -1):
                tkMessageBox.showinfo(title='health ', message=read_line)
                break
        handler.seek(0)
        handler.truncate()
    os.system("pskill.exe HDTunePro.exe")


# *********************************log test result***************************************************************
def log_test_result(log_enable, log_file, disk_no, dev_serial):
    with open(log_file, "w+") as handler:
        if (log_test_debug == 1):
            print("log_file = %s , disk_no = %s " % (log_file, disk_no))
        if log_enable != '1':
            result = tkMessageBox.askokcancel(title='log ', message=u'设备ID:' + disk_no + "####" + u'序列号:' + dev_serial)
        else:
            result = True
        if (True == result):
            if (log_test_debug == 1):
                print("write file")
            # disk_info = u'dev id:' + disk_no + "        " + u'serial:' + dev_serial + '\n'
            disk_info = dev_serial + '\n'
            handler.write(disk_info)
            handler.flush()
            with open(log_result_file, "w+") as result_handler:
                test_result = '1'
                result_handler.write(test_result)
                result_handler.flush()
        else:
            if (log_test_debug == 1):
                print("error: not save")
            with open(log_result_file, "w+") as result_handler:
                test_result = '0'
                result_handler.write(test_result)
                result_handler.flush()


# ****************************************************************************************************************
#
#  main:
#  argv[0] = phsion_disk_check.py
#  argv[1] = log file name
#  argv[2] = set disk no
#  argv[3] = log file
#
# ***************************************************************************************************************
if __name__ == '__main__':
    try:
        # os.remove(log_result_file)
        with open(log_result_file, "w+") as init_result_data:
            init_result_data.write('0')
    except:
        print("remove file %s" % log_result_file)
    try:
        log_file_name = sys.argv[1]
        disk_no = sys.argv[2]
        try:
            log_enable = sys.argv[3]
        except:
            log_enable = None
    except:
        log_file_name = None
        disk_no = None
        log_enable = None
    for i in range(0, len(reverse_file_name)):
        if log_file_name != None:
            if cmp(log_file_name, reverse_file_name[i]) == 0:
                tkMessageBox.showinfo(title='error ', message=u'记录文件名被占用，请重命名')
                os._exit(0)
    # load test standard
    init_dev(setup_file_name)

    # get diskpart tool path  which disk
    disk_part_letter = disk_stand[search_item("diskpart", disk_tital)]
    sata_test_letter = disk_stand[search_item("letter", disk_tital)]
    hd_test_enable = disk_stand[search_item("hdtest", disk_tital)]
    hd_disknum = disk_stand[search_item("disknum", disk_tital)]
    print("disk letter %s \n" % hd_disknum)
    # diskpart：build disk
    disk_part = disk_part_letter + ":\\Windows\\System32\\diskpart.exe /s disk.txt"
    r = os.system(disk_part)
    if r != 0:
        tkMessageBox.showinfo(title='error ', message=u'磁盘状态错误，已清除')
        disk_part = disk_part_letter + ":\\Windows\\System32\\diskpart.exe /s clean.txt"
        r = os.system(disk_part)
        os._exit(0)
    try:
    # write sata test result to file
        with open(sata_file_name , "w+") as sata_handler:
            r = os.popen("sata.exe")
            sata_test_reslut = r.readlines()
            sata_handler.writelines(sata_test_reslut)
            sata_handler.flush()
    except:
        tkMessageBox.showinfo(title='error ', message=u'SATA.txt 不存在脚本退出')
        os._exit(0)
    # compare result
    read_sata_info(sata_file_name)
    result, error_str = compare_sata_test_result()
    if result != True:
        if print_debug == 1:
            print("str not satisfy %s" % error_str)
        sata_test_error(error_str)
    # diskpart：clean disk
    disk_part = disk_part_letter + ":\\Windows\\System32\\diskpart.exe /s clean.txt"
    r = os.system(disk_part)
    # hdtune test
    if hd_test_enable == '1':
        hdtune_test(hd_disknum)
    # write log
    if log_file_name != None:
        dev_serial_no = sata_result[search_item(sata_tital[0], sata_tital)]
        log_test_result(log_enable, log_file_name, disk_no, dev_serial_no)
    os._exit(0)
