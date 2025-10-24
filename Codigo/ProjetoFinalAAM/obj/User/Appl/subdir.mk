################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Appl/Appl.c \
../User/Appl/main.c 

C_DEPS += \
./User/Appl/Appl.d \
./User/Appl/main.d 

OBJS += \
./User/Appl/Appl.o \
./User/Appl/main.o 


EXPANDS += \
./User/Appl/Appl.c.234r.expand \
./User/Appl/main.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
User/Appl/%.o: ../User/Appl/%.c
	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/Debug" -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/Core" -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/User" -I"/home/kali/Faculdade/Disciplinas/AAM/Projeto_Final_AAM/Codigo/ProjetoFinalAAM/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

