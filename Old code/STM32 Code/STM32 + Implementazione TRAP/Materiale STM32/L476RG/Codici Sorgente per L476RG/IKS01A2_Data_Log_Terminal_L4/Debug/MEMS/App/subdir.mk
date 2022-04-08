################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MEMS/App/app_mems.c 

OBJS += \
./MEMS/App/app_mems.o 

C_DEPS += \
./MEMS/App/app_mems.d 


# Each subdirectory must supply rules for building sources it contributes
MEMS/App/%.o: ../MEMS/App/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -DUSE_HAL_DRIVER -DSTM32L476xx -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/MEMS/App" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/MEMS/Target" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Core/Inc" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/BSP/STM32L4xx_Nucleo" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/STM32L4xx_HAL_Driver/Inc" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/CMSIS/Device/ST/STM32L4xx/Include" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/CMSIS/Include" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/BSP/Components/lsm6dsl" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/BSP/Components/lsm303agr" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/BSP/Components/hts221" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/BSP/Components/lps22hb" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/BSP/IKS01A2" -I"C:/Users/Embedded Systems/Documents/IKS01A2_6D_Orientation_L4/Drivers/BSP/Components/Common"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


