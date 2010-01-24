# -*- coding: utf-8 -*-

from PyQt4 import QtCore

from PyQt4.QtGui import *

class Ui_Main(object):
    def setupUi(self, Main):
        Main.setObjectName("Main")
        Main.resize(602, 402)

        # CentralWidget
        self.CentralWidget = QWidget(Main)

        # top
        self.topSplitter = QSplitter(self.CentralWidget)
        self.topSplitter.setOrientation(QtCore.Qt.Horizontal)

        # top :: left side
        self.listView = QListWidget(self.topSplitter)
#        sizePolicy = QSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
#        sizePolicy.setHorizontalStretch(0)
#        sizePolicy.setVerticalStretch(0)
#        sizePolicy.setHeightForWidth(self.listView.sizePolicy().hasHeightForWidth())
#        self.listView.setSizePolicy(sizePolicy)
        self.listView.setObjectName("listView")

        # top :: right side
        self.rightSplitter = QSplitter(self.topSplitter)
        self.rightSplitter.setOrientation(QtCore.Qt.Vertical)

        # top :: right side :: instance info
        self.instanceInfo_LayoutWidget = QWidget(self.rightSplitter)
        self.instanceInfo_Layout = QGridLayout(self.instanceInfo_LayoutWidget)

        # top :: right side :: instance info :: query display label and box
        self.queryDisplayLabel = QLabel(self.instanceInfo_LayoutWidget)
        self.instanceInfo_Layout.addWidget(self.queryDisplayLabel, 0, 0)

        self.queryDisplayBox = QLineEdit(self.instanceInfo_LayoutWidget)
        self.queryDisplayBox.setReadOnly(True)
        self.queryDisplayBox.setObjectName("queryDisplayBox")
        self.instanceInfo_Layout.addWidget(self.queryDisplayBox, 0, 1)

        # top :: right side :: instance info :: views display label and box
        self.viewsDisplayLabel = QLabel(self.instanceInfo_LayoutWidget)
        self.instanceInfo_Layout.addWidget(self.viewsDisplayLabel, 1, 0)

        self.viewsDisplayBox = QListView(self.instanceInfo_LayoutWidget)
        self.viewsDisplayBox.setObjectName("viewsDisplayBox")
        self.instanceInfo_Layout.addWidget(self.viewsDisplayBox, 1, 1)

        # top :: right side :: solutions
        self.solutions_LayoutWidget = QWidget(self.rightSplitter)
        self.solutions_Layout = QVBoxLayout(self.solutions_LayoutWidget)

        # top :: right side :: solutions :: dnnf
        self.dnnf_Widget = QGroupBox()
        self.dnnf_Widget.setTitle("Intermediate Representation")
        self.solutions_Layout.addWidget(self.dnnf_Widget)

        self.dnnf_Layout = QVBoxLayout()
        self.dnnf_Widget.setLayout(self.dnnf_Layout)

        # top :: right side :: solutions :: dnnf :: dnnf label layout
        self.dnnf_LabelWidget = QWidget()
        self.dnnf_Layout.addWidget(self.dnnf_LabelWidget)

        self.dnnf_LabelLayout = QHBoxLayout()
        self.dnnf_LabelWidget.setLayout(self.dnnf_LabelLayout)

        # top :: right side :: solutions :: dnnf :: dnnf label layout :: dnnf label
        self.dnnfLabel = QLabel(self.dnnf_Widget)
        self.dnnf_LabelLayout.addWidget(self.dnnfLabel)

        # top :: right side :: solutions :: dnnf :: dnnf label layout :: dnnf status label
        self.dnnfStatusLabel = QLabel(self.dnnf_Widget)
        self.dnnf_LabelLayout.addWidget(self.dnnfStatusLabel)

        # top :: right side :: solutions :: dnnf :: dnnf buttons layout
        self.dnnf_ButtonsWidget = QWidget()
        self.dnnf_Layout.addWidget(self.dnnf_ButtonsWidget)

        self.dnnf_ButtonsLayout = QHBoxLayout()
        self.dnnf_ButtonsWidget.setLayout(self.dnnf_ButtonsLayout)

        # top :: right side :: solutions :: dnnf :: dnnf buttons layout :: dnnf genandload button
        self.dnnfGenAndLoadButton = QPushButton("Generate && Load")
        self.dnnf_ButtonsLayout.addWidget(self.dnnfGenAndLoadButton)

        self.dnnfLoadFromFileButton = QPushButton("Load From File")
        self.dnnf_ButtonsLayout.addWidget(self.dnnfLoadFromFileButton)

        # top :: right side :: solutions :: rewritings
        self.rewritings_Widget = QGroupBox()
        self.rewritings_Widget.setTitle("Rewritings")
        self.solutions_Layout.addWidget(self.rewritings_Widget)

        self.rewritings_Layout = QVBoxLayout()
        self.rewritings_Widget.setLayout(self.rewritings_Layout)

        # top :: right side :: solutions :: rewritings :: irx label
        self.irxLabel = QLabel(self.dnnf_Widget)
        self.rewritings_Layout.addWidget(self.irxLabel)


#        self.horizontalLayoutWidget = QWidget(self.rightSplitter)
#        self.horizontalLayoutWidget.setObjectName("horizontalLayoutWidget")
#        self.horizontalLayout = QHBoxLayout(self.horizontalLayoutWidget)
#        self.horizontalLayout.setObjectName("horizontalLayout")
#        self.label = QLabel(self.horizontalLayoutWidget)
#        self.label.setObjectName("label")
#        self.horizontalLayout.addWidget(self.label)


#        self.horizontalLayout.addWidget(self.queryDisplay)

#        self.horizontalLayoutWidget_3 = QWidget(self.rightSplitter)
#        self.horizontalLayoutWidget_3.setObjectName("horizontalLayoutWidget_3")
#        self.horizontalLayout_3 = QHBoxLayout(self.horizontalLayoutWidget_3)
#        self.horizontalLayout_3.setObjectName("horizontalLayout_3")

        Main.setCentralWidget(self.CentralWidget)

        self.retranslateUi(Main)
        QtCore.QMetaObject.connectSlotsByName(Main)

    def retranslateUi(self, Main):
        Main.setWindowTitle(QApplication.translate("Main", "SsdSat Demo", None, QApplication.UnicodeUTF8))
        self.queryDisplayLabel.setText(QApplication.translate("Main", "Workflow", None, QApplication.UnicodeUTF8))
        self.viewsDisplayLabel.setText(QApplication.translate("Main", "Concrete Services", None, QApplication.UnicodeUTF8))

        self.dnnfLabel.setText(QApplication.translate("Main", "d-DNNF: loaded", None, QApplication.UnicodeUTF8))
        #TODO activate next label
        #self.dnnfStatusLabel.setText(QApplication.translate("Main", "loaded", None, QApplication.UnicodeUTF8))

        self.irxLabel.setText(QApplication.translate("Main", "REP", None, QApplication.UnicodeUTF8))

