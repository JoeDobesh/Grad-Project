################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/CommandPrompt.c \
../Core/Src/Event.c \
../Core/Src/FIFO.c \
../Core/Src/HeartbeatTask.c \
../Core/Src/KernalThread.c \
../Core/Src/MailBag.c \
../Core/Src/Modbus.c \
../Core/Src/Mutex.c \
../Core/Src/RS485.c \
../Core/Src/RealTimeClock.c \
../Core/Src/SoftTimers.c \
../Core/Src/main.c \
../Core/Src/stm32f4xx_hal_msp.c \
../Core/Src/stm32f4xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f4xx.c 

OBJS += \
./Core/Src/CommandPrompt.o \
./Core/Src/Event.o \
./Core/Src/FIFO.o \
./Core/Src/HeartbeatTask.o \
./Core/Src/KernalThread.o \
./Core/Src/MailBag.o \
./Core/Src/Modbus.o \
./Core/Src/Mutex.o \
./Core/Src/RS485.o \
./Core/Src/RealTimeClock.o \
./Core/Src/SoftTimers.o \
./Core/Src/main.o \
./Core/Src/stm32f4xx_hal_msp.o \
./Core/Src/stm32f4xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f4xx.o 

C_DEPS += \
./Core/Src/CommandPrompt.d \
./Core/Src/Event.d \
./Core/Src/FIFO.d \
./Core/Src/HeartbeatTask.d \
./Core/Src/KernalThread.d \
./Core/Src/MailBag.d \
./Core/Src/Modbus.d \
./Core/Src/Mutex.d \
./Core/Src/RS485.d \
./Core/Src/RealTimeClock.d \
./Core/Src/SoftTimers.d \
./Core/Src/main.d \
./Core/Src/stm32f4xx_hal_msp.d \
./Core/Src/stm32f4xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su Core/Src/%.cyclo: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -DSTM32_THREAD_SAFE_STRATEGY=2 -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/apps/http -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Core/ThreadSafe -Og -ffunction-sections -fdata-sections -Wall -v -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/CommandPrompt.cyclo ./Core/Src/CommandPrompt.d ./Core/Src/CommandPrompt.o ./Core/Src/CommandPrompt.su ./Core/Src/Event.cyclo ./Core/Src/Event.d ./Core/Src/Event.o ./Core/Src/Event.su ./Core/Src/FIFO.cyclo ./Core/Src/FIFO.d ./Core/Src/FIFO.o ./Core/Src/FIFO.su ./Core/Src/HeartbeatTask.cyclo ./Core/Src/HeartbeatTask.d ./Core/Src/HeartbeatTask.o ./Core/Src/HeartbeatTask.su ./Core/Src/KernalThread.cyclo ./Core/Src/KernalThread.d ./Core/Src/KernalThread.o ./Core/Src/KernalThread.su ./Core/Src/MailBag.cyclo ./Core/Src/MailBag.d ./Core/Src/MailBag.o ./Core/Src/MailBag.su ./Core/Src/Modbus.cyclo ./Core/Src/Modbus.d ./Core/Src/Modbus.o ./Core/Src/Modbus.su ./Core/Src/Mutex.cyclo ./Core/Src/Mutex.d ./Core/Src/Mutex.o ./Core/Src/Mutex.su ./Core/Src/RS485.cyclo ./Core/Src/RS485.d ./Core/Src/RS485.o ./Core/Src/RS485.su ./Core/Src/RealTimeClock.cyclo ./Core/Src/RealTimeClock.d ./Core/Src/RealTimeClock.o ./Core/Src/RealTimeClock.su ./Core/Src/SoftTimers.cyclo ./Core/Src/SoftTimers.d ./Core/Src/SoftTimers.o ./Core/Src/SoftTimers.su ./Core/Src/main.cyclo ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f4xx_hal_msp.cyclo ./Core/Src/stm32f4xx_hal_msp.d ./Core/Src/stm32f4xx_hal_msp.o ./Core/Src/stm32f4xx_hal_msp.su ./Core/Src/stm32f4xx_it.cyclo ./Core/Src/stm32f4xx_it.d ./Core/Src/stm32f4xx_it.o ./Core/Src/stm32f4xx_it.su ./Core/Src/syscalls.cyclo ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.cyclo ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f4xx.cyclo ./Core/Src/system_stm32f4xx.d ./Core/Src/system_stm32f4xx.o ./Core/Src/system_stm32f4xx.su

.PHONY: clean-Core-2f-Src

