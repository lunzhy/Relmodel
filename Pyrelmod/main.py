#! /usr/bin/env python3
# -*- coding: utf-8 -*- 
__author__ = 'Lunzhy'
import argparse
from pyrelmod.utils import clean_project, prepare_project
from pyrelmod.caller import call_cpprelmod


def do_solve(prj_path, charge_file):
    call_cpprelmod(prj_path, charge_file)
    return


def do_clean(prj_path):
    clean_project(prj_path)
    return


def do_prepare(prj_path):
    prepare_project(prj_path)
    return


def main():
    parser = argparse.ArgumentParser(prog='relmodel')
    subparsers = parser.add_subparsers()

    parser_solve = subparsers.add_parser('solve')
    parser_solve.add_argument('project_to_solve')
    parser_solve.add_argument('charge_file')

    parser_clean = subparsers.add_parser('clean')
    parser_clean.add_argument('project_to_clean')

    parser_prepare = subparsers.add_parser('prepare')
    parser_prepare.add_argument('project_path')

    args = parser.parse_args()

    if 'charge_file' in args:  # solve
        do_solve(args.project_to_solve, args.charge_file)
    elif 'project_to_clean' in args:  # clean
        do_clean(args.project_to_clean)
    elif 'project_path' in args:
        do_prepare(args.project_path)


if __name__ == '__main__':
    main()