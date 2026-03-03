
## Firmware Update Procedure

1. Change the project version in the `.yaml` file.

2. Build the firmware. Locate:
   .esphome/build/raspiaudio-radio/.pioenvs/raspiaudio-radio/firmware.ota.bin
   Rename it to:
   update_firmware.bin

3. Compute checksum:
   md5sum update_firmware.bin

4. Edit manifest_update.json and replace the "md5" value with the result from step 3.# esphomeMangaCastMusic
