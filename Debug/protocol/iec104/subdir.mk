################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../protocol/iec104/iec104.c 

OBJS += \
./protocol/iec104/iec104.o 

C_DEPS += \
./protocol/iec104/iec104.d 


# Each subdirectory must supply rules for building sources it contributes
protocol/iec104/%.o: ../protocol/iec104/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-arago-linux-gnueabi-gcc -I"/root/Dropbox/Linux_Project/rtu-ii/protocol/iec104" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


