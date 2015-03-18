# -*- mode: mak; indent-tabs-mode: t; tab-width: 8; -*-
# Makefile for MC libwin32
#
#

ROOT=		..
MAKEFLAGS=

include		../Makefile.common

D_OBJ:=		$(D_OBJ)/libw32

CINCLUDE=	$(ISWITCH).
ifdef MSVC
CINCLUDE+=	$(ISWITCH)./msvc
endif
CEXTRA=		-DHAVE_CONFIG_H
CFLAGS=		$(CDEBUG) $(COPT) $(CWARN) $(CINCLUDE) $(CEXTRA)
LDLIBS=
LDFLAGS=	$(LDOPT) $(LDEBUG) $(LDLIBS)

RMFLAGS=	-f
ARFLAGS=	-nologo

ifndef MSVC
CFLAGS:=	$(subst /,\,$(CFLAGS))
LDFLAGS:=	$(subst /,\,$(LDFLAGS))
endif

############################################################

TARGET=			$(D_LIB)/libw32$(A)

OBJS=	\
	$(D_OBJ)/w32_check$(O)		\
	$(D_OBJ)/w32_child$(O)		\
	$(D_OBJ)/w32_chown$(O)		\
	$(D_OBJ)/w32_close$(O)		\
	$(D_OBJ)/w32_dir$(O)		\
	$(D_OBJ)/w32_dirent$(O)		\
	$(D_OBJ)/w32_errno$(O)		\
	$(D_OBJ)/w32_fcntl$(O)		\
	$(D_OBJ)/w32_fsync$(O)		\
	$(D_OBJ)/w32_getcwd$(O)		\
	$(D_OBJ)/w32_gethostname$(O)	\
	$(D_OBJ)/w32_getopt$(O)		\
	$(D_OBJ)/w32_gistrerror$(O)	\
	$(D_OBJ)/w32_grp$(O)		\
	$(D_OBJ)/w32_ino$(O)		\
	$(D_OBJ)/w32_io$(O)		\
	$(D_OBJ)/w32_langinfo$(O)	\
	$(D_OBJ)/w32_link$(O)		\
	$(D_OBJ)/w32_mknod$(O)		\
	$(D_OBJ)/w32_mkstemp$(O)	\
	$(D_OBJ)/w32_mmap$(O)		\
	$(D_OBJ)/w32_neterr$(O)		\
	$(D_OBJ)/w32_popen$(O)		\
	$(D_OBJ)/w32_pwd$(O)		\
	$(D_OBJ)/w32_read$(O)		\
	$(D_OBJ)/w32_shell$(O)		\
	$(D_OBJ)/w32_signal$(O)		\
	$(D_OBJ)/w32_slang$(O)		\
	$(D_OBJ)/w32_socket$(O)		\
	$(D_OBJ)/w32_statfs$(O)		\
	$(D_OBJ)/w32_strftime$(O)	\
	$(D_OBJ)/w32_string$(O)		\
	$(D_OBJ)/w32_sysdir$(O)		\
	$(D_OBJ)/w32_time$(O)		\
	$(D_OBJ)/w32_trace$(O)		\
	$(D_OBJ)/w32_truncate$(O)	\
	$(D_OBJ)/w32_uname$(O)		\
	$(D_OBJ)/w32_unlink$(O)		\
	$(D_OBJ)/w32_user$(O)		\
	$(D_OBJ)/w32_util$(O)		\
	$(D_OBJ)/w32_write$(O)		\
	\
	$(D_OBJ)/bsd_qsort$(O)		\
	$(D_OBJ)/bsd_bsearch$(O)

all:		object $(TARGET)

$(TARGET):		$(OBJS)
		$(AR) $(ARFLAGS) /OUT:$@ $^

object:				$(D_OBJ)/.created
$(D_OBJ)/.created:
		-@$(MKDIR) $(D_OBJ)
		@echo "created, do no delete" > $@

clean:
		-$(RM) $(RMFLAGS) $(TARGET) *.pdb *.err >nul 2>&1
		-$(RM) $(OBJS) $(subst $(O),.mbr,$(OBJS)) >nul 2>&1

$(D_OBJ)/%$(O) : %.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

#end

