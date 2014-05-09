################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../config.c \
../main.c \
../thread01_net1_data_comm.c 

OBJS += \
./config.o \
./main.o \
./thread01_net1_data_comm.o 

C_DEPS += \
./config.d \
./main.d \
./thread01_net1_data_comm.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-arago-linux-gnueabi-gcc -I"/root/Dropbox/Linux_Project/rtu-ii/protocol/iec104" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


