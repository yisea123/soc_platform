:: make symbolic links of SOC platform modules in product firmware directory tree
@echo off
cd /d %~dp0
echo %cd%
cd ..\..\
echo %cd%
set product_root=%cd%
set firmware_root=%product_root%\firmware
cd ..
set platform_root=%cd%\platform
set bootloader_root=%cd%\bootloader

echo platform_root = %platform_root%
echo bootloader_root = %bootloader_root%
echo product_root = %product_root%
echo firmware_root = %firmware_root%

if not exist %platform_root% goto noplatform
if not exist %bootloader_root% goto nobootloader

REM make links to include, FreeRTOS, fw_platform
cd %firmware_root%
echo %cd%
if not exist include mklink /D include %platform_root%\include
if not exist FreeRTOS mklink /D FreeRTOS %platform_root%\FreeRTOS
if not exist fw_platform mklink /D fw_platform %platform_root%\fw_platform

REM make links to bootloader
cd %firmware_root%
echo %cd%
if not exist bootloader mklink /D bootloader %bootloader_root%\application

REM make links in hw_platform
cd %firmware_root%\hw_platform
echo %cd%
if not exist CMSIS mklink /D CMSIS %platform_root%\hw_platform\CMSIS
if not exist hal mklink /D hal %platform_root%\hw_platform\hal
if not exist drivers mklink /D drivers %platform_root%\hw_platform\drivers

REM make links to SOC platform IAR project files
cd %firmware_root%\project\IAR-EWARM
echo %cd%
if not exist freertos.ewp mklink freertos.ewp %platform_root%\project\IAR-EWARM\freertos.ewp
if not exist firmware_platform.ewp mklink firmware_platform.ewp %platform_root%\project\IAR-EWARM\firmware_platform.ewp
if not exist hardware_platform.ewp mklink hardware_platform.ewp %platform_root%\project\IAR-EWARM\hardware_platform.ewp
if not exist bootloader.eww mklink /H bootloader.eww %bootloader_root%\project\IAR-EWARM\bootloader.eww
if not exist bootloader.ewp mklink /H bootloader.ewp %bootloader_root%\project\IAR-EWARM\bootloader.ewp
if not exist bootloader.ewd mklink bootloader.ewd %bootloader_root%\project\IAR-EWARM\bootloader.ewd
if not exist smartfusion2_envm_bootloader.icf mklink smartfusion2_envm_bootloader.icf %bootloader_root%\project\IAR-EWARM\smartfusion2_envm_bootloader.icf


goto end

:noplatform
echo SOC firmware platform directory is not found !
echo Please make sure the platform directory is in the SAME directory of the product source root !

:nobootloader
echo SOC firmware bootloader directory is not found !
echo Please make sure the bootloader directory is in the SAME directory of the product source root !

:end
cd %firmware_root%