@echo off
echo Checking out project and filters file
p4 edit db_v141.vcxproj
echo Regenerating
..\..\..\..\..\..\shared_tools\python\27\python.exe ..\..\tools\ProjectFileGenerator\ProjectFileGenerator.py -i db.ccpproj --toolset=v141
pause
