################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include src/subdir.mk
-include .metadata/.plugins/org.eclipse.cdt.make.core/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += \
SpaceInvaders_project.elf.size \

ELFCHECK += \
SpaceInvaders_project.elf.elfcheck \


# All Target
all: SpaceInvaders_project.elf secondary-outputs

# Tool invocations
SpaceInvaders_project.elf: $(OBJS) ../src/lscript.ld $(USER_OBJS)
	@echo Building target: $@
	@echo Invoking: MicroBlaze gcc linker
	mb-gcc -Wl,-T -Wl,../src/lscript.ld -L../../hello_world_bsp_0/microblaze_0/lib -mlittle-endian -mxl-barrel-shift -mxl-pattern-compare -mcpu=v8.20.b -mno-xl-soft-mul -o"SpaceInvaders_project.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo Finished building target: $@
	@echo ' '

SpaceInvaders_project.elf.size: SpaceInvaders_project.elf
	@echo Invoking: MicroBlaze Print Size
	mb-size SpaceInvaders_project.elf  |tee "SpaceInvaders_project.elf.size"
	@echo Finished building: $@
	@echo ' '

SpaceInvaders_project.elf.elfcheck: SpaceInvaders_project.elf
	@echo Invoking: Xilinx ELF Check
	elfcheck SpaceInvaders_project.elf -hw ../../SpaceInvaders_HW_hw_platform/system.xml -pe microblaze_0  |tee "SpaceInvaders_project.elf.elfcheck"
	@echo Finished building: $@
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES)$(ELFSIZE)$(ELFCHECK)$(S_UPPER_DEPS) SpaceInvaders_project.elf
	-@echo ' '

secondary-outputs: $(ELFSIZE) $(ELFCHECK)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
