@echo off
echo ���python��������
set  regpath=HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Session Manager\Environment
set  pythonpath=X:\Python27;
set  PATH=%PATH%;%pythonpath%
reg  add "%regpath%" /v "Path" /t REG_EXPAND_SZ /d "%PATH%" /f
pause>nul