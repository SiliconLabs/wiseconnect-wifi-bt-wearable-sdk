=======================================================================================================================================================================
Steps to do to run the demo's with this release:
-----------------------------------------------
1. Extract the release package to local directory, say RSI_SDK_WEARABLES_vX/
2. The latest firmware to use with this example project is located at RSI_SDK_WEARABLES_vX/firmware.
   Update the Redpine EVK with this firmware version prior to running the applications.
3. Change/map SmartStudio's workspace to RSI_SDK_WEARABLES_vX.
4. Import project from 'RSI_SDK_WEARABLES_vX/platforms/NXP_FRDM_K28/reference_projects' into 'Project Explorer'.
5. Once project is imported, all examples are located under the examples/.
6. There is a common inc folder, which includes all the common headers.
7. For each example , there is a local header file that needs to be modified before running the example.
8. For each supported and verified example, there is a user guide that provides the steps on how to run this example located in RSI_SDK_WEARABLES_vX/doc.
   For example, COEX_MAX_DEMO_57 has a COEX_MAX_DEMO_57.pdf file in RSI_SDK_WEARABLES_vX/doc.
9. For 1.8v, enable 'ENABLE_1P8V' macro in RSI_SDK_WEARABLES_vX->examples->inc
10. To disable RTOS support, remove 'RSI_WITH_OS' in "Project->Properties->C/C++ Build->Settings->Tool Settings->GNU ARM Cross C Compiler->Preprocessor" and click 'Apply and Close'.

NOTE:
----
1. By default, 'RSI_WITH_OS' is enabled in the project.
2. It is recommended to extract the release package to any high-level directory (like 'C:\' or 'D:\' for Windows and '/root/' in Linux)  instead of using multiple sub-folders. 
SmartStudio will not be able to find project files if the path is too long.
3. Refer to SmartStudio QuickStart Guide.pdf for SmartStudio related details.
4. To run A2DP streaming case on RT595 host without an SD Card, follow these steps - 
   > Disable rsi_mcu_sdcard_init() in sapi\driver\device_interface\sdio\rsi_sdio_iface_init.c
   > Import pcm_data_buff.h (can be found in COEX_MAX_DEMO folder) into project environment (Import pcm_data_buff.h to only one demo folder at a time, remove before running on another demo)
   > In rsi_bt_config_DEMO_XX.h define RSI_AUDIO_DATA_SRC as ARRAY
6. SAPI error codes and other references can be found at - https://docs.silabs.com/d/wifibt-wc-sapi-reference/2.52/error-codes
7. COEX_MAX_BLE_2MAS_8SLAV_DEMO_69 application can be run only with RT595 boards. The procedure to run the demo is similar to COEX_MAX_DEMO_57 and its doc can be referred to run COEX_MAX_BLE_2MAS_8SLAV_DEMO_69 as well.
8. For COEX_MAX_DEMO_57 and COEX_MAX_DEMO_BLE_2MAS_8SLAV_69 applications, the RSI_TCP_IP_BYPASS feature is only supported in IAR ide.
9. Powersave needs to be disabled and A2DP_BT_ONLY_CONNECTION needs to be enabled when running Wifi download + BT + BLE coex use case.
10. Use "RS9116_WC_SI_WEARABLES.rps" binary for 1.5 silicon EVK.