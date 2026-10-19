@echo off
setlocal
cd /d "%~dp0"
echo 1 - Portugues
echo 2 - English
set "MKW_LANG=pt"
set /p "MKW_CHOICE=Idioma / Language [1]: "
if "%MKW_CHOICE%"=="2" set "MKW_LANG=en"
set "MKW_PY="
where py >nul 2>nul
if errorlevel 1 goto pythonpath
for %%V in (3.13 3.12 3.11 3.14) do (
  if not defined MKW_PY (
    py -%%V -c "import sys;sys.exit(0)" >nul 2>nul
    if not errorlevel 1 set "MKW_PY=py -%%V"
  )
)
if defined MKW_PY goto ready
:pythonpath
python -c "import sys;sys.exit(not ((3,11) <= sys.version_info[:2] < (3,15)))" >nul 2>nul
if not errorlevel 1 set "MKW_PY=python"
if not defined MKW_PY (
  echo Instala Python 3.13, com Add python.exe to PATH.
  echo Install Python 3.13 and enable Add python.exe to PATH.
  echo https://www.python.org/downloads/
  echo Consulta INICIO_PT.md / See START_HERE_EN.md
  pause
  exit /b 1
)
:ready
set "PYTHONUTF8=1"
set "PYTHONUNBUFFERED=1"
%MKW_PY% preparar.py --lang %MKW_LANG% %*
set "MKW_RESULT=%errorlevel%"
pause
exit /b %MKW_RESULT%
