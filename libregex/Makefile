# -*- mode: mak; indent-tabs-mode: t; tab-width: 8; -*-
# win32 libregex
#
#

ROOT=		..
MAKEFLAGS=

include		../Makefile.common

CINCLUDE=	$(ISWITCH). $(ISWITCH)../libw32 $(ISWITCH)../libregex $(ISWITCH)..
CEXTRA=		-DHAVE_CONFIG_H
CFLAGS=		$(CDEBUG) $(COPT) $(CWARN) $(CINCLUDE) $(CEXTRA)
LDLIBS=		$(LW)w32$(A) $(LW)regex$(A) ole32$(A) user32$(A) shell32$(A) wsock32$(A)
LDFLAGS=	$(LDOPT) $(LDEBUG)

ifndef MSVC
CFLAGS:=	$(subst /,\,$(CFLAGS))
LDFLAGS:=	$(subst /,\,$(LDFLAGS))
LDLIBS:=	$(subst /,\,$(LDLIBS))
endif

RMFLAGS=	-f
ARFLAGS=	-nologo

############################################################

D_OBJ:=			$(D_OBJ)/libregex
LIBREGEX=		$(LW)regex$(A)

VPATH=			.

OBJS=\
	$(D_OBJ)/regcomp$(O)	\
	$(D_OBJ)/regexec$(O)	\
	$(D_OBJ)/regerror$(O)	\
	$(D_OBJ)/regfree$(O)

all:			$(LIBREGEX)

$(LIBREGEX):		$(D_OBJ)/.created $(OBJS)
		$(AR) $(ARFLAGS) /OUT:$@ $(OBJS)

$(D_OBJ)/.created:
		-@$(MKDIR) $(D_OBJ)
		@echo "created, do no delete" > $@

clean:
		-$(RM) $(RMFLAGS) $(LIBREGEX) $(OBJS) $(subst $(O),.mbr,$(OBJS)) vc*.pdb >nul 2>&1

$(D_OBJ)/%$(O) : %.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

#end

