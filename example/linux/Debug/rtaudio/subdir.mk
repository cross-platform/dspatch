################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/home/parallels/Projects/DSPatch/example/rtaudio/RtAudio.cpp 

OBJS += \
./rtaudio/RtAudio.o 

CPP_DEPS += \
./rtaudio/RtAudio.d 


# Each subdirectory must supply rules for building sources it contributes
rtaudio/RtAudio.o: /home/parallels/Projects/DSPatch/example/rtaudio/RtAudio.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -D__LINUX_ALSA__ -I../rtaudio/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


