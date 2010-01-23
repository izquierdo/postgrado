# -*- coding: utf-8 -*-

from PyQt4 import QtCore

from PyQt4.QtGui import *

class Ui_Main(object):
    def setupUi(self, Main):
        Main.setObjectName("Main")
        Main.resize(602, 402)

        #CentralWid
        self.CentralWid = QWidget(Main)
        self.CentralWid.setObjectName("CentralWid")

        # top-level horizontal splitter
        self.splitter_2 = QSplitter(self.CentralWid)
        self.splitter_2.setOrientation(QtCore.Qt.Horizontal)
        self.splitter_2.setObjectName("splitter_2")

        self.listView = QListView(self.splitter_2)
#        sizePolicy = QSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
#        sizePolicy.setHorizontalStretch(0)
#        sizePolicy.setVerticalStretch(0)
#        sizePolicy.setHeightForWidth(self.listView.sizePolicy().hasHeightForWidth())
#        self.listView.setSizePolicy(sizePolicy)
        self.listView.setObjectName("listView")

        # right side vertical splitter
        self.splitter = QSplitter(self.splitter_2)
        self.splitter.setOrientation(QtCore.Qt.Vertical)
        self.splitter.setObjectName("splitter")

        # main :: main right side :: instance info
        self.instanceInfo_gridLayoutWidget = QWidget(self.splitter)
        self.instanceInfo_gridLayout = QGridLayout(self.instanceInfo_gridLayoutWidget)

        # main :: main right side :: instance info :: query display
        self.queryDisplay = QLineEdit(self.instanceInfo_gridLayoutWidget)
        self.queryDisplay.setReadOnly(True)
        self.queryDisplay.setObjectName("queryDisplay")
        self.instanceInfo_gridLayout.addWidget(self.queryDisplay)

#        self.horizontalLayoutWidget = QWidget(self.splitter)
#        self.horizontalLayoutWidget.setObjectName("horizontalLayoutWidget")
#        self.horizontalLayout = QHBoxLayout(self.horizontalLayoutWidget)
#        self.horizontalLayout.setObjectName("horizontalLayout")
#        self.label = QLabel(self.horizontalLayoutWidget)
#        self.label.setObjectName("label")
#        self.horizontalLayout.addWidget(self.label)


#        self.horizontalLayout.addWidget(self.queryDisplay)

        self.horizontalLayoutWidget_2 = QWidget(self.splitter)
        self.horizontalLayoutWidget_2.setObjectName("horizontalLayoutWidget_2")
        self.horizontalLayout_2 = QHBoxLayout(self.horizontalLayoutWidget_2)
        self.horizontalLayout_2.setObjectName("horizontalLayout_2")

#        self.horizontalLayoutWidget_3 = QWidget(self.splitter)
#        self.horizontalLayoutWidget_3.setObjectName("horizontalLayoutWidget_3")
#        self.horizontalLayout_3 = QHBoxLayout(self.horizontalLayoutWidget_3)
#        self.horizontalLayout_3.setObjectName("horizontalLayout_3")

        Main.setCentralWidget(self.CentralWid)

        self.retranslateUi(Main)
        QtCore.QMetaObject.connectSlotsByName(Main)

    def retranslateUi(self, Main):
        Main.setWindowTitle(QApplication.translate("Main", "MainWindow", None, QApplication.UnicodeUTF8))
        #self.label.setText(QApplication.translate("Main", "Queryx:", None, QApplication.UnicodeUTF8))

