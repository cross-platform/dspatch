################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/home/parallels/Projects/DSPatch/example/DspAudioDevice.cpp \
/home/parallels/Projects/DSPatch/example/DspOscillator.cpp \
/home/parallels/Projects/DSPatch/example/DspWaveStreamer.cpp \
/home/parallels/Projects/DSPatch/example/main.cpp 

OBJS += \
./DspAudioDevice.o \
./DspOscillator.o \
./DspWaveStreamer.o \
./main.o 

CPP_DEPS += \
./DspAudioDevice.d \
./DspOscillator.d \
./DspWaveStreamer.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
DspAudioDevice.o: /home/parallels/Projects/DSPatch/example/DspAudioDevice.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__LINUX_ALSA__ -I../rtaudio/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

DspOscillator.o: /home/parallels/Projects/DSPatch/example/DspOscillator.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__LINUX_ALSA__ -I../rtaudio/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

DspWaveStreamer.o: /home/parallels/Projects/DSPatch/example/DspWaveStreamer.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__LINUX_ALSA__ -I../rtaudio/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.o: /home/parallels/Projects/DSPatch/example/main.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__LINUX_ALSA__ -I../rtaudio/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


