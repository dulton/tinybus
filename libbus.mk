##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=libbus
ConfigurationName      :=Debug
WorkspacePath          := "E:\workroom\libbus"
ProjectPath            := "E:\workroom\libbus"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=张世勇
Date                   :=04/07/14
CodeLitePath           :="C:\Program Files (x86)\CodeLite"
LinkerName             :=gcc
SharedObjectLinkerName :=gcc -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.o.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/lib$(ProjectName).a
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E 
ObjectsFileList        :="libbus.txt"
PCHCompileFlags        :=
MakeDirCommand         :=makedir
RcCmpOptions           := 
RcCompilerName         :=windres
LinkOptions            :=  
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). $(IncludeSwitch)./linux 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := ar rcus
CXX      := gcc
CC       := gcc
CXXFLAGS :=  -g $(Preprocessors)
CFLAGS   :=  -g $(Preprocessors)
ASFLAGS  := 
AS       := as


##
## User defined environment variables
##
CodeLiteDir:=C:\Program Files (x86)\CodeLite
UNIT_TEST_PP_SRC_DIR:=C:\Program Files (x86)\UnitTest++-1.3
Objects0=$(IntermediateDirectory)/tinybus$(ObjectSuffix) $(IntermediateDirectory)/queue$(ObjectSuffix) $(IntermediateDirectory)/asyncqueue$(ObjectSuffix) 



Objects=$(Objects0) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild
all: $(IntermediateDirectory) $(OutputFile)

$(OutputFile): $(Objects)
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	$(AR) $(ArchiveOutputSwitch)$(OutputFile) @$(ObjectsFileList) $(ArLibs)
	@$(MakeDirCommand) "E:\workroom\libbus/.build-debug"
	@echo rebuilt > "E:\workroom\libbus/.build-debug/libbus"

./Debug:
	@$(MakeDirCommand) "./Debug"

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/tinybus$(ObjectSuffix): tinybus.c $(IntermediateDirectory)/tinybus$(DependSuffix)
	$(CC) $(SourceSwitch) "E:/workroom/libbus/tinybus.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/tinybus$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/tinybus$(DependSuffix): tinybus.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/tinybus$(ObjectSuffix) -MF$(IntermediateDirectory)/tinybus$(DependSuffix) -MM "tinybus.c"

$(IntermediateDirectory)/tinybus$(PreprocessSuffix): tinybus.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/tinybus$(PreprocessSuffix) "tinybus.c"

$(IntermediateDirectory)/queue$(ObjectSuffix): queue.c $(IntermediateDirectory)/queue$(DependSuffix)
	$(CC) $(SourceSwitch) "E:/workroom/libbus/queue.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/queue$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/queue$(DependSuffix): queue.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/queue$(ObjectSuffix) -MF$(IntermediateDirectory)/queue$(DependSuffix) -MM "queue.c"

$(IntermediateDirectory)/queue$(PreprocessSuffix): queue.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/queue$(PreprocessSuffix) "queue.c"

$(IntermediateDirectory)/asyncqueue$(ObjectSuffix): asyncqueue.c $(IntermediateDirectory)/asyncqueue$(DependSuffix)
	$(CC) $(SourceSwitch) "E:/workroom/libbus/asyncqueue.c" $(CFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/asyncqueue$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/asyncqueue$(DependSuffix): asyncqueue.c
	@$(CC) $(CFLAGS) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/asyncqueue$(ObjectSuffix) -MF$(IntermediateDirectory)/asyncqueue$(DependSuffix) -MM "asyncqueue.c"

$(IntermediateDirectory)/asyncqueue$(PreprocessSuffix): asyncqueue.c
	@$(CC) $(CFLAGS) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/asyncqueue$(PreprocessSuffix) "asyncqueue.c"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) $(IntermediateDirectory)/tinybus$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/tinybus$(DependSuffix)
	$(RM) $(IntermediateDirectory)/tinybus$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/queue$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/queue$(DependSuffix)
	$(RM) $(IntermediateDirectory)/queue$(PreprocessSuffix)
	$(RM) $(IntermediateDirectory)/asyncqueue$(ObjectSuffix)
	$(RM) $(IntermediateDirectory)/asyncqueue$(DependSuffix)
	$(RM) $(IntermediateDirectory)/asyncqueue$(PreprocessSuffix)
	$(RM) $(OutputFile)
	$(RM) $(OutputFile)
	$(RM) ".build-debug/libbus"


