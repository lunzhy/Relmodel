#! /usr/bin/env python3
# -*- coding: utf-8 -*- 
__author__ = 'Lunzhy'
from numpy import random as nrandom
import os


def put(prj_path, grid_with, grid_thick):
    vert_count = (grid_with + 1) * (grid_thick + 1)

    charge_ids = []
    for i in range(50):
        charge_id = nrandom.random_integers(0, vert_count - 1)
        charge_ids.append(charge_id)

    with open(os.path.join(prj_path, 'test'), 'w') as f:
        f.write('# charge\n')
        for vert_id in range(vert_count):
            if vert_id in charge_ids:
                charge = 1
            else:
                charge = 0
            line = '%s\t%s\n' % (vert_id, charge)
            f.write(line)
    return


if __name__ == '__main__':
    put(r'E:\PhD Study\Relmodel\Demo', grid_with=100, grid_thick=2000)