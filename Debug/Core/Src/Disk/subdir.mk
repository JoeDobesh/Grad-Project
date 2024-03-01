################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/Disk/Boot.c \
../Core/Src/Disk/Directories.c \
../Core/Src/Disk/FATSystem.c \
../Core/Src/Disk/FileSystem.c \
../Core/Src/Disk/SD_Card.c \
../Core/Src/Disk/SPI.c 

OBJS += \
./Core/Src/Disk/Boot.o \
./Core/Src/Disk/Directories.o \
./Core/Src/Disk/FATSystem.o \
./Core/Src/Disk/FileSystem.o \
./Core/Src/Disk/SD_Card.o \
./Core/Src/Disk/SPI.o 

C_DEPS += \
./Core/Src/Disk/Boot.d \
./Core/Src/Disk/Directories.d \
./Core/Src/Disk/FATSystem.d \
./Core/Src/Disk/FileSystem.d \
./Core/Src/Disk/SD_Card.d \
./Core/Src/Disk/SPI.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/Disk/%.o Core/Src/Disk/%.su: ../Core/Src/Disk/%.c Core/Src/Disk/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F429xx -c -I../Core/Inc -I../Core/Inc/Disk -I../Core/Inc/Network -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../LWIP/App -I../LWIP/Target -I../Middlewares/Third_Party/LwIP/src/include -I../Middlewares/Third_Party/LwIP/system -I../Drivers/BSP/Components/lan8742 -I../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../Middlewares/Third_Party/LwIP/src/include/lwip -I../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../Middlewares/Third_Party/LwIP/src/include/netif -I../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../Middlewares/Third_Party/LwIP/system/arch -I../Middlewares/Third_Party/LwIP/src/apps/http -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Src-2f-Disk

clean-Core-2f-Src-2f-Disk:
	-$(RM) ./Core/Src/Disk/Boot.d ./Core/Src/Disk/Boot.o ./Core/Src/Disk/Boot.su ./Core/Src/Disk/Directories.d ./Core/Src/Disk/Directories.o ./Core/Src/Disk/Directories.su ./Core/Src/Disk/FATSystem.d ./Core/Src/Disk/FATSystem.o ./Core/Src/Disk/FATSystem.su ./Core/Src/Disk/FileSystem.d ./Core/Src/Disk/FileSystem.o ./Core/Src/Disk/FileSystem.su ./Core/Src/Disk/SD_Card.d ./Core/Src/Disk/SD_Card.o ./Core/Src/Disk/SD_Card.su ./Core/Src/Disk/SPI.d ./Core/Src/Disk/SPI.o ./Core/Src/Disk/SPI.su

.PHONY: clean-Core-2f-Src-2f-Disk

