import os
import subprocess
from os.path import dirname
from typing import Optional

from PyQt5.QtWidgets import (QMainWindow, QFileDialog)
from PyQt5.QtCore import Qt
from PyQt5 import QtGui, QtCore

from config import NaoStatusColors, PROJECT_NAME
from debugger import BembelDbugFactory

from widgets.data_canvas import DataCanvas

import numpy as np
from scipy import misc as sp_misc

#  0.299  0.587  0.144
# -0.169 -0.311  0.499  + 128
#  0.498 -0.419 -0.0813 + 128

def RGB2YUV(r,g,b):
    y = int( 0.299*r + 0.587*g + 0.144 *b)
    u = int(-0.169*r - 0.331*g + 0.499 *b + 128)
    v = int( 0.498*r - 0.419*g - 0.0813*b + 128)
    # 0.966     1.337     0.7243   
    assert y < 256 and y >= 0
    assert u < 256 and u >= 0
    assert v < 256 and v >= 0

    return y, u, v

def YUV2RGB(y, u, v):
    u = u - 128
    v = v - 128







    # y = 0.299*r + 0.587*g + 0.144*b
    # if (x%2 == 0)
    #   u = -0.169*r -0.331 * g + 0.499 * b + 128 
    # else
    #   v = (0.498 * r - 0.419 * g - 0.0813 * b + 128




class PatchProcessor(QMainWindow):
  def __init__(self):
    QMainWindow.__init__(self)
    self.file_list = []
    self.current_file = 0
    self.data = None
    self.test()

  def test(self):
    data = np.array([[[255,0,0],[0,255,0],[0,0,255]],[[255,255,0],[0,255,255],[255,0,255]]])
    #data = np.random.randint(0, 255, (5, 4), np.uint8)
    self.canvas = DataCanvas()
    self.setCentralWidget(self.canvas)
    #testytestytest
    self.open_files()
    

    
    #img = QtGui.QImage(data, 100, 100, 300, QtGui.QImage.Format_RGB888)
    #self.setCentralWidget(img)



  def loadImage(self, filename):
    data = sp_misc.imread(filename)
    y = data[:,:,2].astype(np.float)
    u = data[:,:,1].astype(np.float)
    v = data[:,:,0].astype(np.float)
    #data = np.reshape(self.data, (self.data.shape[0], int(self.data.shape[1]*3)))
    self.data = data
    u[:,:] -= 128
    v[:,:] -= 128



    #B = 1.164(Y - 16)                   + 2.018(U - 128)
    #G = 1.164(Y - 16) - 0.813(V - 128) - 0.391(U - 128)
    #R = 1.164(Y - 16) + 1.596(V - 128)
    #b = 1.164*(y[:,:]-16)                      + 2.018*(u[:,:]-128)
    #g = 1.164*(y[:,:]-16) - 0.813*(v[:,:] -128) - 2.018*(u[:,:]-128)
    #r = 1.164*(y[:,:]-16) + 1.596*(v[:,:]-128)

    #r = y[:,:] + 1.403*u[:,:]
    #g = y - 0.344*u[:,:] - 0.714*v[:,:]
    #b = y + 1.770*v[:,:]

    #r = y[:,:] + 1.14*u[:,:]
    #g = y - 0.395*u[:,:] - 0.581*v[:,:]
    #b = y + 2.032*u[:,:]

    r = y[:,:]*0.973962 + u[:,:]*-0.0520438 + v[:,:]*1.4056
    g = y[:,:]*0.96889 + u[:,:]*-0.396288 + v[:,:]*-0.716206
    b = y[:,:]*0.972549 + u[:,:]*1.72351 + v[:,:]*0.000965744



    # framework/frontend/src/vision/image/yuv422.cpp YuvImage::setRGB
    # y = 0.299*r + 0.587*g + 0.144*b
    # if (x%2 == 0)
    #   u = -0.169*r -0.331 * g + 0.499 * b + 128 
    # else
    #   v = (0.498 * r - 0.419 * g - 0.0813 * b + 128

    # 



    r = np.reshape( r, (r.shape[0],r.shape[1],1) )
    g = np.reshape( g, (g.shape[0],g.shape[1],1) )
    b = np.reshape( b, (b.shape[0],b.shape[1],1) )
    rgb = np.concatenate([r,g,b], axis=2)
    rgb = np.clip(rgb, 0, 255)
    rgb = rgb.astype(np.uint8)
    print(rgb)

    yuv = np.concatenate( [y,u,v], axis=1)


    self.data = yuv
    self.canvas.setRawRGB(rgb)
    #self.canvas.set2D(yuv)
    self.update()


  def open_files(self):
    self.file_list = QFileDialog.getOpenFileNames(self, 'Open file', '.' ,"Patches (*.png)")[0]
    self.current_file = 0
    self.loadImage(self.file_list[self.current_file])

  def nextImage(self):
      self.current_file += 1
      self.current_file %= len(self.file_list)
      self.loadImage(self.file_list[self.current_file])

  def prevImage(self):
      self.current_file -= 1
      self.current_file %= len(self.file_list)
      self.loadImage(self.file_list[self.current_file])


    
     

  def keyPressEvent(self, event):
    key = event.key()
    if key == Qt.Key_Left:
      print('Left Arrow Pressed')
    elif key == Qt.Key_Q:
      exit()
    elif key == Qt.Key_O:
      self.open_files()
    elif key == Qt.Key_Down:
      self.nextImage()
    elif key == Qt.Key_Up:
      self.prevImage()







if __name__ == "__main__":
    app = QtGui.QApplication([])
    window = Patchifier(sys.argv[1:])
    window.show()
    sys.exit(app.exec_())
