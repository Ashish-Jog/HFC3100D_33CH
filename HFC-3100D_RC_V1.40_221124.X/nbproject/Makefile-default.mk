#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-default.mk)" "nbproject/Makefile-local-default.mk"
include nbproject/Makefile-local-default.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=default
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/HFC-3100D_RC_V1.40_221124.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=${DISTDIR}/HFC-3100D_RC_V1.40_221124.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

ifeq ($(COMPARE_BUILD), true)
COMPARISON_BUILD=
else
COMPARISON_BUILD=
endif

ifdef SUB_IMAGE_ADDRESS

else
SUB_IMAGE_ADDRESS_COMMAND=
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=Compensator_PSFB.c Init_PSFB.c Main_PSFB.c Variables_PSFB.c delay.c eCAN.c RS232.c RS485.c Interrupt.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/Compensator_PSFB.o ${OBJECTDIR}/Init_PSFB.o ${OBJECTDIR}/Main_PSFB.o ${OBJECTDIR}/Variables_PSFB.o ${OBJECTDIR}/delay.o ${OBJECTDIR}/eCAN.o ${OBJECTDIR}/RS232.o ${OBJECTDIR}/RS485.o ${OBJECTDIR}/Interrupt.o
POSSIBLE_DEPFILES=${OBJECTDIR}/Compensator_PSFB.o.d ${OBJECTDIR}/Init_PSFB.o.d ${OBJECTDIR}/Main_PSFB.o.d ${OBJECTDIR}/Variables_PSFB.o.d ${OBJECTDIR}/delay.o.d ${OBJECTDIR}/eCAN.o.d ${OBJECTDIR}/RS232.o.d ${OBJECTDIR}/RS485.o.d ${OBJECTDIR}/Interrupt.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/Compensator_PSFB.o ${OBJECTDIR}/Init_PSFB.o ${OBJECTDIR}/Main_PSFB.o ${OBJECTDIR}/Variables_PSFB.o ${OBJECTDIR}/delay.o ${OBJECTDIR}/eCAN.o ${OBJECTDIR}/RS232.o ${OBJECTDIR}/RS485.o ${OBJECTDIR}/Interrupt.o

# Source Files
SOURCEFILES=Compensator_PSFB.c Init_PSFB.c Main_PSFB.c Variables_PSFB.c delay.c eCAN.c RS232.c RS485.c Interrupt.c



CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

.build-conf:  ${BUILD_SUBPROJECTS}
ifneq ($(INFORMATION_MESSAGE), )
	@echo $(INFORMATION_MESSAGE)
endif
	${MAKE}  -f nbproject/Makefile-default.mk ${DISTDIR}/HFC-3100D_RC_V1.40_221124.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}

MP_PROCESSOR_OPTION=33FJ64GS606
MP_LINKER_FILE_OPTION=,-Tp33FJ64GS606.gld
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/Compensator_PSFB.o: Compensator_PSFB.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Compensator_PSFB.o.d 
	@${RM} ${OBJECTDIR}/Compensator_PSFB.o.ok ${OBJECTDIR}/Compensator_PSFB.o.err 
	@${RM} ${OBJECTDIR}/Compensator_PSFB.o 
	@${FIXDEPS} "${OBJECTDIR}/Compensator_PSFB.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Compensator_PSFB.o.d" -o ${OBJECTDIR}/Compensator_PSFB.o Compensator_PSFB.c    
	
${OBJECTDIR}/Init_PSFB.o: Init_PSFB.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Init_PSFB.o.d 
	@${RM} ${OBJECTDIR}/Init_PSFB.o.ok ${OBJECTDIR}/Init_PSFB.o.err 
	@${RM} ${OBJECTDIR}/Init_PSFB.o 
	@${FIXDEPS} "${OBJECTDIR}/Init_PSFB.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Init_PSFB.o.d" -o ${OBJECTDIR}/Init_PSFB.o Init_PSFB.c    
	
${OBJECTDIR}/Main_PSFB.o: Main_PSFB.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Main_PSFB.o.d 
	@${RM} ${OBJECTDIR}/Main_PSFB.o.ok ${OBJECTDIR}/Main_PSFB.o.err 
	@${RM} ${OBJECTDIR}/Main_PSFB.o 
	@${FIXDEPS} "${OBJECTDIR}/Main_PSFB.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Main_PSFB.o.d" -o ${OBJECTDIR}/Main_PSFB.o Main_PSFB.c    
	
${OBJECTDIR}/Variables_PSFB.o: Variables_PSFB.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Variables_PSFB.o.d 
	@${RM} ${OBJECTDIR}/Variables_PSFB.o.ok ${OBJECTDIR}/Variables_PSFB.o.err 
	@${RM} ${OBJECTDIR}/Variables_PSFB.o 
	@${FIXDEPS} "${OBJECTDIR}/Variables_PSFB.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Variables_PSFB.o.d" -o ${OBJECTDIR}/Variables_PSFB.o Variables_PSFB.c    
	
${OBJECTDIR}/delay.o: delay.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/delay.o.d 
	@${RM} ${OBJECTDIR}/delay.o.ok ${OBJECTDIR}/delay.o.err 
	@${RM} ${OBJECTDIR}/delay.o 
	@${FIXDEPS} "${OBJECTDIR}/delay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/delay.o.d" -o ${OBJECTDIR}/delay.o delay.c    
	
${OBJECTDIR}/eCAN.o: eCAN.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/eCAN.o.d 
	@${RM} ${OBJECTDIR}/eCAN.o.ok ${OBJECTDIR}/eCAN.o.err 
	@${RM} ${OBJECTDIR}/eCAN.o 
	@${FIXDEPS} "${OBJECTDIR}/eCAN.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/eCAN.o.d" -o ${OBJECTDIR}/eCAN.o eCAN.c    
	
${OBJECTDIR}/RS232.o: RS232.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/RS232.o.d 
	@${RM} ${OBJECTDIR}/RS232.o.ok ${OBJECTDIR}/RS232.o.err 
	@${RM} ${OBJECTDIR}/RS232.o 
	@${FIXDEPS} "${OBJECTDIR}/RS232.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/RS232.o.d" -o ${OBJECTDIR}/RS232.o RS232.c    
	
${OBJECTDIR}/RS485.o: RS485.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/RS485.o.d 
	@${RM} ${OBJECTDIR}/RS485.o.ok ${OBJECTDIR}/RS485.o.err 
	@${RM} ${OBJECTDIR}/RS485.o 
	@${FIXDEPS} "${OBJECTDIR}/RS485.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/RS485.o.d" -o ${OBJECTDIR}/RS485.o RS485.c    
	
${OBJECTDIR}/Interrupt.o: Interrupt.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Interrupt.o.d 
	@${RM} ${OBJECTDIR}/Interrupt.o.ok ${OBJECTDIR}/Interrupt.o.err 
	@${RM} ${OBJECTDIR}/Interrupt.o 
	@${FIXDEPS} "${OBJECTDIR}/Interrupt.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE) -g -D__DEBUG  -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Interrupt.o.d" -o ${OBJECTDIR}/Interrupt.o Interrupt.c    
	
else
${OBJECTDIR}/Compensator_PSFB.o: Compensator_PSFB.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Compensator_PSFB.o.d 
	@${RM} ${OBJECTDIR}/Compensator_PSFB.o.ok ${OBJECTDIR}/Compensator_PSFB.o.err 
	@${RM} ${OBJECTDIR}/Compensator_PSFB.o 
	@${FIXDEPS} "${OBJECTDIR}/Compensator_PSFB.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Compensator_PSFB.o.d" -o ${OBJECTDIR}/Compensator_PSFB.o Compensator_PSFB.c    
	
${OBJECTDIR}/Init_PSFB.o: Init_PSFB.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Init_PSFB.o.d 
	@${RM} ${OBJECTDIR}/Init_PSFB.o.ok ${OBJECTDIR}/Init_PSFB.o.err 
	@${RM} ${OBJECTDIR}/Init_PSFB.o 
	@${FIXDEPS} "${OBJECTDIR}/Init_PSFB.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Init_PSFB.o.d" -o ${OBJECTDIR}/Init_PSFB.o Init_PSFB.c    
	
${OBJECTDIR}/Main_PSFB.o: Main_PSFB.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Main_PSFB.o.d 
	@${RM} ${OBJECTDIR}/Main_PSFB.o.ok ${OBJECTDIR}/Main_PSFB.o.err 
	@${RM} ${OBJECTDIR}/Main_PSFB.o 
	@${FIXDEPS} "${OBJECTDIR}/Main_PSFB.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Main_PSFB.o.d" -o ${OBJECTDIR}/Main_PSFB.o Main_PSFB.c    
	
${OBJECTDIR}/Variables_PSFB.o: Variables_PSFB.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Variables_PSFB.o.d 
	@${RM} ${OBJECTDIR}/Variables_PSFB.o.ok ${OBJECTDIR}/Variables_PSFB.o.err 
	@${RM} ${OBJECTDIR}/Variables_PSFB.o 
	@${FIXDEPS} "${OBJECTDIR}/Variables_PSFB.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Variables_PSFB.o.d" -o ${OBJECTDIR}/Variables_PSFB.o Variables_PSFB.c    
	
${OBJECTDIR}/delay.o: delay.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/delay.o.d 
	@${RM} ${OBJECTDIR}/delay.o.ok ${OBJECTDIR}/delay.o.err 
	@${RM} ${OBJECTDIR}/delay.o 
	@${FIXDEPS} "${OBJECTDIR}/delay.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/delay.o.d" -o ${OBJECTDIR}/delay.o delay.c    
	
${OBJECTDIR}/eCAN.o: eCAN.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/eCAN.o.d 
	@${RM} ${OBJECTDIR}/eCAN.o.ok ${OBJECTDIR}/eCAN.o.err 
	@${RM} ${OBJECTDIR}/eCAN.o 
	@${FIXDEPS} "${OBJECTDIR}/eCAN.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/eCAN.o.d" -o ${OBJECTDIR}/eCAN.o eCAN.c    
	
${OBJECTDIR}/RS232.o: RS232.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/RS232.o.d 
	@${RM} ${OBJECTDIR}/RS232.o.ok ${OBJECTDIR}/RS232.o.err 
	@${RM} ${OBJECTDIR}/RS232.o 
	@${FIXDEPS} "${OBJECTDIR}/RS232.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/RS232.o.d" -o ${OBJECTDIR}/RS232.o RS232.c    
	
${OBJECTDIR}/RS485.o: RS485.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/RS485.o.d 
	@${RM} ${OBJECTDIR}/RS485.o.ok ${OBJECTDIR}/RS485.o.err 
	@${RM} ${OBJECTDIR}/RS485.o 
	@${FIXDEPS} "${OBJECTDIR}/RS485.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/RS485.o.d" -o ${OBJECTDIR}/RS485.o RS485.c    
	
${OBJECTDIR}/Interrupt.o: Interrupt.c  nbproject/Makefile-${CND_CONF}.mk 
	@${MKDIR} "${OBJECTDIR}" 
	@${RM} ${OBJECTDIR}/Interrupt.o.d 
	@${RM} ${OBJECTDIR}/Interrupt.o.ok ${OBJECTDIR}/Interrupt.o.err 
	@${RM} ${OBJECTDIR}/Interrupt.o 
	@${FIXDEPS} "${OBJECTDIR}/Interrupt.o.d" $(SILENT) -rsi ${MP_CC_DIR}../ -c ${MP_CC} $(MP_EXTRA_CC_PRE)  -g -omf=elf -x c -c -mcpu=$(MP_PROCESSOR_OPTION) -Wall -I"." -MMD -MF "${OBJECTDIR}/Interrupt.o.d" -o ${OBJECTDIR}/Interrupt.o Interrupt.c    
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${DISTDIR}/HFC-3100D_RC_V1.40_221124.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -D__DEBUG  -o ${DISTDIR}/HFC-3100D_RC_V1.40_221124.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}         -Wl,--defsym=__MPLAB_BUILD=1,-L".",-Map="${DISTDIR}/HFC-3100D_RC.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__ICD2RAM=1,--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1
else
${DISTDIR}/HFC-3100D_RC_V1.40_221124.X.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   
	@${MKDIR} ${DISTDIR} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -omf=elf -mcpu=$(MP_PROCESSOR_OPTION)  -o ${DISTDIR}/HFC-3100D_RC_V1.40_221124.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}         -Wl,--defsym=__MPLAB_BUILD=1,-L".",-Map="${DISTDIR}/HFC-3100D_RC.${IMAGE_TYPE}.map",--report-mem$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION)
	${MP_CC_DIR}\\pic30-bin2hex ${DISTDIR}/HFC-3100D_RC_V1.40_221124.X.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} -omf=elf
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${OBJECTDIR}
	${RM} -r ${DISTDIR}

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
