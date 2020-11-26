@echo off
setlocal EnableDelayedExpansion
chcp 65001 >NUL

@rem Confirm this is the directory to search
set proceed=0
echo This script will remove any non-ASCII characters from the
echo names of EVERY file found in '%CD%'.
echo|set /p="Do you wish to continue "
choice

if "%ERRORLEVEL%" == "1" (
    set proceed=1
)

if "%proceed%" == "0" (
    echo Exited without renaming any files
    pause
    exit /b 0
)

@REM Get a list of all files within this directory
set count=0
set i=0
for /F "tokens=*" %%g in (
    'dir /s/b/a-d %CD%'
) do (
    set /A i+=1
    set paths[!i!]=%%g
)
set count=%i%

@REM Iterate over each file, replacing any 'bad' characters with underscores
set changed=0
set dir=""
set file=""
set i=1
set "permitted=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 !#$%%^^&'()+,-.=@[]_{}"
:RenameLoop
    @REM Split into directory and file
    for %%P in ("!paths[%i%]!") do (
        set dir=%%~dpP
        set file=%%~nxP
    )

    @REM Replace necessary characters
    set tmp=!file!
    set new=
    set pos=0
    :NextChar
        for /f "delims=*~ eol=*" %%C in ("!tmp:~0,1!") do (
            if "!permitted:%%C=!" NEQ "!permitted!" (
                set "new=!new!%%C"
            ) else (
                set "new=!new!_"
            )
        )
        set "tmp=!tmp:~1!"
        if "!tmp!" NEQ "" (
            goto NextChar
        )

    @REM Rename file if changed
    if "!file!" NEQ "!new!" (
        rename "!dir!!file!" "!new!"
        set /a changed=changed+1
        echo !dir!{!file! ==^> !new!}
    )

    @REM Loop if needed
    set /a i=i+1
    if "%i%" NEQ "%count%" (
        goto RenameLoop
    )
)

@REM Print results
echo Done^^! Renamed %changed%/%count% files
pause