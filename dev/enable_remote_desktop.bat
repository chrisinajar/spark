@setlocal enableextensions enabledelayedexpansion
@echo off

echo "Enabling remote desktop on fleet... %1"

aws gamelift update-fleet-port-settings --fleet-id  "%1" --inbound-permission-authorizations "FromPort=3389,ToPort=3389,IpRange=71.236.194.134/32,Protocol=TCP"

FOR /F "delims=" %%i IN ('aws gamelift describe-instances --fleet-id "%1" ^| findstr InstanceId') DO (SET readLine=%%i)

set instanceId=%readLine:~27,-2%

FOR /F "delims=" %%i IN ('aws gamelift get-instance-access --fleet-id "%1" --instance-id "%instanceId%" ^| findstr "IpAddress"') DO (SET readLine=%%i)
FOR /F "delims=" %%i IN ('aws gamelift get-instance-access --fleet-id "%1" --instance-id "%instanceId%" ^| findstr "UserName"') DO (echo %%i)
FOR /F "delims=" %%i IN ('aws gamelift get-instance-access --fleet-id "%1" --instance-id "%instanceId%" ^| findstr "Secret"') DO (echo %%i)

set ServerName=%readLine:~22,-2%

echo %ServerName%

mstsc.exe /v:%ServerName%

endlocal
