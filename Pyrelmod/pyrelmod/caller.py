#! /usr/bin/env python3
# -*- coding: utf-8 -*- 
__author__ = 'Lunzhy'
import subprocess
import os

Default_param_file = 'default.param'
Command_name = 'Cpprelmod.exe'


def call_cpprelmod(prj_path, charge_file):
    default_param = Default_param_file
    exe_file = Command_name
    command = '%s %s %s %s' % (exe_file, default_param, prj_path, charge_file)
    print(command)
    subprocess.call(command.split(' '))
    return