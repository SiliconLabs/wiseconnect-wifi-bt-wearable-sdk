=======================================================================================================================================================================
Release            : Wearables_SDK_v1.0.0.0002(RSI_SDK_WEARABLES_v2.0)
Date               : July 19 2023
_______________________________________________________________
  Copyright (C) Silicon Labs 2023
_______________________________________________________________

1.0 Release Status:
===================

    Test         ( )
    Alpha        (X)
    Beta         ( )
    Production   ( )

2.0 New features/changes added in this release:
=========================================== 
1. Added COEX_THROUGHPUT_DEMO_67 and UNIFIED_PROTOCOL_DEMO_54 applications 


3.0 Known Issues
================
1. In UNIFIED_PROTOCOL_DEMO_54, "Avrcp reg notification cmd status" and "on avrcp notify" prints will spam and are commented off because of this.
2. In COEX_THROUGHPUT_DEMO_67, "abs vol" and "on avrcp notify" prints will spam and are commented off because of this.

4.0 Other Notes
===============
1. WiSeConnect firmware version - v1611.1.4.0.0.2
2. For 1.5v Silicon Chip's UULP_GPIO_0 is used as a wakeup indication to host, user should update following bitmaps to make use of this chip version
        - BIT(0) should be set in RSI_CONFIG_FEATURE_BIT_MAP &
        - BIT(31) should be set in both TCP_IP_FEATURE_BIT_MAP and EXT_TCP_IP_FEATURE_BIT_MAP.
        - BIT(23) should be set in RSI_EXT_CUSTOM_FEATURE_BIT_MAP.     
        - EXT_FEAT_XTAL_CLK_ENABLE (BIT(22)) should be reset in RSI_EXT_CUSTOM_FEATURE_BIT_MAP.
3. SAPI error codes and other references can be found at - https://docs.silabs.com/d/wifibt-wc-sapi-reference/2.52/error-codes
4. COEX_MAX_BLE_2MAS_8SLAV_DEMO_69 application can be run only with RT595 boards. The procedure to run the demo is similar to COEX_MAX_DEMO_57 and its doc can be referred to run COEX_MAX_BLE_2MAS_8SLAV_DEMO_69 as well.

5. For COEX_MAX_DEMO_57 and COEX_MAX_DEMO_BLE_2MAS_8SLAV_69 applications, the RSI_TCP_IP_BYPASS feature is only supported in IAR ide.
6. Powersave needs to be disabled and A2DP_BT_ONLY_CONNECTION needs to be enabled when running Wifi download + BT + BLE coex use case.
7. Use "RS9116_WC_SI_WEARABLES.rps" binary for 1.5 silicon EVK.

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

=======================================================================================================================================================================
Release            : Wearables_SDK_v1.0.0.0001(RSI_SDK_WEARABLES_v2.0)
Date               : June 30 2023
_______________________________________________________________
  Copyright (C) Silicon Labs 2023
_______________________________________________________________

1.0 Release Status:
===================

    Test         ( )
    Alpha        (X)
    Beta         ( )
    Production   ( )

2.0 Features supported in this release:
=========================================== 
1. COEX_MAX_DEMO_57 and COEX_MAX_BLE_2MAS_8SLAV_DEMO_69 applications supported in this release.
2. COEX_MAX_DEMO_57 supports upto 5 BLE connections (2 Master + 3 Slave) and COEX_MAX_BLE_2MAS_8SLAV_DEMO_69 supports upto 10 BLE connections
  (2 Master + 8 Slaves).
3. BT Features supported - Master, Slave, A2DP Streaming, AVRCP, GAP, L2CAP, Role Switch, Enhanced Data Rate
4. BLE features supported - BLE Peripheral, BLE Central with multiple peripheral support, Data Length Extension, LE Dual Role,
   LL Privacy, BLE Whitelisting, GAP, GATT, SMP, LE L2CAP
5. WLAN features supported - HTTP download, HTTPS download
6. Detailed feature list can be found at - https://docs.silabs.com/rs9116-wiseconnect/2.8.0/wifibt-wc-sapi-reference/features

3.0 Other Notes
===============
1. WiSeConnect firmware version - v1611.1.4.0.0.1
2. For 1.5v Silicon Chip's UULP_GPIO_0 is used as a wakeup indication to host, user should update following bitmaps to make use of this chip version
        - BIT(0) should be set in RSI_CONFIG_FEATURE_BIT_MAP &
        - BIT(31) should be set in both TCP_IP_FEATURE_BIT_MAP and EXT_TCP_IP_FEATURE_BIT_MAP.
        - BIT(23) should be set in RSI_EXT_CUSTOM_FEATURE_BIT_MAP. 	
        - EXT_FEAT_XTAL_CLK_ENABLE (BIT(22)) should be reset in RSI_EXT_CUSTOM_FEATURE_BIT_MAP.
3. SAPI error codes and other references can be found at - https://docs.silabs.com/d/wifibt-wc-sapi-reference/2.52/error-codes
4. COEX_MAX_BLE_2MAS_8SLAV_DEMO_69 application can be run only with RT595 boards. The procedure to run the demo is similar to COEX_MAX_DEMO_57 and its doc can be reffered to run COEX_MAX_BLE_2MAS_8SLAV_DEMO_69 as well.
5. Powersave needs to be disabled and A2DP_BT_ONLY_CONNECTION needs to be enabled when running Wifi download + BT + BLE coex use case.
6. Coex usecases tested -
        - Using DEMO_57(With Powersave) tested “5 BLE Connections (2 Master + 3 Slave) with Data Transfer + BT A2DP Stream + Wifi Scan/Connect” CoEx case.
        - Using DEMO_69(With Powersave) tested “10 BLE Connections (2 Master + 8 Slave) with Data Transfer + BT A2DP Stream + Wifi Scan/Connect” CoEx case.
        - Using DEMO_57 (Without powersave) tested “5 BLE Connections (2 Master + 3 Slave) with Data Transfer + BT A2DP Connection + Wifi HTTP download” CoEx case.
        - Using DEMO_69 (Without powersave) tested “10 BLE Connections (2 Master + 8 Slave) with Data Transfer + BT A2DP Connection + Wifi HTTP download” CoEx case.
        - Using DEMO_57 and DEMO_69 applications tested “1 BLE Connection + BT A2DP + HTTPS download” CoEx case.
7. Use "RS9116_WC_SI_WEARABLES.rps" binary for 1.5 silicon EVK.

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
   > Import pcm_data_buff.h (can be found in COEX_MAX_DEMO folder) into project environment
   > In rsi_bt_config_DEMO_XX.h define RSI_AUDIO_DATA_SRC as ARRAY
