echo off
netstat -ano | find "CLOSE_WAIT" > out.txt
FOR /F "tokens=5 delims= " %%I IN (
    'netstat -ano ^| find "CLOSE_WAIT"'
) DO (
    taskkill /PID %%I /F
)

del out.txt

netstat -ano | find "TIME_WAITT" > out.txt
FOR /F "tokens=5 delims= " %%I IN (
    'netstat -ano ^| find "TIME_WAITT"'
) DO (
    taskkill /PID %%I /F
)
del out.txt

netstat -ano | find "127.0.0.1" > out.txt
FOR /F "tokens=5 delims= " %%I IN (
    'netstat -ano ^| find "127.0.0."'
) DO (
    taskkill /PID %%I /F
)
del out.txt

netstat -ano | find "0.0.0.0" > out.txt
FOR /F "tokens=5 delims= " %%I IN (
    'netstat -ano ^| find "0.0.0.0"'
) DO (
    taskkill /PID %%I /F
)
del out.txt