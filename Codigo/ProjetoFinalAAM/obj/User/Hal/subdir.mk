################################################################################
# MRS Version: 2.2.0
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../User/Hal/Hal.c 

C_DEPS += \
./User/Hal/Hal.d 

OBJS += \
./User/Hal/Hal.o 


EXPANDS += \
./User/Hal/Hal.c.234r.expand 



# Each subdirectory must supply rules for building sources it contributes
User/Hal/%.o: ../User/Hal/%.c
	@	riscv-none-embed-gcc -march=rv32ecxw -mabi=ilp32e -msmall-data-limit=0 -msave-restore -fmax-errors=20 -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common -Wunused -Wuninitialized -g -I"/home/vitor/mounriver-studio-projects/ProjetoFinalAAM/Debug" -I"/home/vitor/mounriver-studio-projects/ProjetoFinalAAM/Core" -I"/home/vitor/mounriver-studio-projects/ProjetoFinalAAM/User" -I"/home/vitor/mounriver-studio-projects/ProjetoFinalAAM/Peripheral/inc" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"

