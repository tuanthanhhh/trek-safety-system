################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Peripheral/LSM303DLHC/lsm303dlhc.c 

OBJS += \
./Core/Peripheral/LSM303DLHC/lsm303dlhc.o 

C_DEPS += \
./Core/Peripheral/LSM303DLHC/lsm303dlhc.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Peripheral/LSM303DLHC/%.o Core/Peripheral/LSM303DLHC/%.su Core/Peripheral/LSM303DLHC/%.cyclo: ../Core/Peripheral/LSM303DLHC/%.c Core/Peripheral/LSM303DLHC/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/App" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/BME280" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/DS3231" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LSM303DLHC" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/SH1160" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/GPS" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LED" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa/Slave_LoRa" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa/LoRa_Frame" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa/LoRa_Lib" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Peripheral-2f-LSM303DLHC

clean-Core-2f-Peripheral-2f-LSM303DLHC:
	-$(RM) ./Core/Peripheral/LSM303DLHC/lsm303dlhc.cyclo ./Core/Peripheral/LSM303DLHC/lsm303dlhc.d ./Core/Peripheral/LSM303DLHC/lsm303dlhc.o ./Core/Peripheral/LSM303DLHC/lsm303dlhc.su

.PHONY: clean-Core-2f-Peripheral-2f-LSM303DLHC

