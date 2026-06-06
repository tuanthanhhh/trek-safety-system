################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Peripheral/LCD/i2c-lcd.c 

OBJS += \
./Core/Peripheral/LCD/i2c-lcd.o 

C_DEPS += \
./Core/Peripheral/LCD/i2c-lcd.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Peripheral/LCD/%.o Core/Peripheral/LCD/%.su Core/Peripheral/LCD/%.cyclo: ../Core/Peripheral/LCD/%.c Core/Peripheral/LCD/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Core/Peripheral/Button_Led -I../Core/Peripheral/LCD -I../Core/Peripheral/Led -I../Core/Peripheral/USB -I../Core/LoRa/LoRa_Lib -I../Core/LoRa/LoRa_Frame -I../Core/LoRa/Master_LoRa -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Peripheral-2f-LCD

clean-Core-2f-Peripheral-2f-LCD:
	-$(RM) ./Core/Peripheral/LCD/i2c-lcd.cyclo ./Core/Peripheral/LCD/i2c-lcd.d ./Core/Peripheral/LCD/i2c-lcd.o ./Core/Peripheral/LCD/i2c-lcd.su

.PHONY: clean-Core-2f-Peripheral-2f-LCD

