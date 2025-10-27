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
./User/Appl/Appl.c.253r.expand \
./User/Appl/main.c.253r.expand 



# Each subdirectory must supply rules for building sources it contributes
User/Appl/%.o: ../User/Appl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv-wch-elf-gcc -march=rv32ecxw -mabi=ilp32e -mtune=size -mcmodel=medlow -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -fsingle-precision-constant --param=highcode-gen-section-name=1 -pipe -Wunused -Wuninitialized -Wall -Wshadow -Waggregate-return -Wfloat-equal -g -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/Debug" -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/Core" -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/User" -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@

