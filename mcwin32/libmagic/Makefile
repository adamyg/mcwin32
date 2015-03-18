# -*- mode: mak; indent-tabs-mode: t; tab-width: 8; -*-
# $Id: Makefile.in,v 1.10 2004/12/01 18:54:56 ayoung Exp $
# Makefile for MC WIN32
# $Log: Makefile.in,v $
#
#

ROOT=		..
MAKEFLAGS=

include		../Makefile.common

CINCLUDE=	$(ISWITCH). $(ISWITCH)../libw32 $(ISWITCH)../libregex
CEXTRA=		-DHAVE_CONFIG_H -DWIN32
CFLAGS=		$(CDEBUG) $(COPT) $(CWARN) $(CINCLUDE) $(CEXTRA)
LDLIBS=		$(LW)w32$(A) $(LW)regex$(A) $(XLDLIBS)
LDFLAGS=	$(LDOPT) $(LDEBUG)

ifndef MSVC
CFLAGS:=	$(subst /,\,$(CFLAGS))
LDFLAGS:=	$(subst /,\,$(LDFLAGS))
LDLIBS:=	$(subst /,\,$(LDLIBS))
endif

RMFLAGS=	-f
ARFLAGS=	-nologo

############################################################

D_OBJ:=			$(D_OBJ)/libmagic

TARGET=			$(D_BIN)/file$(E)
MAGICLIB=		$(LW)magic$(A)
MAGICSRC=		./file

VPATH=			./file

MSRC:=			$(shell ls $(MAGICSRC)/Magdir/*)

FILEOBJS=\
	$(D_OBJ)/file$(O)

LIBOBJS=\
	$(D_OBJ)/apprentice$(O)		\
	$(D_OBJ)/apptype$(O)		\
	$(D_OBJ)/ascmagic$(O)		\
	$(D_OBJ)/cdf$(O)		\
	$(D_OBJ)/cdf_time$(O)		\
	$(D_OBJ)/compress$(O)		\
	$(D_OBJ)/encoding$(O)		\
	$(D_OBJ)/fsmagic$(O)		\
	$(D_OBJ)/funcs$(O)		\
	$(D_OBJ)/getline$(O)		\
	$(D_OBJ)/is_tar$(O)		\
	$(D_OBJ)/magic$(O)		\
	$(D_OBJ)/print$(O)		\
	$(D_OBJ)/readcdf$(O)		\
	$(D_OBJ)/readelf$(O)		\
	$(D_OBJ)/softmagic$(O)		\
	\
	$(D_OBJ)/getopt_long$(O)	\
	$(D_OBJ)/asprintf$(O)		\
	$(D_OBJ)/vasprintf$(O)

OBJS=			$(FILEOBJS) $(LIBOBJS)

all:			object $(MAGICLIB) $(TARGET) magic.mgc

$(TARGET):		$(FILEOBJS) $(LIBOBJS)
		$(CC) $(LDFLAGS) $(subst /,\,$^) $(XSWITCH)$@ $(LDLIBS)

$(MAGICLIB):		$(LIBOBJS)
		$(AR) $(ARFLAGS) /OUT:$@ $^

magic:
			cat $(MAGICSRC)/Header $(MAGICSRC)/Localstuff $(MSRC) > $@

magic.mgc:		magic
		$(D_BIN)/file -C -m magic

object:			$(D_OBJ)/.created
$(D_OBJ)/.created:
		-@$(MKDIR) $(D_OBJ)
		-@echo "created, do no delete" > $@

clean:
		-$(RM) $(RMFLAGS) $(OBJS) $(subst $(O),.mbr,$(OBJS)) vc*.pdb >nul 2>&1
		-$(RM) $(RMFLAGS) $(TARGET) $(MAGICLIB) magic magic.mgc >nul 2>&1

$(D_OBJ)/%$(O)		: %.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

#end
