@echo off
setlocal
set toolarch=amd64

if /I x%1 == x%toolarch% (
    exit /B 0
) else (
    echo must have WRK\tools\%toolarch% in path
    exit /B 1
)

