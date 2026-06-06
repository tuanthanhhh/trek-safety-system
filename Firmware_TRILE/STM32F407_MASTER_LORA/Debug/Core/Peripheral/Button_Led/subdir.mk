################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Peripheral/Button_Led/button_led.c 

OBJS += \
./Core/Peripheral/Button_Led/button_led.o 

C_DEPS += \
./Core/Peripheral/Button_Led/button_led.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Peripheral/Button_Led/%.o Core/Peripheral/Button_Led/%.su Core/Peripheral/Button_Led/%.cyclo: ../Core/Peripheral/Button_Led/%.c Core/Peripheral/Button_Led/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -c -I../Core/Inc -I../Core/Peripheral/Button_Led -I../Core/Peripheral/LCD -I../Core/Peripheral/Led -I../Core/Peripheral/USB -I../Core/LoRa/LoRa_Lib -I../Core/LoRa/LoRa_Frame -I../Core/LoRa/Master_LoRa -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Peripheral-2f-Button_Led

clean-Core-2f-Peripheral-2f-Button_Led:
	-$(RM) ./Core/Peripheral/Button_Led/button_led.cyclo ./Core/Peripheral/Button_Led/button_led.d ./Core/Peripheral/Button_Led/button_led.o ./Core/Peripheral/Button_Led/button_led.su

.PHONY: clean-Core-2f-Peripheral-2f-Button_Led

