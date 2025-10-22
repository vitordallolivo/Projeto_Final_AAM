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
	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/Debug" -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/Core" -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/User" -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

