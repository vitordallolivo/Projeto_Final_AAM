################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Hal/Comunication.c \
../User/Hal/Hal.c \
../User/Hal/PPM.c 

C_DEPS += \
./User/Hal/Comunication.d \
./User/Hal/Hal.d \
./User/Hal/PPM.d 

OBJS += \
./User/Hal/Comunication.o \
./User/Hal/Hal.o \
./User/Hal/PPM.o 


EXPANDS += \
./User/Hal/Comunication.c.253r.expand \
./User/Hal/Hal.c.253r.expand \
./User/Hal/PPM.c.253r.expand 



# Each subdirectory must supply rules for building sources it contributes
User/Hal/%.o: ../User/Hal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GNU RISC-V Cross C Compiler'
	riscv-wch-elf-gcc -march=rv32ecxw -mabi=ilp32e -mtune=size -mcmodel=medlow -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -fsingle-precision-constant --param=highcode-gen-section-name=1 -pipe -Wunused -Wuninitialized -Wall -Wshadow -Waggregate-return -Wfloat-equal -g -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/Debug" -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/Core" -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/User" -I"/home/vitor/Documentos/Facul/AAM/Projeto_final/Codigo/ProjetoFinalAAM/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@

