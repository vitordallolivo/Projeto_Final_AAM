################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Drivers/ADC.c \
../User/Drivers/PWM.c 

C_DEPS += \
./User/Drivers/ADC.d \
./User/Drivers/PWM.d 

OBJS += \
./User/Drivers/ADC.o \
./User/Drivers/PWM.o 


EXPANDS += \
./User/Drivers/ADC.c.234r.expand \
./User/Drivers/PWM.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
User/Drivers/%.o: ../User/Drivers/%.c
	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/Debug" -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/Core" -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/User" -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

