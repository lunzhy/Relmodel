#! /usr/bin/env python3
# -*- coding: utf-8 -*- 
__author__ = 'Lunzhy'
import numpy as np
import matplotlib.pyplot as plt
import scipy.interpolate
import os


def read_data_2d(file, skip=1):
    convert_cm_to_nm = 1e7
    data = np.loadtxt(file, skiprows=skip)
    cols = data.shape[1]  # return ndarray shape
    x, y = data[:, 0], data[:, 1]
    x, y = x * convert_cm_to_nm, y * convert_cm_to_nm
    values = (x, y)
    for col in range(2, cols):
        values = values + (data[:, col],)
    return values


def plot_potential(ax, prj_path, file_name):
    file_path = os.path.join(prj_path, 'Potential', file_name)
    x, y, potential = read_data_2d(file_path, 1)
    xi, yi = np.linspace(min(x), max(x), 200), np.linspace(min(y), max(y), 2000)
    grid_x, grid_y = np.meshgrid(xi, yi)
    grid_z = scipy.interpolate.griddata((x, y), potential, (grid_x, grid_y), method='linear')
    im = ax.imshow(grid_z, cmap=plt.cm.jet, vmin=0.05, vmax=0.95, origin='lower',
                   extent=[min(x), max(x), min(y), max(y)], aspect='auto')
    return im


def plot_charge(ax, prj_path, file_name):
    file_path = os.path.join(prj_path, 'Trap', file_name)
    x, y, trapped = read_data_2d(file_path, 1)
    x_scatter, y_scatter = [], []
    for x_coord, y_coord, charge in zip(x, y, trapped):
        if not charge == 0:
            x_scatter.append(x_coord)
            y_scatter.append(y_coord)
    ax.scatter(x_scatter, y_scatter, facecolors='none', edgecolors='black', s=50)
    return


def test_plot(prj_path):
    fig = plt.figure()
    ax = fig.add_axes([0.1, 0.1, 0.8, 0.8])
    im = plot_potential(ax, prj_path, 'test.potential')
    plot_charge(ax, prj_path, 'test.trap')
    plt.colorbar(im)
    plt.show()
    return


if __name__ == '__main__':
    test_plot(r'E:\PhD Study\Relmodel\Demo')


