Directory structure:
    - Core/         STM32 specific drivers
    - Debug/        STM32 build debug dir
    - Drivers/      STM32 HAL drivers 
    - FATFS/        FATFS used for STM32 SD card
    - fusion360/    Interrupter case 3d files
    - Middlewares/  FATFS and USB stuff
    - USB_DEVICE/   STM32 USB drivers


Important notes:
    The program assumes that the SD card has .will formatted files in two directories:
        - single_coil/
        - dual_coils/
    Upon boot, the STM32 reads and stores all filenames from both these directories.
    Filenames are limited to 64 characters, including extension
    Up to 32 files can be in each directory (Fix this in the future)

    Need to enable "#define _FS_RPATH 1" in L151_interrupter\FATFS\Target\ffconf.h to enable "chdir()" function

    Setting A sets ONLY A 
    But setting B sets a and B