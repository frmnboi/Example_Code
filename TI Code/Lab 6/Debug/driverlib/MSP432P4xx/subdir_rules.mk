################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
driverlib/MSP432P4xx/%.obj: ../driverlib/MSP432P4xx/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="C:/ti/ccs910/ccs/ccs_base/arm/include" --include_path="C:/Users/Daniel Foreman/Desktop/Coding/TI Code/Lab 6/driverlib/MSP432P4xx" --include_path="C:/Users/Daniel Foreman/Desktop/Coding/TI Code/Lab 6/driverlib" --include_path="C:/ti/ccs910/ccs/ccs_base/arm/include/CMSIS" --include_path="C:/Users/Daniel Foreman/Desktop/Coding/TI Code/Lab 6" --include_path="C:/ti/ccs910/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --advice:power=all --define=__MSP432P401R__ --define=ccs -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi --preproc_with_compile --preproc_dependency="driverlib/MSP432P4xx/$(basename $(<F)).d_raw" --obj_directory="driverlib/MSP432P4xx" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


