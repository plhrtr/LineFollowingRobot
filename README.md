# Mobile_Roboter
Codebase for the 'Mobile Roboter' course

# Building and flashing the project 

```bash 
# Prepare the build directory
cmake -B build -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=cmake/gcc-arm-none-eabi.cmake

# Build the project 
cmake --build build

# Flash the project to the STM32
st-flash write build/ArmuroL432.bin 0x08000000
```

# Debugging the project

```bash 
# start st-util
st-util
# Start gdb 
arm-none-eabi-gdb ./build/ArmuroL432.elf
# Set the remote target to use the st-util server
target extended-remote :4242
# Halt and reset the MCU
monitor reset halt
# Flash the selected .elf
load
# Set break points 
break main
# Start debugging 
continue
```

# Logging inside the project

``` C 
// Each file defines log modules, 
// consisting of a name and a flag whether logging for this module is enabled. 
// This log module is passed when logging a message 
static char bar = 1;

static const log_module_t foo = {
  "foo",
  &bar
};
// A file can define multiple log modules at once for fine granular configuration
// You can enable of disable module on a per file basis

// Initate the logger in main via 
logger_init(LOG_DEBUG, CSV, UART);
// Or use the default values (LOG_WARNING, PLAIN_TEXT, UART)

// Use the makro do log a message. The message is a formatted string
// This will append the time, filename and line number for you
LOGGER_LOG(LOG_DEBUG, foo, "Foo: %s, %u", var1, var2);
```

