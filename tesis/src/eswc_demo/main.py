import sys

from PyQt4.QtGui import *

from ui.prueba import Ui_Main

class MyForm(QMainWindow):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)
        self.ui = Ui_Main()
        self.ui.setupUi(self)

        #queryDisplay = self.ui.CentralWid.findChild(QLineEdit, "queryDisplay")
        #queryDisplay.setText("pepe")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    myapp = MyForm()
    myapp.show()
    sys.exit(app.exec_())
