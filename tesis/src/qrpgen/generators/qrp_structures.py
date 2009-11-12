class Argument:
    def __init__(self, type, name):
        """
        type may be 'var' or 'const'
        name should be a number
        """

        self.type = type
        self.name = str(name)

    def __str__(self):
        if self.type == 'var':
            return 'X' + self.name

        if self.type == 'const':
            return self.name

class Table:
    def __init__(self, name, args):
        self.name = name
        self.args = args

    def __str__(self):
        arglist = ",".join([str(arg) for arg in self.args])

        return "%s(%s)" % (self.name, arglist)

class Query:
    def __init__(self, qt, sos):
        self.qt = qt
        self.sos = sos

    def __str__(self):
        solist = ",".join([str(so) for so in self.sos])

        return "%s :- %s" % (str(self.qt), solist)

