import sys

from PyQt4.QtGui import *

from ui.prueba import Ui_Main

class MyForm(QMainWindow):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.ui = Ui_Main()
        self.ui.setupUi(self)

        queryDisplay = self.ui.CentralWidget.findChild(QLineEdit, "queryDisplayBox")
        queryDisplay.setText("q(X1,X2) :- r(X1,X3),r(X3,X4),r(X4,X5),r(X5,X2)")

        listView = self.ui.CentralWidget.findChild(QListWidget, "listView")

        if listView == None:
            print "sno"

        listView.insertItem(0, QListWidgetItem("60 views", listView))
        listView.insertItem(0, QListWidgetItem("80 views", listView))
        listView.insertItem(0, QListWidgetItem("100 views", listView))

        #parte de archivo TODO
        viewsDisplayBox = self.ui.CentralWidget.findChild(QListWidget, "viewsDisplayBox")

        f = open("views-60-5.txt")

        for line in f:
            viewsDisplayBox.insertItem(0, QListWidgetItem(line))

        f.close()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    myapp = MyForm()
    myapp.show()
    sys.exit(app.exec_())
