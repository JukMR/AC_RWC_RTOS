################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/DHT.c \
../Core/Src/ESP8266_HAL.c \
../Core/Src/ESPDataLogger.c \
../Core/Src/ESP_DATA_HANDLER.c \
../Core/Src/UartRingbuffer.c \
../Core/Src/freertos.c \
../Core/Src/gpio.c \
../Core/Src/led.c \
../Core/Src/main.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/system_stm32f4xx.c \
../Core/Src/usart.c 

OBJS += \
./Core/Src/DHT.o \
./Core/Src/ESP8266_HAL.o \
./Core/Src/ESPDataLogger.o \
./Core/Src/ESP_DATA_HANDLER.o \
./Core/Src/UartRingbuffer.o \
./Core/Src/freertos.o \
./Core/Src/gpio.o \
./Core/Src/led.o \
./Core/Src/main.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/system_stm32f4xx.o \
./Core/Src/usart.o 

C_DEPS += \
./Core/Src/DHT.d \
./Core/Src/ESP8266_HAL.d \
./Core/Src/ESPDataLogger.d \
./Core/Src/ESP_DATA_HANDLER.d \
./Core/Src/UartRingbuffer.d \
./Core/Src/freertos.d \
./Core/Src/gpio.d \
./Core/Src/led.d \
./Core/Src/main.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/system_stm32f4xx.d \
./Core/Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o: ../Core/Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DUSE_HAL_DRIVER -DSTM32F411xE -I"/home/julian/Documents/famaf/micro/stm32/projects/RTOS_DHT_ESP8266/Core/Inc" -I"/home/julian/Documents/famaf/micro/stm32/projects/RTOS_DHT_ESP8266/Drivers/STM32F4xx_HAL_Driver/Inc" -I"/home/julian/Documents/famaf/micro/stm32/projects/RTOS_DHT_ESP8266/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy" -I"/home/julian/Documents/famaf/micro/stm32/projects/RTOS_DHT_ESP8266/Middlewares/Third_Party/FreeRTOS/Source/include" -I"/home/julian/Documents/famaf/micro/stm32/projects/RTOS_DHT_ESP8266/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS" -I"/home/julian/Documents/famaf/micro/stm32/projects/RTOS_DHT_ESP8266/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F" -I"/home/julian/Documents/famaf/micro/stm32/projects/RTOS_DHT_ESP8266/Drivers/CMSIS/Device/ST/STM32F4xx/Include" -I"/home/julian/Documents/famaf/micro/stm32/projects/RTOS_DHT_ESP8266/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


