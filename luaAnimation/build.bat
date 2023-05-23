@echo off
set s=scripts
set script=json2lua.lua
set r=%~dp1
set n=%~n1
set e=%~x1
set f=%n%%e%
@REM echo "-----start" >> d:\out.txt
if exist %r%%s% ( 
    cd %r%%s%
)

lua %script% %f%
exit