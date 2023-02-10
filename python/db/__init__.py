import blue
import sys

sys.modules[__name__] = blue.LoadExtension("_db")
