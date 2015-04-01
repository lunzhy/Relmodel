#! /usr/bin/env python3
# -*- coding: utf-8 -*- 
__author__ = 'Lunzhy'
from cx_Freeze import setup, Executable

exe = Executable(
    script="main.py",
    targetName='relmodel.exe',
)

includefiles=['E:\MyCode\Relmodel\Cpprelmod\Release\Cpprelmod.exe',
              'E:\MyCode\Relmodel\Cpprelmod\Cpprelmod\default.param']

setup(
    name = "Pyrelmod",
    options = {"build_exe": {"include_files":includefiles}},
    executables = [exe]
)