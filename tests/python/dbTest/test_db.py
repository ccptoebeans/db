import unittest
import collections
import sys
import db
import os

CONNECTION_INFO = {
    "uid": "zzp_user",
    "database": "NOTSET",
    "application_name": "eveDB2 test OLE DB",
    "server": "SqlDev1IS",
    "pwd": "zzp_user",
    "provider": "MSOLEDBSQL"
}

CONNECTION_STRING = ""

class Schema:
    def __init__(self):
        self.procedures = {}

    def __getattr__(self, procedureName):
        procedure = self.procedures.get(procedureName)

        if not procedure:
            raise AttributeError(procedureName)

        return procedure

class DbUnitTests(unittest.TestCase):

    def setUp(self):
        self.session = db.NSession(CONNECTION_STRING)
        print("Set up done")

    def tearDown(self):
        print("Done")

    @unittest.skip("Requires localdb")
    def testGetSchema(self):
        schema = self.session.GetSchema(False)
        self.assertTrue(len(schema) > 0)

    @unittest.skip("Requires localdb")
    def testExcuteProcWithStrArgument(self):
        self.session.allowSync = 1
        procs, tables = self.session.GetSchema(False)
        schemas = collections.defaultdict(Schema)
        for schemaProcName, proc in procs.items():
            if "." in schemaProcName:
                schemaName, procName = schemaProcName.split(".")
                schemas[schemaName].procedures[procName] = schemaProcName

        # Execute SQL proc
        zsystemSchema = schemas['zsystem']
        zsystemSQLProcName = zsystemSchema.procedures['SQL']
        ret = self.session.Execute(zsystemSQLProcName, ["SELECT a = NULL"])
        self.assertTrue(ret != None)    

    @unittest.skip("Requires localdb")
    def testGetSessionStatus(self):
        sessionStatus = self.session.GetSessionStatus()
        self.assertEqual(sessionStatus['sessionCount'], 1)
        self.assertEqual(sessionStatus['sessionsInUse'], 0)
        self.assertEqual(sessionStatus['freeSessions'], 1)

    @unittest.skip("Requires localdb")
    def testGetSessionSettings(self):
        sessionsettings = self.session.GetSessionSettings()
        self.assertTrue(isinstance(sessionsettings['cleanEvery'], float))
        self.assertTrue(isinstance(sessionsettings['maxSessions'], int))
        self.assertTrue(isinstance(sessionsettings['maxFreeSessions'], int))
        self.assertTrue(isinstance(sessionsettings['minFreeSessions'], int))

    @unittest.skip("Requires localdb")
    def testSetSessionSettings(self):
        #NOTE: Passed in values are not currently type checked
        targetCleanEveryValue = 11.0
        targetMaxSessions = 33
        targetMaxFreeSessions = 9
        targetMinFreeSessions = 3
        self.session.SetSessionSettings({'cleanEvery':targetCleanEveryValue, 'maxSessions':targetMaxSessions, 'maxFreeSessions':targetMaxFreeSessions, 'minFreeSessions':targetMinFreeSessions})

        sessionsettings = self.session.GetSessionSettings()

        self.assertEqual(sessionsettings['cleanEvery'], targetCleanEveryValue)
        self.assertEqual(sessionsettings['maxSessions'], targetMaxSessions)
        self.assertEqual(sessionsettings['maxFreeSessions'], targetMaxFreeSessions)
        self.assertEqual(sessionsettings['minFreeSessions'], targetMinFreeSessions)

if __name__ == '__main__':

    # Parse arguments from environment variables
    CONNECTION_INFO["uid"] = os.environ.get('UID', CONNECTION_INFO["uid"])
    CONNECTION_INFO["database"] = os.environ.get('DATABASE', CONNECTION_INFO["database"])
    CONNECTION_INFO["application_name"] = os.environ.get('APPLICATION_NAME', CONNECTION_INFO["application_name"])
    CONNECTION_INFO["server"] = os.environ.get('SERVER', CONNECTION_INFO["server"])
    CONNECTION_INFO["pwd"] = os.environ.get('PWD', CONNECTION_INFO["pwd"])
    CONNECTION_INFO["provider"] = os.environ.get('PROVIDER', CONNECTION_INFO["provider"])

    CONNECTION_STRING = "uid={0};database={1};application name={2};server={3};pwd={4};provider={5}".format(
        CONNECTION_INFO["uid"],
        CONNECTION_INFO["database"],
        CONNECTION_INFO["application_name"],
        CONNECTION_INFO["server"],
        CONNECTION_INFO["pwd"],
        CONNECTION_INFO["provider"]
    )

    unittest.main()