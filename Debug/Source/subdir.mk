################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Source/UsartDriverStm32F103.c \
../Source/syscalls.c \
../Source/sysmem.c \
../Source/system_stm32f1xx.c \
../Source/vHardwareSetup.c 

OBJS += \
./Source/UsartDriverStm32F103.o \
./Source/syscalls.o \
./Source/sysmem.o \
./Source/system_stm32f1xx.o \
./Source/vHardwareSetup.o 

C_DEPS += \
./Source/UsartDriverStm32F103.d \
./Source/syscalls.d \
./Source/sysmem.d \
./Source/system_stm32f1xx.d \
./Source/vHardwareSetup.d 


# Each subdirectory must supply rules for building sources it contributes
Source/%.o Source/%.su Source/%.cyclo: ../Source/%.c Source/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32F103xB -DSTM32F103RBTx -DSTM32 -DSTM32F1 -c -I../Inc -I"D:/Documents/Power_Electronic/Firmware/cubeWorkSpace/NUCLEO-F103RB_Setup" -I../Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Source

clean-Source:
	-$(RM) ./Source/UsartDriverStm32F103.cyclo ./Source/UsartDriverStm32F103.d ./Source/UsartDriverStm32F103.o ./Source/UsartDriverStm32F103.su ./Source/syscalls.cyclo ./Source/syscalls.d ./Source/syscalls.o ./Source/syscalls.su ./Source/sysmem.cyclo ./Source/sysmem.d ./Source/sysmem.o ./Source/sysmem.su ./Source/system_stm32f1xx.cyclo ./Source/system_stm32f1xx.d ./Source/system_stm32f1xx.o ./Source/system_stm32f1xx.su ./Source/vHardwareSetup.cyclo ./Source/vHardwareSetup.d ./Source/vHardwareSetup.o ./Source/vHardwareSetup.su

.PHONY: clean-Source

