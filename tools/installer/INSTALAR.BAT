@echo off
cls

set T_DIR=%1

IF X%T_DIR% == X GOTO ERROR_PARAM

:VALIDATE
mkdir %T_DIR% > nul
echo ok > %T_DIR%\test.tmp
if not exist %T_DIR%\test.tmp goto ERR_WRITE
del %T_DIR%\test.tmp > nul

:WELCOME
cls
type title.txt
echo.
echo   Se va a instalar en: "%T_DIR%"
echo.
echo         +--------------------------------------------------------------+
echo         I             ELIJA SI DESEA INSTALAR EL PROGRAMA              I
echo         +--------------------------------------------------------------+
echo         I                                                              I
echo         I               [E] Empezar         [C] Cancelar               I
echo         I                                                              I
echo         +--------------------------------------------------------------+
choice /C:EC /N
if errorlevel 2 goto ABORT
if errorlevel 1 goto UNZIP
goto WELCOME

:UNZIP
cls
echo.
echo         +--------------------------------------------------------------+
echo         I                     EXTRAYENDO ARCHIVOS                      I
echo         +--------------------------------------------------------------+
echo         I                                                              I
echo         I                     Espere, por favor...                     I
echo         I                                                              I
echo         +--------------------------------------------------------------+
echo.
unzip.exe -o DATA.ZIP -d %T_DIR%
if errorlevel 2 goto ERR_UNZIP

:SUCCESS
cls
echo.
echo         +--------------------------------------------------------------+
echo         I            SE HA COMPLETADO LA COPIA DE FICHEROS             I
echo         +--------------------------------------------------------------+
echo         I                                                              I
echo         I            El proceso ha terminado exitosamente.             I
echo         I                                                              I
echo         I             Pulse cualquier tecla para terminar.             I
echo         I                                                              I
echo         +--------------------------------------------------------------+
echo.
pause > nul
cls
echo Se ha instalado correctamente en: "%T_DIR%"
echo.
goto END

:ABORT
cls
echo Cancelado por el usuario.
echo.
goto END

:ERROR_PARAM
cls
echo ERROR: No se ha especificado la ruta de destino.
echo Uso: INSTALAR.BAT [RUTA]
echo.
goto END

:ERR_WRITE
echo ERROR FATAL: No se pudo escribir en "%T_DIR%".
echo.
goto END

:ERR_UNZIP
echo ERROR FATAL: Fallo al descomprimir ficheros.
echo.
goto END

:END
