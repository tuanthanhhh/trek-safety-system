################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_SRCS += \
../Core/Startup/startup_stm32f407vetx.s 

OBJS += \
./Core/Startup/startup_stm32f407vetx.o 

S_DEPS += \
./Core/Startup/startup_stm32f407vetx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Startup/%.o: ../Core/Startup/%.s Core/Startup/subdir.mk
	arm-none-eabi-gcc -mcpu=cortex-m4 -g3 -DDEBUG -c -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/App" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/BME280" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/DS3231" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LSM303DLHC" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/SH1160" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/GPS" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LED" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa/Slave_LoRa" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa/LoRa_Frame" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa/LoRa_Lib" -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@" "$<"

clean: clean-Core-2f-Startup

clean-Core-2f-Startup:
	-$(RM) ./Core/Startup/startup_stm32f407vetx.d ./Core/Startup/startup_stm32f407vetx.o

.PHONY: clean-Core-2f-Startup

