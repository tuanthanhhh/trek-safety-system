################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Uart/uart.c 

OBJS += \
./Core/Uart/uart.o 

C_DEPS += \
./Core/Uart/uart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Uart/%.o Core/Uart/%.su Core/Uart/%.cyclo: ../Core/Uart/%.c Core/Uart/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Core/Button_Led -I../Core/Uart -I../Core/LoRa/LoRa_Lib -I../Core/LoRa/LoRa_Frame -I../Core/LoRa/Master_LoRa -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Uart

clean-Core-2f-Uart:
	-$(RM) ./Core/Uart/uart.cyclo ./Core/Uart/uart.d ./Core/Uart/uart.o ./Core/Uart/uart.su

.PHONY: clean-Core-2f-Uart

