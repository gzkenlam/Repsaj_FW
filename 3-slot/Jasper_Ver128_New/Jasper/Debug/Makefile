################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../A2d.c \
../BattAuth.c \
../BattChrg.c \
../BattComm.c \
../BattFound.c \
../Debug.c \
../Eeprom.c \
../I2C_Batt.c \
../Led.c \
../Main.c \
../Ports.c \
../Pwm.c \
../Timers.c \
../Uart.c


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
A2d.o \
BattAuth.o \
BattChrg.o \
BattComm.o \
BattFound.o \
Debug.o \
Eeprom.o \
I2C_Batt.o \
Led.o \
Main.o \
Ports.o \
Pwm.o \
Timers.o \
Uart.o

OBJS_AS_ARGS +=  \
A2d.o \
BattAuth.o \
BattChrg.o \
BattComm.o \
BattFound.o \
Debug.o \
Eeprom.o \
I2C_Batt.o \
Led.o \
Main.o \
Ports.o \
Pwm.o \
Timers.o \
Uart.o

C_DEPS +=  \
A2d.d \
BattAuth.d \
BattChrg.d \
BattComm.d \
BattFound.d \
Debug.d \
Eeprom.d \
I2C_Batt.d \
Led.d \
Main.d \
Ports.d \
Pwm.d \
Timers.d \
Uart.d

C_DEPS_AS_ARGS +=  \
A2d.d \
BattAuth.d \
BattChrg.d \
BattComm.d \
BattFound.d \
Debug.d \
Eeprom.d \
I2C_Batt.d \
Led.d \
Main.d \
Ports.d \
Pwm.d \
Timers.d \
Uart.d

OUTPUT_FILE_PATH +=Jasper.elf

OUTPUT_FILE_PATH_AS_ARGS +=Jasper.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler





























./%.o: .././%.c
	@echo Building file: $<
	@echo Invoking: AVR/GNU C Compiler : 4.8.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE)  -x c -funsigned-char -funsigned-bitfields -DDEBUG  -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -g2 -Wall -mmcu=atxmega32a4u -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: AVR/GNU Linker : 4.8.1
	$(QUOTE)C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-gcc.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="Jasper.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,-section-start=.FWVersion=0x7ffc  -mmcu=atxmega32a4u  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "Jasper.elf" "Jasper.hex"
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "Jasper.elf" "Jasper.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "Jasper.elf" > "Jasper.lss"
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "Jasper.elf" "Jasper.srec"
	"C:\Program Files (x86)\Atmel\Atmel Toolchain\AVR8 GCC\Native\3.4.1061\avr8-gnu-toolchain\bin\avr-size.exe" "Jasper.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "Jasper.elf" "Jasper.a" "Jasper.hex" "Jasper.lss" "Jasper.eep" "Jasper.map" "Jasper.srec" "Jasper.usersignatures"
	