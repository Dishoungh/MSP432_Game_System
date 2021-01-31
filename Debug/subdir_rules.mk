################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"/opt/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-7-2017-q4-major/bin/arm-none-eabi-gcc-7.2.1" -c -mcpu=cortex-m4 -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -D__MSP432P401R__ -Dgcc -I"/opt/ti/ccs1020/ccs/ccs_base/arm/include" -I"/opt/ti/ccs1020/ccs/ccs_base/arm/include/CMSIS" -I"/home/dishoungh/git/MSP432_Game_System" -I"/opt/ti/ccs1020/ccs/tools/compiler/gcc-arm-none-eabi-7-2017-q4-major/arm-none-eabi/include" -g -gdwarf-3 -gstrict-dwarf -Wall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$(shell echo $<)"
	@echo 'Finished building: "$<"'
	@echo ' '


