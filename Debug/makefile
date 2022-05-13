################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := Morcom_v2
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
Morcom_v2.elf \

MAP_FILES += \
Morcom_v2.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
Morcom_v2.list \

OBJCOPY_HEX += \
Morcom_v2.hex \

OBJCOPY_BIN += \
Morcom_v2.bin \

OBJCOPY_VERILOG += \
Morcom_v2.verilog \

OBJCOPY_SREC += \
Morcom_v2.srec \

OBJCOPY_SYMBOLSREC += \
Morcom_v2.symbolsrec \


# All Target
all: main-build

# Main-build Target
main-build: Morcom_v2.elf secondary-outputs

# Tool invocations
Morcom_v2.elf Morcom_v2.map: $(OBJS) $(USER_OBJS) R:\Documentos\ RAID\PnP_mati\Morcom_v2\Morcom_v2_wrksp\Morcom_v2\STM32F103C8TX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-g++ -o "Morcom_v2.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"R:\Documentos RAID\PnP_mati\Morcom_v2\Morcom_v2_wrksp\Morcom_v2\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="Morcom_v2.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -lstdc++ -lsupc++ -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

Morcom_v2.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "Morcom_v2.list"
	@echo 'Finished building: $@'
	@echo ' '

Morcom_v2.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "Morcom_v2.hex"
	@echo 'Finished building: $@'
	@echo ' '

Morcom_v2.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "Morcom_v2.bin"
	@echo 'Finished building: $@'
	@echo ' '

Morcom_v2.verilog: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O verilog $(EXECUTABLES) "Morcom_v2.verilog"
	@echo 'Finished building: $@'
	@echo ' '

Morcom_v2.srec: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O srec $(EXECUTABLES) "Morcom_v2.srec"
	@echo 'Finished building: $@'
	@echo ' '

Morcom_v2.symbolsrec: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O symbolsrec $(EXECUTABLES) "Morcom_v2.symbolsrec"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) Morcom_v2.bin Morcom_v2.elf Morcom_v2.hex Morcom_v2.list Morcom_v2.map Morcom_v2.srec Morcom_v2.symbolsrec Morcom_v2.verilog default.size.stdout
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN) $(OBJCOPY_VERILOG) $(OBJCOPY_SREC) $(OBJCOPY_SYMBOLSREC)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
