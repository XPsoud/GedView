@echo off
SET src=
FOR %%f IN (*.png) DO (
	call :AddFile %%f
)
bin2code.exe %src% -s ..\..\sources\toolbaricons
pause
goto :EOF

:AddFile
SET src2=%src% %1
SET src=%src2%