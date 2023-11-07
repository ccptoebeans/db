import unittest
import collections
import blue
import sys
import db
sys.modules["dbTest"] = blue.LoadExtension("_dbTest")
import dbTest

# The following test data was selected in order to test
# * python input and output types
# * That the full storage capacity for the db type is usable

# Test types
A_STRING = "TestString"
SMALL_NUMBER = 10

# Date/Time
VALID_YEAR = 2023
VALID_MONTH = 8
VALID_DAY = 19
VALID_HOUR = 14
VALID_MINUTE = 19
VALID_SECOND = 19
VALID_FRACTION = 123

MAX_YEAR = 9999
MAX_MONTH = 12
MAX_DAY = 31
MAX_HOUR = 23
MAX_MINUTE = 59
MAX_SECOND = 59
MAX_FRACTION = 999

MIN_YEAR = 1601
MIN_MONTH = 1
MIN_DAY = 1
MIN_HOUR = 0
MIN_MINUTE = 0
MIN_SECOND = 0
MIN_FRACTION = 0

# DBTYPE_BOOL
# boolean value
DBTYPE_BOOL = dbTest.dbType.BOOL

# DBTYPE_I1
# one-byte signed integer
DBTYPE_I1_TYPE_ID = dbTest.dbType.I1
DBTYPE_I1_MAX = 127
DBTYPE_I1_MIN = -128

# DBTYPE_UI1
# one-byte, unsigned integer
DBTYPE_UI1_TYPE_ID = dbTest.dbType.UI1
DBTYPE_UI1_MAX = 255
DBTYPE_UI1_MIN = 0

# DBTYPE_I2
# two-byte signed integer
DBTYPE_I2_TYPE_ID = dbTest.dbType.I2
DBTYPE_I2_MAX = 32767
DBTYPE_I2_MIN = -32768

# DBTYPE_UI2
# two-byte unsigned integer
DBTYPE_UI2_TYPE_ID = dbTest.dbType.UI2
DBTYPE_UI2_MAX = 65535
DBTYPE_UI2_MIN = 0

# DBTYPE_I4
# four-byte signed integer
DBTYPE_I4_TYPE_ID = dbTest.dbType.I4
DBTYPE_I4_MAX = 2147483647
DBTYPE_I4_MIN = -2147483648

# DBTYPE_UI4
# four-byte unsigned integer
# Note: DBTYPE_UI4_MAX Should be 4294967295 and DBTYPE_UI4_MIN should be 0
# a bug in PyRowSet.cpp in blue where value is read as PyLong_FromLong rather than PyLong_FromUnsignedLong causes this
# bug has always been present
DBTYPE_UI4_TYPE_ID = dbTest.dbType.UI4
DBTYPE_UI4_MAX = 2147483647 
DBTYPE_UI4_MIN = -2147483648

# DBTYPE_I8
# eight-byte signed integer
DBTYPE_I8_TYPE_ID = dbTest.dbType.I8
DBTYPE_I8_MAX = 9223372036854775807
DBTYPE_I8_MIN = -9223372036854775808

# DBTYPE_FILETIME
# 64 bit value - Test max storage capacity
DBTYPE_FILETIME_TYPE_ID = dbTest.dbType.FILETIME

# DBTYPE_UI8
# eight-byte, unsigned integer
DBTYPE_UI8_TYPE_ID = dbTest.dbType.UI8
DBTYPE_UI8_MAX = 18446744073709551615
DBTYPE_UI8_MIN = 0

# DBTYPE_R4
# single-precision floating-point value
DBTYPE_R4_TYPE_ID = dbTest.dbType.R4

# DBTYPE_R8
# double-precision floating-point value
DBTYPE_R8_TYPE_ID = dbTest.dbType.R8
DBTYPE_R8_VALID = 0.12345678912345
DBTYPE_R8_MAX = sys.float_info.max
DBTYPE_R8_MIN = sys.float_info.min

# DBTYPE_CY
# LARGE_INTEGER, Currency is a fixed-point number with four digits to the right of the decimal point. It is stored in an eight-byte signed integer, scaled by 10,000.
# NOTE: Codebase currently truncates to 2DP valid value test changed accordingly
DBTYPE_CY_TYPE_ID = dbTest.dbType.CY
DBTYPE_CY_VALID_VALUE = 12.12
DBTYPE_CY_INVALID_VALUE = 12.12345

# DBTYPE_STR
# null-terminated ANSI/DBCS character string
DBTYPE_STR_TYPE_ID = dbTest.dbType.STR

# DBTYPE_BSTR
# pointer to a BSTR, as in Automation: Typedef WCHAR * BSTR;
DBTYPE_BSTR_TYPE_ID = dbTest.dbType.BSTR

# DBTYPE_WSTR
# null-terminated Unicode character string
DBTYPE_WSTR_TYPE_ID = dbTest.dbType.WSTR

# DBTYPE_BYTES
# A binary data value. That is, an array of bytes
DBTYPE_BYTES_TYPE_ID = dbTest.dbType.BYTES
DBTYPE_BYTES_VALID = bytes([0x00,0x01,0x02,0x03,0x04])

# DBTYPE_IUNKNOWN
# pointer to an IUnknown interface on a COM object
DBTYPE_IUNKNOWN_TYPE_ID = dbTest.dbType.IUNKNOWN
DBTYPE_IUNKNOWN_VALID = bytes([0x00,0x01,0x02,0x03,0x04])

# DBTYPE_DBTIMESTAMP
# Takes a DBTYPE_FILETIME and converts to DBTYPE_DBTIMESTAMP to store in db. Converts back to DBTYPE_FILETIME on retrival
# Format stores Year, Month, Day, hour, minute, second, fraction
DBTYPE_DBTIMESTAMP_TYPE_ID = dbTest.dbType.DBTIMESTAMP

# DBTYPE_DBDATE
# Takes a DBTYPE_FILETIME and converts to DBTYPE_DBDATE to store in db. Converts back to DBTYPE_FILETIME on retrival
# Format stores Year Month day, extra information will be lost
DBTYPE_DBDATE_TYPE_ID = dbTest.dbType.DBDATE

# DBTYPE_DBTIME2
# Takes a DBTYPE_FILETIME and converts to DBTYPE_DBTIME2 to store in db. Converts back to DBTYPE_FILETIME on retrival
# Format Stores Hour Minute second fraction
DBTYPE_DBTIME2_TYPE_ID = dbTest.dbType.DBTIME2

#DBTYPE_INVALID
DBTYPE_INVALID_TYPE_ID = dbTest.dbType.INVALID

class DbUnitTests(unittest.TestCase):

    def setUp(self):
        
        self.testTools = dbTest.TestTools()
        print("Set up done")

    def tearDown(self):
        print("Done")

    def testDbTypeBool(self):

        print("Test type DBTYPE_BOOL")

        #Set db type for test
        dbTypeID = DBTYPE_BOOL

        #Test True
        val = self.testTools.TestParameter(dbTypeID,True)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],True)

        #Test False
        val = self.testTools.TestParameter(dbTypeID,False)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],False)

    def testDbTypeI1(self):

        print("Test type DBTYPE_I1")

        #Set db type for test
        dbTypeID = DBTYPE_I1_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],SMALL_NUMBER)

        #Test max
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_I1_MAX)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_I1_MAX)

        #Test min
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_I1_MIN)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_I1_MIN)

        #Test error states
        with self.assertRaises(RuntimeError):
            #Over range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_I1_MAX+1)

        with self.assertRaises(RuntimeError):
            #Under range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_I1_MIN-1)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeUI1(self):

        print("Test type DBTYPE_UI1")

        #Set db type for test
        dbTypeID = DBTYPE_UI1_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],SMALL_NUMBER)

        #Test max
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI1_MAX)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_UI1_MAX)

        #Test min
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI1_MIN)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_UI1_MIN)

        #Test error states
        with self.assertRaises(RuntimeError):
            #Over range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI1_MAX+1)

        with self.assertRaises(RuntimeError):
            #Under range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI1_MIN-1)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")
        

    def testDbTypeI2(self):
        print("Test type DBTYPE_I2")

        #Set db type for test
        dbTypeID = DBTYPE_I2_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],SMALL_NUMBER)

        #Test max
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_I2_MAX)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_I2_MAX)

        #Test min
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_I2_MIN)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_I2_MIN)

        #Test error states
        with self.assertRaises(RuntimeError):
            #Over range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_I2_MAX+1)

        with self.assertRaises(RuntimeError):
            #Under range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_I2_MIN-1)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")
        
    def testDbTypeUI2(self):
        print("Test type DBTYPE_UI2")

        #Set db type for test
        dbTypeID = DBTYPE_UI2_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],SMALL_NUMBER)

        #Test max
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI2_MAX)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_UI2_MAX)

        #Test min
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI2_MIN)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_UI2_MIN)

        #Test error states
        with self.assertRaises(RuntimeError):
            #Over range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI2_MAX+1)

        with self.assertRaises(RuntimeError):
            #Under range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI2_MIN-1)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeI4(self):
        print("Test type DBTYPE_I4")

        #Set db type for test
        dbTypeID = DBTYPE_I4_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],SMALL_NUMBER)

        #Test max
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_I4_MAX)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_I4_MAX)

        #Test min
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_I4_MIN)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_I4_MIN)

        #Test error states
        with self.assertRaises(RuntimeError):
            #Over range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_I4_MAX+1)

        with self.assertRaises(RuntimeError):
            #Under range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_I4_MIN-1)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")
    
    def testDbTypeUI4(self):
        # NOTE: Type is being treated as signed due to historical bug
        print("Test type DBTYPE_UI4")

        #Set db type for test
        dbTypeID = DBTYPE_UI4_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],SMALL_NUMBER)

        #Test max
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI4_MAX)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_UI4_MAX)

        #Test min
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI4_MIN)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_UI4_MIN)

        #Test error states
        with self.assertRaises(RuntimeError):
            #Over range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI4_MAX+1)

        with self.assertRaises(RuntimeError):
            #Under range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI4_MIN-1)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeI8(self):
        print("Test type DBTYPE_I8")

        #Set db type for test
        dbTypeID = DBTYPE_I8_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],SMALL_NUMBER)

        #Test max
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_I8_MAX)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_I8_MAX)

        #Test min
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_I8_MIN)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_I8_MIN)

        #Test error states
        with self.assertRaises(RuntimeError):
            #Over range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_I8_MAX+1)

        with self.assertRaises(RuntimeError):
            #Under range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_I8_MIN-1)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeFILETIME(self):
        print("Test type DBTYPE_FILETIME")

        #Set db type for test
        dbTypeID = DBTYPE_FILETIME_TYPE_ID

        #Test valid
        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(VALID_YEAR,VALID_MONTH,VALID_DAY,VALID_HOUR,VALID_MINUTE,VALID_SECOND,VALID_FRACTION)
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(VALID_YEAR,filetimeParts[0])
        self.assertEqual(VALID_MONTH,filetimeParts[1])
        self.assertEqual(VALID_DAY,filetimeParts[3])
        self.assertEqual(VALID_HOUR,filetimeParts[4])
        self.assertEqual(VALID_MINUTE,filetimeParts[5])
        self.assertEqual(VALID_SECOND,filetimeParts[6])
        self.assertEqual(VALID_FRACTION,filetimeParts[7])

        #Test max
        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(MAX_YEAR,MAX_MONTH,MAX_DAY,MAX_HOUR,MAX_MINUTE,MAX_SECOND,MAX_FRACTION)
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(MAX_YEAR,filetimeParts[0])
        self.assertEqual(MAX_MONTH,filetimeParts[1])
        self.assertEqual(MAX_DAY,filetimeParts[3])
        self.assertEqual(MAX_HOUR,filetimeParts[4])
        self.assertEqual(MAX_MINUTE,filetimeParts[5])
        self.assertEqual(MAX_SECOND,filetimeParts[6])
        self.assertEqual(MAX_FRACTION,filetimeParts[7])


        #Test min
        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(MIN_YEAR,MIN_MONTH,MIN_DAY,MIN_HOUR,MIN_MINUTE,MIN_SECOND,MIN_FRACTION)
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(MIN_YEAR,filetimeParts[0])
        self.assertEqual(MIN_MONTH,filetimeParts[1])
        self.assertEqual(MIN_DAY,filetimeParts[3])
        self.assertEqual(MIN_HOUR,filetimeParts[4])
        self.assertEqual(MIN_MINUTE,filetimeParts[5])
        self.assertEqual(MIN_SECOND,filetimeParts[6])
        self.assertEqual(MIN_FRACTION,filetimeParts[7])


        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")


    def testDbTypeUI8(self):
        print("Test type DBTYPE_UI8")

        #Set db type for test
        dbTypeID = DBTYPE_UI8_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],SMALL_NUMBER)

        #Test max
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI8_MAX)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_UI8_MAX)

        #Test min
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI8_MIN)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_UI8_MIN)

        #Test error states
        with self.assertRaises(RuntimeError):
            #Over range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI8_MAX+1)

        with self.assertRaises(RuntimeError):
            #Under range
            val = self.testTools.TestParameter(dbTypeID,DBTYPE_UI8_MIN-1)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeR4(self):
        print("Test type DBTYPE_R4")

        #Set db type for test
        dbTypeID = DBTYPE_R4_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(type(val[0]),float)
        self.assertEqual(val[0],SMALL_NUMBER)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeR8(self):
        print("Test type DBTYPE_R8")

        #Set db type for test
        dbTypeID = DBTYPE_R8_TYPE_ID

        #Test small number
        val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(type(val[0]),float)
        self.assertEqual(val[0],SMALL_NUMBER)

        #Test max
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_R8_MAX)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_R8_MAX)

        #Test min
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_R8_MIN)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_R8_MIN)

        #Test precision
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_R8_VALID)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_R8_VALID)

        #Test error states
        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeCY(self):
        print("Test type DBTYPE_CY")

        #Set db type for test
        dbTypeID = DBTYPE_CY_TYPE_ID

        #Test valid number
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_CY_VALID_VALUE)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_CY_VALID_VALUE)

        #Test invalid number
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_CY_INVALID_VALUE)
        self.assertEqual(type(val),blue.DBRow)
        self.assertNotEqual(val[0],DBTYPE_CY_INVALID_VALUE)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeSTR(self):
        print("Test type DBTYPE_STR")

        #Set db type for test
        dbTypeID = DBTYPE_STR_TYPE_ID

        #Test valid string
        val = self.testTools.TestParameter(dbTypeID,A_STRING)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],A_STRING)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)

    def testDbTypeBSTR(self):
        print("Test type DBTYPE_BSTR")

        #Set db type for test
        dbTypeID = DBTYPE_BSTR_TYPE_ID

        #Test valid string
        val = self.testTools.TestParameter(dbTypeID,A_STRING)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],A_STRING)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)

    def testDbTypeWSTR(self):
        print("Test type DBTYPE_WSTR")

        #Set db type for test
        dbTypeID = DBTYPE_WSTR_TYPE_ID

        #Test valid string
        val = self.testTools.TestParameter(dbTypeID,A_STRING)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],A_STRING)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)

    def testDbTypeBYTES(self):
        print("Test type DBTYPE_BYTES")

        #Set db type for test
        dbTypeID = DBTYPE_BYTES_TYPE_ID

        #Test valid string
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_BYTES_VALID)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_BYTES_VALID)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)

    @unittest.skip("DBTYPE_IUNKNOWN is historically broken from 2005. See CL10082") 
    def testDbTypeIUNKNOWN(self):
        print("Test type DBTYPE_IUNKNOWN")

        self.assertEqual(1,2)

        #Set db type for test
        dbTypeID = DBTYPE_IUNKNOWN_TYPE_ID

        #Test valid string
        val = self.testTools.TestParameter(dbTypeID,DBTYPE_IUNKNOWN_VALID)
        self.assertEqual(type(val),blue.DBRow)
        self.assertEqual(val[0],DBTYPE_IUNKNOWN_VALID)

        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,SMALL_NUMBER)

    def testDbTypeDBTIMESTAMP(self):
        print("Test type DBTYPE_TIMESTAMP")

        #Set db type for test
        dbTypeID = DBTYPE_DBTIMESTAMP_TYPE_ID


        #Test valid
        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(VALID_YEAR,VALID_MONTH,VALID_DAY,VALID_HOUR,VALID_MINUTE,VALID_SECOND,VALID_FRACTION)
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(VALID_YEAR,filetimeParts[0])
        self.assertEqual(VALID_MONTH,filetimeParts[1])
        self.assertEqual(VALID_DAY,filetimeParts[3])
        self.assertEqual(VALID_HOUR,filetimeParts[4])
        self.assertEqual(VALID_MINUTE,filetimeParts[5])
        self.assertEqual(VALID_SECOND,filetimeParts[6])
        self.assertEqual(VALID_FRACTION,filetimeParts[7])

        #Test max
        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(MAX_YEAR,MAX_MONTH,MAX_DAY,MAX_HOUR,MAX_MINUTE,MAX_SECOND,MAX_FRACTION)
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(MAX_YEAR,filetimeParts[0])
        self.assertEqual(MAX_MONTH,filetimeParts[1])
        self.assertEqual(MAX_DAY,filetimeParts[3])
        self.assertEqual(MAX_HOUR,filetimeParts[4])
        self.assertEqual(MAX_MINUTE,filetimeParts[5])
        self.assertEqual(MAX_SECOND,filetimeParts[6])
        self.assertEqual(MAX_FRACTION,filetimeParts[7])

        #Test min
        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(MIN_YEAR,MIN_MONTH,MIN_DAY,MIN_HOUR,MIN_MINUTE,MIN_SECOND,MIN_FRACTION)
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(MIN_YEAR,filetimeParts[0])
        self.assertEqual(MIN_MONTH,filetimeParts[1])
        self.assertEqual(MIN_DAY,filetimeParts[3])
        self.assertEqual(MIN_HOUR,filetimeParts[4])
        self.assertEqual(MIN_MINUTE,filetimeParts[5])
        self.assertEqual(MIN_SECOND,filetimeParts[6])
        self.assertEqual(MIN_FRACTION,filetimeParts[7])


        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeDBDATE(self):
        print("Test type DBTYPE_DBDATE")

        #Set db type for test
        dbTypeID = DBTYPE_DBDATE_TYPE_ID

        #Test valid
        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(VALID_YEAR,VALID_MONTH,VALID_DAY,VALID_HOUR,VALID_MINUTE,VALID_SECOND,VALID_FRACTION)
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(VALID_YEAR,filetimeParts[0])
        self.assertEqual(VALID_MONTH,filetimeParts[1])
        self.assertEqual(VALID_DAY,filetimeParts[3])

        #Test max
        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(MAX_YEAR,MAX_MONTH,MAX_DAY,MAX_HOUR,MAX_MINUTE,MAX_SECOND,MAX_FRACTION)
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(MAX_YEAR,filetimeParts[0])
        self.assertEqual(MAX_MONTH,filetimeParts[1])
        self.assertEqual(MAX_DAY,filetimeParts[3])

        #Test min
        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(MIN_YEAR,MIN_MONTH,MIN_DAY,MIN_HOUR,MIN_MINUTE,MIN_SECOND,MIN_FRACTION)
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(MIN_YEAR,filetimeParts[0])
        self.assertEqual(MIN_MONTH,filetimeParts[1])
        self.assertEqual(MIN_DAY,filetimeParts[3])


        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeDBTIME2(self):
        print("Test type DBTYPE_DBTIME2")

        #Set db type for test
        dbTypeID = DBTYPE_DBTIME2_TYPE_ID

        #Construct FILETIME input
        filetime = blue.os.GetTimeFromParts(VALID_YEAR,VALID_MONTH,VALID_DAY,VALID_HOUR,VALID_MINUTE,VALID_SECOND,VALID_FRACTION)

        #Test valid Hour Minute Second Fraction
        val = self.testTools.TestParameter(dbTypeID,filetime)
        self.assertEqual(type(val),blue.DBRow)
        #Convert filetime to parts
        filetimeParts = blue.os.GetTimeParts(val[0])
        self.assertEqual(VALID_HOUR,filetimeParts[4])
        self.assertEqual(VALID_MINUTE,filetimeParts[5])
        self.assertEqual(VALID_SECOND,filetimeParts[6])
        self.assertEqual(VALID_FRACTION,filetimeParts[7])

        #Test error states
        with self.assertRaises(RuntimeError):
            #Incorrect type supplied
            val = self.testTools.TestParameter(dbTypeID,"string")

    def testDbTypeInvalid(self):
        print("Test type DBTYPE_INVALID")

        #Set db type for test
        dbTypeID = DBTYPE_INVALID_TYPE_ID

        with self.assertRaises(RuntimeError):
            #Db type not recognised
            val = self.testTools.TestParameter(dbTypeID,10)

if __name__ == '__main__':
    unittest.main()