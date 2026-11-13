@echo off
REM ==============================================================================
REM Test CLI Scripts for ESP RainMaker (Lab 11.4)
REM Student ID: 67030181
REM ==============================================================================

echo ================================================================
echo  Lab 11.4 ESP RainMaker CLI Verification Helper
echo ================================================================
echo [1] Install ESP RainMaker CLI (pip install esp-rainmaker-cli)
echo [2] Login to RainMaker Account (rainmaker.py login)
echo [3] Get Connected Nodes (rainmaker.py getnodes)
echo [4] Turn ON Living Room Light (Power = true)
echo [5] Turn OFF Living Room Light (Power = false)
echo [6] Exit
echo ================================================================
choice /c 123456 /m "Select option:"

if errorlevel 6 goto end
if errorlevel 5 goto light_off
if errorlevel 4 goto light_on
if errorlevel 3 goto get_nodes
if errorlevel 2 goto rmaker_login
if errorlevel 1 goto install_cli

:install_cli
echo Installing esp-rainmaker-cli...
pip install esp-rainmaker-cli
pause
goto end

:rmaker_login
echo Logging into ESP RainMaker Cloud...
python -m rainmaker login
pause
goto end

:get_nodes
echo Fetching all registered nodes...
python -m rainmaker getnodes
pause
goto end

:light_on
set /p NODE_ID="Enter your Node ID: "
echo Sending Power ON command to node %NODE_ID%...
python -m rainmaker setparams --nodeid %NODE_ID% "{\"Living Room Light\": {\"Power\": true}}"
pause
goto end

:light_off
set /p NODE_ID="Enter your Node ID: "
echo Sending Power OFF command to node %NODE_ID%...
python -m rainmaker setparams --nodeid %NODE_ID% "{\"Living Room Light\": {\"Power\": false}}"
pause
goto end

:end

