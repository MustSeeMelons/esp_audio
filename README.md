# Debug PC (Program Counter)

Tool located at: `C:\Users\Strau\.platformio\packages\toolchain-xtensa-esp32\bin>`
Commad to run: `.\xtensa-esp32-elf-addr2line.exe -pfiaC -e C:\Git\esp_audio\.pio\build\esp32dev\firmware.elf -a 0x400874e9`

# Use PlatformIO built in esp32_exception_decoder

From the project dir run: `pio device monitor --filter esp32_exception_decoder`

# Keep in mind

- Check return values, dont just assume all is well.

# Stuff

- Partitions: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html
