################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Modbus/Modbus_Parser.c \
../Drivers/Modbus/Modbus_Server.c 

OBJS += \
./Drivers/Modbus/Modbus_Parser.o \
./Drivers/Modbus/Modbus_Server.o 

C_DEPS += \
./Drivers/Modbus/Modbus_Parser.d \
./Drivers/Modbus/Modbus_Server.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Modbus/%.o Drivers/Modbus/%.su Drivers/Modbus/%.cyclo: ../Drivers/Modbus/%.c Drivers/Modbus/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DCORE_CM7 -DUSE_HAL_DRIVER -DSTM32H755xx -DUSE_PWR_DIRECT_SMPS_SUPPLY -c -I../LWIP/App -I"/Users/arunrawat/STM32CubeIDE/workspace_2/STM32_Modbus_TCP_Server/CM7/Drivers/Modbus" -I../LWIP/Target -I../Core/Inc -I../../Middlewares/Third_Party/LwIP/src/include -I../../Middlewares/Third_Party/LwIP/system -I../../Drivers/STM32H7xx_HAL_Driver/Inc -I../../Drivers/STM32H7xx_HAL_Driver/Inc/Legacy -I../../Drivers/BSP/Components/lan8742 -I../../Middlewares/Third_Party/LwIP/src/include/netif/ppp -I../../Drivers/CMSIS/Device/ST/STM32H7xx/Include -I../../Middlewares/Third_Party/LwIP/src/include/lwip -I../../Middlewares/Third_Party/LwIP/src/include/lwip/apps -I../../Middlewares/Third_Party/LwIP/src/include/lwip/priv -I../../Middlewares/Third_Party/LwIP/src/include/lwip/prot -I../../Middlewares/Third_Party/LwIP/src/include/netif -I../../Middlewares/Third_Party/LwIP/src/include/compat/posix -I../../Middlewares/Third_Party/LwIP/src/include/compat/posix/arpa -I../../Middlewares/Third_Party/LwIP/src/include/compat/posix/net -I../../Middlewares/Third_Party/LwIP/src/include/compat/posix/sys -I../../Middlewares/Third_Party/LwIP/src/include/compat/stdc -I../../Middlewares/Third_Party/LwIP/system/arch -I../../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Drivers-2f-Modbus

clean-Drivers-2f-Modbus:
	-$(RM) ./Drivers/Modbus/Modbus_Parser.cyclo ./Drivers/Modbus/Modbus_Parser.d ./Drivers/Modbus/Modbus_Parser.o ./Drivers/Modbus/Modbus_Parser.su ./Drivers/Modbus/Modbus_Server.cyclo ./Drivers/Modbus/Modbus_Server.d ./Drivers/Modbus/Modbus_Server.o ./Drivers/Modbus/Modbus_Server.su

.PHONY: clean-Drivers-2f-Modbus

