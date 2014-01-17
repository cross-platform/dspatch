################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/home/parallels/Projects/DSPatch/src/DSPatch.cpp \
/home/parallels/Projects/DSPatch/src/DspCircuit.cpp \
/home/parallels/Projects/DSPatch/src/DspCircuitThread.cpp \
/home/parallels/Projects/DSPatch/src/DspComponent.cpp \
/home/parallels/Projects/DSPatch/src/DspComponentThread.cpp \
/home/parallels/Projects/DSPatch/src/DspSignal.cpp \
/home/parallels/Projects/DSPatch/src/DspSignalBus.cpp \
/home/parallels/Projects/DSPatch/src/DspWireBus.cpp 

OBJS += \
./src/DSPatch.o \
./src/DspCircuit.o \
./src/DspCircuitThread.o \
./src/DspComponent.o \
./src/DspComponentThread.o \
./src/DspSignal.o \
./src/DspSignalBus.o \
./src/DspWireBus.o 

CPP_DEPS += \
./src/DSPatch.d \
./src/DspCircuit.d \
./src/DspCircuitThread.d \
./src/DspComponent.d \
./src/DspComponentThread.d \
./src/DspSignal.d \
./src/DspSignalBus.d \
./src/DspWireBus.d 


# Each subdirectory must supply rules for building sources it contributes
src/DSPatch.o: /home/parallels/Projects/DSPatch/src/DSPatch.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/DspCircuit.o: /home/parallels/Projects/DSPatch/src/DspCircuit.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/DspCircuitThread.o: /home/parallels/Projects/DSPatch/src/DspCircuitThread.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/DspComponent.o: /home/parallels/Projects/DSPatch/src/DspComponent.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/DspComponentThread.o: /home/parallels/Projects/DSPatch/src/DspComponentThread.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/DspSignal.o: /home/parallels/Projects/DSPatch/src/DspSignal.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/DspSignalBus.o: /home/parallels/Projects/DSPatch/src/DspSignalBus.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/DspWireBus.o: /home/parallels/Projects/DSPatch/src/DspWireBus.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


