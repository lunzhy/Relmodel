#! /usr/bin/env python3
# -*- coding: utf-8 -*- 
__author__ = 'Lunzhy'
import os

Data_folder = ['Potential', 'Band', 'Substrate', 'Miscellaneous', 'ElecField']


def is_file_remain(filename):
    if 'user.param' in filename:
        return True
    if os.path.basename(filename) == 'charge':
        return True
    return False


def del_files(path):
    for file in os.listdir(path):
        file_path = os.path.join(path, file)
        if os.path.isfile(file_path):
            os.remove(file_path)
    return


def prepare_project(prj_path):
    if os.path.exists(prj_path):
        print('Project exists.')
    for folder in Data_folder:
        folder_path = os.path.join(prj_path, folder)
        if not os.path.exists(folder_path):
            os.makedirs(folder_path)
    return


def clean_project(prj_path):
    for folder in Data_folder:
        folder_path = os.path.join(prj_path, folder)
        del_files(folder_path)
    return


def main():
    prepare_project(r'E:\PhD Study\Relmodel\Demo')
    return


if __name__ == '__main__':
    main()