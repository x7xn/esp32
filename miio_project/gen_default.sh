#!/bin/sh
#$1 equal to make param.
:<<!
make menuconfig - Configure IDF project
make defconfig - Set defaults for all new configuration options

make all - Build app, bootloader, partition table
make flash - Flash app, bootloader, partition table to a chip
make clean - Remove all build output
make size - Display the memory footprint of the app
make erase_flash - Erase entire flash contents
make monitor - Display serial output on terminal console

make app - Build just the app
make app-flash - Flash just the app
make app-clean - Clean just the app

See also 'make bootloader', 'make bootloader-flash', 'make bootloader-clean', 
'make partition_table', etc, etc.
!


if [ -z "$1" ];then
	MAKE_PARAM="all"
else
	MAKE_PARAM="$1"
fi

# DIRS 
MIIO_PROJECT_DIR=$(pwd)
echo "MIIO_PROJECT_DIR=$MIIO_PROJECT_DIR"

IDF_DIR="$MIIO_PROJECT_DIR/../esp-idf"
echo "IDF_DIR=$IDF_DIR"
export IDF_PATH=$IDF_DIR

TOOLS_DIR="$MIIO_PROJECT_DIR/../tools"
echo "TOOLS_DIR=$TOOLS_DIR"

# MAKE
cd $MIIO_PROJECT_DIR
rm -rf build/*
make clean
make $MAKE_PARAM || { echo "error: make $MAKE_PARAM failed"; exit 1; }

# add crc
cd $TOOLS_DIR/wifi_firmware_crc
make clean
make all || { echo "error: make wifi_firmware_crc failed"; exit 1; }
./add_crc $MIIO_PROJECT_DIR/build/miio_app.bin $MIIO_PROJECT_DIR/build/miio_app_crc.bin

# combine bin
cd $TOOLS_DIR
python cmb_bin.py --flash 4MB 0x1000 $MIIO_PROJECT_DIR/build/bootloader/bootloader.bin 0x8000 $MIIO_PROJECT_DIR/build/xiaomi_partitions_4M.bin 0xf000 $MIIO_PROJECT_DIR/build/phy_init_data.bin 0x10000 $MIIO_PROJECT_DIR/build/miio_app.bin 0x170000 $MIIO_PROJECT_DIR/build/miio_app.bin 0x2d0000 gpio_test/ESP32_for_xiaomi_gpio_ver_2_20170413.bin || { echo "error: python cmb_bin.py failed"; exit 1; }

# change to MIIO_PROJECT_DIR
cd $MIIO_PROJECT_DIR

# APP_VERSION
if [ -z "$2" ]; then
	MODEL=`cat components/miio_sdk/miio_instance/miio_instance_config.h | grep "#define MIIO_INSTANCE_MODEL" | awk '{print $3}' | sed 's/"//g'`
	APP_VER_NUMBER=`cat components/miio_sdk/miio_instance/miio_instance_config.h | grep "#define MIIO_APP_VERSION_NUMBER" | awk '{print $3}'`
	APP_VERSION=${MODEL}-${APP_VER_NUMBER}
else
	APP_VERSION="$2"
fi
echo "APP_VERSION=$APP_VERSION"

# BINS_DIR
if [ -z "$3" ]; then
	BINS_DIR="$MIIO_PROJECT_DIR/../builds/$APP_VERSION/bins"
else
	BINS_DIR="$3"
fi

if [ ! -d "$BINS_DIR" ]; then
	mkdir -p $BINS_DIR
fi

rm -rf $BINS_DIR/*

# COPY BINS
mv  $TOOLS_DIR/cmb_bin.bin $BINS_DIR/iflash_${APP_VERSION}_4MB.bin
mv  build/miio_app_crc.bin  $BINS_DIR/upd_${APP_VERSION}.bin
mv  build/miio_app.elf  $BINS_DIR/upd_${APP_VERSION}.elf
mv  build/xiaomi_partitions_4M.bin $BINS_DIR/xiaomi_partitions_4M.bin
mv  build/phy_init_data.bin $BINS_DIR/phy_init_data.bin
mv  build/bootloader/bootloader.bin $BINS_DIR/bootloader.bin


