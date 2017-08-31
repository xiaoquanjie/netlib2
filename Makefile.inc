#all src dir defination
ALL_DIR_SRC		?=

#srcs defination
SUFFIX_SRCS		:= .cpp .cc .c
SUFFIX_SRCS_PAT		:= $(foreach var, $(SUFFIX_SRCS), %$(var))
SUFFIX_SRCS_WD		:= $(foreach var, $(SUFFIX_SRCS), *$(var))
SRCS_WD_PATH	 	 = $(foreach var, $(SUFFIX_SRCS_WD), $(1)/$(var)) 
ALL_SRC			 = $(foreach var, $(ALL_DIR_SRC), $(wildcard $(call SRCS_WD_PATH, $(var))))

#complie defination
INCLUDES		?=
CPPFLAGS		?=
CXXFLAGS		 = $(CPPFLAGS) $(INCLUDES)

#objs defination
ALL_OBJ_PAT		 = $(foreach var, $(SUFFIX_SRCS_PAT), $(patsubst $(var), %.o, $(1)))
ALL_OBJ			 = $(foreach var, $(ALL_SRC), $(filter %.o, $(call ALL_OBJ_PAT, $(var))))

RUNO			 = $(CXX) $(CXXFLAGS) -c $^ -o $@

define OBJ_RULE
$(basename $(1)).o : $(1)
	@echo $$@
	$$(RUNO)
endef

$(foreach var, $(ALL_SRC), $(eval $(call OBJ_RULE, $(var))))


.PHONY : clean
clean :
	rm -rf $(ALL_OBJ) 

