################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/System/Sound.c \
../User/System/SysError.c \
../User/System/SysTick.c \
../User/System/ThrustManager.c 

C_DEPS += \
./User/System/Sound.d \
./User/System/SysError.d \
./User/System/SysTick.d \
./User/System/ThrustManager.d 

OBJS += \
./User/System/Sound.o \
./User/System/SysError.o \
./User/System/SysTick.o \
./User/System/ThrustManager.o 


EXPANDS += \
./User/System/Sound.c.234r.expand \
./User/System/SysError.c.234r.expand \
./User/System/SysTick.c.234r.expand \
./User/System/ThrustManager.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
User/System/%.o: ../User/System/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -mtune=size -mcmodel=medlow -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -fsingle-precision-constant --param=highcode-gen-section-name=1 -pipe -Wunused -Wuninitialized -Wall -Wshadow -Waggregate-return -Wfloat-equal -g -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/Debug" -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/Core" -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/User" -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@

