import unittest
import collections
import db
import os
from test import support

import blue
import stackless


CONNECTION_STRING = os.environ.get('CCP_LOCALDB_CONNECTION_STRING', None)

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
        if CONNECTION_STRING != None:
            self.session = db.NSession(CONNECTION_STRING)
        print("Set up done")

    def tearDown(self):
        print("Done")

    @unittest.skipIf(not CONNECTION_STRING, "Skipping because it requires a local db") 
    def testGetSchema(self):
        schema = self.session.GetSchema(False)
        self.assertTrue(len(schema) > 0)

    @unittest.skipIf(not CONNECTION_STRING, "Skipping because it requires a local db") 
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

    @unittest.skipIf(not CONNECTION_STRING, "Skipping because it requires a local db") 
    def testGetSessionStatus(self):
        sessionStatus = self.session.GetSessionStatus()
        self.assertEqual(sessionStatus['sessionCount'], 1)
        self.assertEqual(sessionStatus['sessionsInUse'], 0)
        self.assertEqual(sessionStatus['freeSessions'], 1)

    @unittest.skipIf(not CONNECTION_STRING, "Skipping because it requires a local db") 
    def testGetSessionSettings(self):
        sessionsettings = self.session.GetSessionSettings()
        self.assertTrue(isinstance(sessionsettings['cleanEvery'], float))
        self.assertTrue(isinstance(sessionsettings['maxSessions'], int))
        self.assertTrue(isinstance(sessionsettings['maxFreeSessions'], int))
        self.assertTrue(isinstance(sessionsettings['minFreeSessions'], int))

    @unittest.skipIf(not CONNECTION_STRING, "Skipping because it requires a local db") 
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


def main():
    exc = None

    def wrap_run(testcase):
        nonlocal exc
        try:
            support.run_unittest(testcase)
        except Exception as e:
            exc = e

    t = stackless.tasklet(wrap_run)(DbUnitTests)
    while t.alive:
        blue.os.Pump()
    if exc:
        raise exc


if __name__ == "__main__":
    main()
