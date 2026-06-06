################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Peripheral/SH1160/SH1106.c \
../Core/Peripheral/SH1160/bitmap.c \
../Core/Peripheral/SH1160/fonts.c 

OBJS += \
./Core/Peripheral/SH1160/SH1106.o \
./Core/Peripheral/SH1160/bitmap.o \
./Core/Peripheral/SH1160/fonts.o 

C_DEPS += \
./Core/Peripheral/SH1160/SH1106.d \
./Core/Peripheral/SH1160/bitmap.d \
./Core/Peripheral/SH1160/fonts.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Peripheral/SH1160/%.o Core/Peripheral/SH1160/%.su Core/Peripheral/SH1160/%.cyclo: ../Core/Peripheral/SH1160/%.c Core/Peripheral/SH1160/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/App" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/BME280" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/DS3231" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LSM303DLHC" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/SH1160" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/GPS" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LED" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa/Slave_LoRa" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa/LoRa_Frame" -I"C:/Users/ACER/STM32CubeIDE/workspace_2.1.1/BTL_TKHTN/Main_Link 5/Firmware/Core/Peripheral/LoRa/LoRa_Lib" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Peripheral-2f-SH1160

clean-Core-2f-Peripheral-2f-SH1160:
	-$(RM) ./Core/Peripheral/SH1160/SH1106.cyclo ./Core/Peripheral/SH1160/SH1106.d ./Core/Peripheral/SH1160/SH1106.o ./Core/Peripheral/SH1160/SH1106.su ./Core/Peripheral/SH1160/bitmap.cyclo ./Core/Peripheral/SH1160/bitmap.d ./Core/Peripheral/SH1160/bitmap.o ./Core/Peripheral/SH1160/bitmap.su ./Core/Peripheral/SH1160/fonts.cyclo ./Core/Peripheral/SH1160/fonts.d ./Core/Peripheral/SH1160/fonts.o ./Core/Peripheral/SH1160/fonts.su

.PHONY: clean-Core-2f-Peripheral-2f-SH1160

