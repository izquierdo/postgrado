import sys

from PyQt4.QtGui import *

from ui.prueba import Ui_Main

class MyForm(QMainWindow):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.ui = Ui_Main()
        self.ui.setupUi(self)

        queryDisplay = self.ui.CentralWidget.findChild(QLineEdit, "queryDisplayBox")
        queryDisplay.setText("W(x, w, y, z) :- uscity(x), flight(x, w), flight(w, y), flight(y, z), flight(z, x)")

        listView = self.ui.CentralWidget.findChild(QListWidget, "listView")

        if listView == None:
            print "sno"

        listView.insertItem(0, QListWidgetItem("60 views", listView))
        listView.insertItem(0, QListWidgetItem("80 views", listView))
        listView.insertItem(0, QListWidgetItem("100 views", listView))

        #parte de archivo TODO
        viewsDisplayBox = self.ui.CentralWidget.findChild(QListWidget, "viewsDisplayBox")

        #f = open("views-60-5.txt")
        f = open("views-paper.txt")
        lines = [l for l in f]
        f.close()

        lines.reverse()

        for s in lines:
            viewsDisplayBox.insertItem(0, QListWidgetItem(s.strip()))

        f.close()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    myapp = MyForm()
    myapp.show()
    sys.exit(app.exec_())
