# -*- mode: mak; indent-tabs-mode: t; tab-width: 8 -*-
# Midnight command WIN32 builds 4.8.13, as at 21/03/15
#
#
#

ROOT=		.
MAKEFLAGS=

MAPFILE=	$(D_BIN)/mc.map
include		Makefile.common

PERL=		perl
BUSYBOX=	$(shell which busybox)
MCSOURCE=	../mcsrc
#MCSOURCE=	../mcmerge
GTKINSTALL=	../gtk

CINCLUDE=	$(ISWITCH).\
		$(ISWITCH)$(MCSOURCE) \
		$(ISWITCH)./libw32 \
		$(ISWITCH)./libregex \
		$(ISWITCH)./libmagic \
		$(ISWITCH)$(GTKINSTALL)/include \
		$(ISWITCH)$(GTKINSTALL)/include/glib-2.0
ifdef MSVC
CINCLUDE+=	$(ISWITCH)./libw32/msvc
endif
CEXTRA=		-DHAVE_CONFIG_H
CFLAGS=		$(CDEBUG) $(COPT) $(CWARN) $(CINCLUDE) $(CEXTRA)
LDLIBS=		$(GTKINSTALL)/lib/glib-2.0.lib $(GTKINSTALL)/lib/intl.lib $(XLDLIBS)
LDFLAGS=	$(LDOPT) $(LDEBUG) $(LDLIBS)

ifndef MSVC
CFLAGS:=	$(subst /,\,$(CFLAGS))
LDFLAGS:=	$(subst /,\,$(LDFLAGS))
endif

MAN2HLP=	@$(PERL) bin/man2hlp.pl
RMFLAGS=	-f
ARFLAGS=	-nologo

D_ETC=		$(D_BIN)/etc
D_PLUGIN=	$(D_BIN)/plugin
D_SHARE=	$(D_BIN)/share
D_DOC=		$(D_BIN)/doc
D_LOCALE=	$(D_BIN)/locale


#########################################################################################

MC_SRC=	\
	$(D_OBJ)/args$(O)			\
	$(D_OBJ)/clipboard$(O)			\
	$(D_OBJ)/cons.handler$(O)		\
	$(D_OBJ)/events_init$(O)		\
	$(D_OBJ)/execute$(O)			\
	$(D_OBJ)/help$(O)			\
	$(D_OBJ)/keybind-defaults$(O)		\
	$(D_OBJ)/learn$(O)			\
	$(D_OBJ)/main$(O)			\
	$(D_OBJ)/selcodepage$(O)		\
	$(D_OBJ)/setup$(O)			\
	$(D_OBJ)/textconf$(O)			\
	$(D_OBJ)/util$(O)
#	$(D_OBJ)/subshell$(O)			\
#	$(D_OBJ)/background$(O)

D_OBJFM=	$(D_OBJ)/filemanager

MC_FILEMANAGER=\
	$(D_OBJFM)/achown$(O)			\
	$(D_OBJFM)/boxes$(O)			\
	$(D_OBJFM)/chmod$(O)			\
	$(D_OBJFM)/chown$(O)			\
	$(D_OBJFM)/cmd$(O)			\
	$(D_OBJFM)/command$(O)			\
	$(D_OBJFM)/dir$(O)			\
	$(D_OBJFM)/ext$(O)			\
	$(D_OBJFM)/file$(O)			\
	$(D_OBJFM)/filegui$(O)			\
	$(D_OBJFM)/filenot$(O)			\
	$(D_OBJFM)/fileopctx$(O)		\
	$(D_OBJFM)/find$(O)			\
	$(D_OBJFM)/hotlist$(O)			\
	$(D_OBJFM)/info$(O)			\
	$(D_OBJFM)/layout$(O)			\
	$(D_OBJFM)/listmode$(O)			\
	$(D_OBJFM)/midnight$(O)			\
	$(D_OBJFM)/mountlist$(O)		\
	$(D_OBJFM)/panel$(O)			\
	$(D_OBJFM)/panelize$(O)			\
	$(D_OBJFM)/tree$(O)			\
	$(D_OBJFM)/treestore$(O)		\
	$(D_OBJFM)/usermenu$(O)			\
	\
	$(D_OBJFM)/drive$(O)

D_OBJED=	$(D_OBJ)/editor

MC_EDITOR=\
	$(D_OBJED)/bookmark$(O)			\
	$(D_OBJED)/choosesyntax$(O)		\
	$(D_OBJED)/edit$(O)			\
	$(D_OBJED)/editbuffer$(O)		\
	$(D_OBJED)/editcmd$(O)			\
	$(D_OBJED)/editcmd_dialogs$(O)		\
	$(D_OBJED)/editdraw$(O)			\
	$(D_OBJED)/editmenu$(O)			\
	$(D_OBJED)/editoptions$(O)		\
	$(D_OBJED)/editwidget$(O)		\
	$(D_OBJED)/etags$(O)			\
	$(D_OBJED)/format$(O)			\
	$(D_OBJED)/spell$(O)			\
	$(D_OBJED)/spell_dialogs$(O)		\
	$(D_OBJED)/syntax$(O)

#4.8.11	$(D_OBJED)/wordproc$(O)
#488	$(D_OBJED)/editkeys$(O)

D_OBJVW=	$(D_OBJ)/viewer

MC_VIEWER=\
	$(D_OBJVW)/viewer_actions_cmd$(O)	\
	$(D_OBJVW)/viewer_coord_cache$(O)	\
	$(D_OBJVW)/viewer_datasource$(O)	\
	$(D_OBJVW)/viewer_dialogs$(O)		\
	$(D_OBJVW)/viewer_display$(O)		\
	$(D_OBJVW)/viewer_growbuf$(O)		\
	$(D_OBJVW)/viewer_hex$(O)		\
	$(D_OBJVW)/viewer_lib$(O)		\
	$(D_OBJVW)/viewer_mcviewer$(O)		\
	$(D_OBJVW)/viewer_move$(O)		\
	$(D_OBJVW)/viewer_nroff$(O)		\
	$(D_OBJVW)/viewer_plain$(O)		\
	$(D_OBJVW)/viewer_search$(O)

MC_DIFFVIEWER=\
	$(D_OBJVW)/diffviewer_ydiff$(O)		\
	$(D_OBJVW)/diffviewer_search$(O)

MC_OBJS=\
	$(MC_SRC)				\
	$(MC_FILEMANAGER)			\
	$(MC_VIEWER)				\
	$(MC_EDITOR)				\
	$(MC_DIFFVIEWER)

#mcutil
#	lib
#		- $(D_OBJLIB)/utilunix$(O)
#
#		+ $(D_OBJLIB)/win32_utl$(O)
#		+ $(D_OBJLIB)/win32_glib$(O)
#	lib/event
#	lib/filehighlight
#	lib/mcconfig
#	lib/search
#	lib/skin
#	lib/strutil
#	lib/tty
#
#		- $(D_OBJLIB)/tty_color-ncurses$(O)
#		- $(D_OBJLIB)/tty_tty-ncurses$(O)
#		- $(D_OBJMC)/tty_tty-slang$(O)
#		- $(D_OBJLIB)/tty_key$(O)
#		- $(D_OBJLIB)/tty_win$(O)
#
#		+ $(D_OBJ)/win32_key$(O)
#		+ $(D_OBJ)/win32_tty$(O)
#		+ $(D_OBJ)/win32_win$(O)
#	lib/vfs
#	lib/widget
#
D_OBJMC=	$(D_OBJ)/libmc

MC_LIBMC=\
	$(D_OBJMC)/lib_charsets$(O)		\
	$(D_OBJMC)/lib_glibcompat$(O)		\
	$(D_OBJMC)/lib_global$(O)		\
	$(D_OBJMC)/lib_hook$(O) 		\
	$(D_OBJMC)/lib_keybind$(O)		\
	$(D_OBJMC)/lib_lock$(O) 		\
	$(D_OBJMC)/lib_logging$(O)		\
	$(D_OBJMC)/lib_serialize$(O)		\
	$(D_OBJMC)/lib_timefmt$(O)		\
	$(D_OBJMC)/lib_timer$(O)		\
	$(D_OBJMC)/lib_util$(O)			\
	\
	$(D_OBJMC)/event_event$(O)		\
	$(D_OBJMC)/event_manage$(O)		\
	$(D_OBJMC)/event_raise$(O)		\
	\
	$(D_OBJMC)/hilite_common$(O)		\
	$(D_OBJMC)/hilite_get-color$(O)		\
	$(D_OBJMC)/hilite_ini-file-read$(O)	\
	\
	$(D_OBJMC)/mcconfig_common$(O)		\
	$(D_OBJMC)/mcconfig_get$(O)		\
	$(D_OBJMC)/mcconfig_paths$(O)		\
	$(D_OBJMC)/mcconfig_set$(O)		\
	\
	$(D_OBJMC)/search_glob$(O)		\
	$(D_OBJMC)/search_hex$(O)		\
	$(D_OBJMC)/search_lib$(O)		\
	$(D_OBJMC)/search_normal$(O)		\
	$(D_OBJMC)/search_regex$(O)		\
	$(D_OBJMC)/search_search$(O)		\
	\
	$(D_OBJMC)/skin_colors$(O)		\
	$(D_OBJMC)/skin_colors-old$(O)		\
	$(D_OBJMC)/skin_common$(O)		\
	$(D_OBJMC)/skin_hc-skins$(O)		\
	$(D_OBJMC)/skin_ini-file$(O)		\
	$(D_OBJMC)/skin_lines$(O)		\
	\
	$(D_OBJMC)/strutil_strescape$(O)	\
	$(D_OBJMC)/strutil_strutil$(O)		\
	$(D_OBJMC)/strutil_strutil8bit$(O)	\
	$(D_OBJMC)/strutil_strutilascii$(O)	\
	$(D_OBJMC)/strutil_strutilutf8$(O)	\
	$(D_OBJMC)/strutil_strverscmp$(O)	\
	$(D_OBJMC)/strutil_replace$(O)		\
	$(D_OBJMC)/strutil_xstrtol$(O)		\
	\
	$(D_OBJMC)/tty_color$(O)		\
	$(D_OBJMC)/tty_color-internal$(O)	\
	$(D_OBJMC)/tty_color-slang$(O)		\
	$(D_OBJMC)/tty_keyxdef$(O)		\
	$(D_OBJMC)/tty_mouse$(O)		\
	$(D_OBJMC)/tty_tty$(O)			\
	$(D_OBJMC)/tty_x11conn$(O)		\
	\
	$(D_OBJMC)/win32_key$(O)		\
	$(D_OBJMC)/win32_tty$(O)		\
	$(D_OBJMC)/win32_win$(O)		\
	$(D_OBJMC)/win32_utl$(O)		\
	$(D_OBJMC)/win32_glib$(O)		\
	\
	$(D_OBJMC)/vfs_direntry$(O)		\
	$(D_OBJMC)/vfs_gc$(O)			\
	$(D_OBJMC)/vfs_interface$(O)		\
	$(D_OBJMC)/vfs_netutil$(O)		\
	$(D_OBJMC)/vfs_parse_ls_vga$(O)		\
	$(D_OBJMC)/vfs_path$(O)			\
	$(D_OBJMC)/vfs_utilvfs$(O)		\
	$(D_OBJMC)/vfs_vfs$(O)			\
	\
	$(D_OBJMC)/widget_button$(O)		\
	$(D_OBJMC)/widget_buttonbar$(O)		\
	$(D_OBJMC)/widget_check$(O)		\
	$(D_OBJMC)/widget_dialog$(O)		\
	$(D_OBJMC)/widget_dialog-switch$(O)	\
	$(D_OBJMC)/widget_gauge$(O)		\
	$(D_OBJMC)/widget_groupbox$(O)		\
	$(D_OBJMC)/widget_history$(O)		\
	$(D_OBJMC)/widget_hline$(O)		\
	$(D_OBJMC)/widget_input$(O)		\
	$(D_OBJMC)/widget_input_complete$(O)	\
	$(D_OBJMC)/widget_label$(O)		\
	$(D_OBJMC)/widget_listbox$(O)		\
	$(D_OBJMC)/widget_listbox-window$(O)	\
	$(D_OBJMC)/widget_menu$(O)		\
	$(D_OBJMC)/widget_quick$(O)		\
	$(D_OBJMC)/widget_radio$(O)		\
	$(D_OBJMC)/widget_widget-common$(O)	\
	$(D_OBJMC)/widget_wtools$(O)

#libvfs
#	vfs/
#	vfs/local
#	vfs/cpio
#	vfs/extfs
#	vfs/fish	TODO -- shell/pipe
#		- $(D_OBJVFS)/fish_fish$(O)
#	vfs/ftpfs
#	vfs/sfs
#	vfs/sftpfs	TODO -- libssh2
#		- $(D_OBJVFS)/sftpfs_config_parser$(O)
#		- $(D_OBJVFS)/sftpfs_connection$(O)
#		- $(D_OBJVFS)/sftpfs_dir$(O)
#		- $(D_OBJVFS)/sftpfs_file$(O)
#		- $(D_OBJVFS)/sftpfs_init$(O)
#		- $(D_OBJVFS)/sftpfs_internal$(O)
#		- $(D_OBJVFS)/sftpfs_vfs_class$(O)
#		- $(D_OBJVFS)/sftpfs_vfs_subclass$(O)
#	vfs/smbfs
#		- $(D_OBJVFS)/smbfs_smbfs$(O)
#	vfs/tar
#	vfs/undelfs	linux specific
#		- $(D_OBJVFS)/undelfs_undelfs$(O)
#
D_OBJVFS=	$(D_OBJ)/libvfs

MC_LIBVFS=\
	$(D_OBJVFS)/plugins_init$(O)		\
	$(D_OBJVFS)/local_local$(O)		\
	$(D_OBJVFS)/cpio_cpio$(O)		\
	$(D_OBJVFS)/extfs_extfs$(O)		\
	$(D_OBJVFS)/ftpfs_ftpfs$(O)		\
	$(D_OBJVFS)/sfs_sfs$(O)			\
	$(D_OBJVFS)/tar_tar$(O)

MC_RES=		$(D_OBJ)/mc.res

OBJS=		$(MC_OBJS)

#########################################################################################
# configuration

HINTS_SRC=		$(MCSOURCE)/doc/hints
MAN_SRC=		$(MCSOURCE)/doc/man
HLP_SRC=		$(MCSOURCE)/doc/hlp
EXTD_SRC=		$(MCSOURCE)/misc/ext.d
EXTFS_SRC=		$(MCSOURCE)/src/vfs/extfs/helpers
FISH_SRC=		$(MCSOURCE)/src/vfs/fish/helpers
MISC_SRC=		$(MCSOURCE)/misc
SKINS_SRC=		$(MCSOURCE)/misc/skins
SYNTAX_SRC=		$(MCSOURCE)/misc/syntax
MACROSDATA_SRC=		$(MCSOURCE)/misc/macros.d
DOC_SRC=		$(MCSOURCE)/doc
PO_SRC=			$(MCSOURCE)/po

WIN32_ETC_SRC=		./win32_etc
WIN32_DOC_SRC=		./doc

CFG_ETC=\
	$(D_ETC)/filehighlight.ini		\
	$(D_ETC)/mc.ext				\
	$(D_ETC)/mc.keymap			\
	$(D_ETC)/mc.keymap.emacs		\
	$(D_ETC)/mc.menu			\
	$(D_ETC)/mc.menu.sr			\
	$(D_ETC)/mcedit.menu			\
	$(D_ETC)/edit.indent.rc

CFG_WIN32_ETC:=		$(notdir $(wildcard $(WIN32_ETC_SRC)/*))

CFG_ETC_MAGIC=\
	$(D_ETC)/magic.mgc

CFG_SHARE=\
	$(D_SHARE)/mc.macros			\
	$(D_SHARE)/mc.charsets			\
	$(D_SHARE)/mc.lib

CFG_SHARE_HELP=\
	$(D_SHARE)/help/mc.hlp			\
	$(D_SHARE)/help/mc.hlp.es		\
	$(D_SHARE)/help/mc.hlp.hu		\
	$(D_SHARE)/help/mc.hlp.it		\
	$(D_SHARE)/help/mc.hlp.pl		\
	$(D_SHARE)/help/mc.hlp.ru		\
	$(D_SHARE)/help/mc.hlp.sr

CFG_SHARE_HINTS:=	$(addprefix $(D_SHARE)/hints/, \
				$(notdir $(wildcard $(HINTS_SRC)/mc.hint*)))

CFG_SHARE_SKINS:=	$(addprefix $(D_SHARE)/skins/, \
				$(notdir $(wildcard $(SKINS_SRC)/*.ini)))

CFG_SHARE_SYNTAX:=	$(addprefix $(D_SHARE)/syntax/, \
				$(notdir $(wildcard $(SYNTAX_SRC)/*ax)))

CFG_SHARE_MACROS_D:=	$(addprefix $(D_SHARE)/macros.d/, \
				$(notdir $(wildcard $(MACROSDATA_SRC)/*sh)))

CFG_PLUGIN:=		$(addprefix $(D_PLUGIN)/, \
				$(subst .sh.in,.sh,$(notdir $(wildcard $(EXTD_SRC)/*sh*))))

CFG_PLUGIN_EXTFS_D:=	$(addprefix $(D_PLUGIN)/extfs.d/, \
				$(subst .in,,$(filter-out Makefile.am sfs.ini,$(notdir $(wildcard $(EXTFS_SRC)/*)))))

CFG_PLUGIN_FISH:=	$(addprefix $(D_PLUGIN)/fish/, \
				$(filter-out Makefile.am,$(notdir $(wildcard $(FISH_SRC)/*))))

CFG_DOC=\
	$(D_DOC)/FAQ				\
	$(D_DOC)/COPYING			\
	$(D_DOC)/HACKING			\
	$(D_DOC)/MAINTAINERS			\
	$(D_DOC)/NEWS				\
	$(D_DOC)/README				\
	$(D_DOC)/TODO

CFG_DOC+=		$(addprefix $(D_DOC)/, \
				$(notdir $(wildcard $(WIN32_DOC_SRC)/*)))

CONFIGURATION=\
	$(CFG_ETC)				\
	$(CFG_ETC_MAGIC)			\
	$(CFG_SHARE)				\
	$(CFG_SHARE_HELP)			\
	$(CFG_SHARE_NEWS)			\
	$(CFG_SHARE_HINTS)			\
	$(CFG_SHARE_SKINS)			\
	$(CFG_SHARE_SYNTAX)			\
	$(CFG_SHARE_MACROS_D)			\
	$(CFG_PLUGIN)				\
	$(CFG_PLUGIN_EXTFS_D)			\
	$(CFG_PLUGIN_FISH)			\
	$(CFG_DOC)

#########################################################################################
# targets

BUILD_FILE=		BUILDNUMBER

TARGETS=\
	$(D_BIN)/mc$(E)				\
	$(D_BIN)/man2hlp.pl

IMPORT=\
	$(D_BIN)/busybox.exe			\
	$(D_BIN)/libglib-2.0-0.dll		\
	$(D_BIN)/intl.dll			\
	$(D_BIN)/zlib1.dll

LIBRARIES=\
	$(LW)mcutil$(A)				\
	$(LW)mcvfs$(A)				\
	$(LW)w32$(A)				\
	$(LW)regex$(A)				\
	$(LW)magic$(A)

DIRECTORIES:=\
	$(D_LIB)/.created			\
	$(D_OBJ)/.created			\
		$(D_OBJMC)/.created		\
		$(D_OBJVFS)/.created		\
		$(D_OBJFM)/.created		\
		$(D_OBJED)/.created		\
		$(D_OBJVW)/.created		\
	$(D_BIN)/.created			\
		$(D_DOC)/.created		\
		$(D_ETC)/.created		\
		$(D_LOCALE)/.created		\
		$(D_PLUGIN)/.created		\
		$(D_PLUGIN)/extfs.d/.created	\
		$(D_PLUGIN)/fish/.created	\
		$(D_SHARE)/.created		\
		$(D_SHARE)/help/.created	\
		$(D_SHARE)/hints/.created	\
		$(D_SHARE)/man/.created		\
		$(D_SHARE)/skins/.created	\
		$(D_SHARE)/syntax/.created	\
		$(D_SHARE)/macros.d/.created	\


#########################################################################################

.DEFAULT:
.PHONY:				all directories targets configuration

build:				directories targets configuration import

release:			new_buildnumber build package

directories:			$(DIRECTORIES)

targets:			$(TARGETS)

configuration:			$(CONFIGURATION) configuration-win32

.PHONY:				configuration-win32
configuration-win32:				# unconditional export
	@echo win32 specialised files ...
	@$(BUSYBOX) sh -c "\
		for x in $(CFG_WIN32_ETC); do \
			echo exporting $$x ...; \
			rm -f $(D_ETC)/$$x; \
			cp -f $(WIN32_ETC_SRC)/$$x $(D_ETC)/$$x; \
		done;\
		"

.PHONY:
import:				$(IMPORT)

$(D_BIN)/busybox.exe:		$(BUSYBOX)
	@echo importing $(BUSYBOX)
	@cp -f $^ $@

$(D_BIN)/%.dll:			$(GTKINSTALL)/bin/%.dll
	@echo importing $^
	@cp -f $^ $@

.PHONY:
package:
	$(ComSpec) /c start mc-inno-setup.iss

.PHONY:		clean
clean:
	-$(RM) $(RMFLAGS) $(TARGETS) $(LIBRARIES) $(CLEAN) $(XCLEAN) >nul 2>&1
	-$(RM) $(RMFLAGS) $(OBJS) $(MC_RES) $(subst $(O),.mbr,$(OBJS)) >nul 2>&1
	-$(RM) $(RMFLAGS) $(subst $(O),.mbr,$(MC_LIBMC)) $(LW)mcutil$(A) >nul 2>&1
	-$(RM) $(RMFLAGS) $(subst $(O),.mbr,$(MC_LIBVFS)) $(LW)mcvfs$(A) >nul 2>&1
	$(MAKE) -C libmagic clean
	$(MAKE) -C libregex clean
	$(MAKE) -C libw32 clean
	-$(RM) $(DIRECTORIES) >nul 2>&1
	-$(RMDIR) $(dir $(DIRECTORIES)) >nul 2>&1
	-$(RMDIR) $(D_OBJ) $(D_LIB) $(D_BIN) $(D_ETC) >nul 2>&1

#########################################################################################
# applications

$(D_BIN)/mc$(E):		$(LIBRARIES) $(MC_OBJS) $(MC_RES)
		$(CC) $(XSWITCH)$@ $(subst /,\,$^) $(LDFLAGS)

$(MC_RES):			config.h buildinfo.h

buildinfo.h:			$(BUILD_FILE)
	@$(BUSYBOX) sh -c "\
		echo // Auto-generated by Makefile >$@; \
		echo \#define BUILD_DATE \\\"$(BUILD_DATE)\\\" >>$@; \
		echo \#define BUILD_NUMBER \\\"$(BUILD_NUMBER)\\\" >>$@; \
		"

BUILD_DATE=			$(shell $(BUSYBOX) date +'%Y%m%d')
BUILD_NUMBER=			$(shell $(BUSYBOX) cat $(BUILD_FILE))

.PHONY:				new_buildnumber
new_buildnumber:
	-@chmod +w $(BUILD_FILE) >nul 2>&1

$(BUILD_FILE):			Makefile $(MC_OBJS) $(LIBRARIES)
	@echo incrementing build number ...
	-@$(BUSYBOX) sh -c "\
		if [ ! -f $(BUILD_FILE) ]; then echo 1 >$(BUILD_FILE); fi;\
		echo $$(($$(cat $(BUILD_FILE)) + 1)) >$(BUILD_FILE);\
		"
	-@chmod -w $(BUILD_FILE) >nul 2>&1

$(D_OBJ)/%.res: %.rc
		$(RC) -nologo -Fo$@ $<

$(D_BIN)/man2hlp.pl:		$(MCSOURCE)/src/man2hlp/man2hlp.in
		@echo importing $@ ..
		@$(CP) $< $@


#########################################################################################
# resources

# .. etc
$(D_ETC)/% :			$(MISC_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

$(D_ETC)/% :			$(MISC_SRC)/%.in			# mc.ext.in
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

# .. etc/magic
$(D_ETC)/magic.mgc:		libmagic/magic.mgc
	@echo importing $@ ..
	@$(CP) $< $@

# .. plugin
$(D_PLUGIN)/% :			$(EXTD_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

$(D_PLUGIN)/% :			$(EXTD_SRC)/%.in
	@echo converting $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

# .. plugin/extfs.d
$(D_PLUGIN)/extfs.d/% :		$(EXTFS_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

$(D_PLUGIN)/extfs.d/% :		$(EXTFS_SRC)/%.in
	@echo converting $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

# .. plugin/fish
$(D_PLUGIN)/fish/% :		$(FISH_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

# .. share
$(D_SHARE)/% :			$(MISC_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

# .. share/help
$(D_SHARE)/help/mc.hlp:		$(MAN_SRC)/mc.1.in $(HLP_SRC)/xnc.hlp
	@echo building $@ ..
	-$(MAN2HLP) $^ $@
	@busybox dos2unix $@

$(D_SHARE)/help/mc.hlp.%:	$(MAN_SRC)/%/mc.1.in $(HLP_SRC)/%/xnc.hlp
	@echo building $@ ..
	-$(MAN2HLP) $^ $@
	@busybox dos2unix $@

# .. share/hints
$(D_SHARE)/hints/% :		$(HINTS_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

# .. share/skins
$(D_SHARE)/skins/% :		$(SKINS_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

# .. share/syntax
$(D_SHARE)/syntax/% :		$(SYNTAX_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

# .. share/macro.d
$(D_SHARE)/macros.d/% :		$(MACROSDATA_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox dos2unix $@

# .. doc
$(D_DOC)/% :			$(WIN32_DOC_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox unix2dos $@

$(D_DOC)/% :			$(DOC_SRC)/%
	@echo importing $@ ..
	@$(CP) $^ $@
	@busybox unix2dos $@

#########################################################################################
# libraries

$(LW)mcutil$(A):		$(D_OBJMC)/.created $(MC_LIBMC)
		-@$(RM) -f $@ >nul 2>&1
		$(AR) $(ARFLAGS) /OUT:$@ $(subst /,\,$(MC_LIBMC))

$(LW)mcvfs$(A):			$(D_OBJVFS)/.created $(MC_LIBVFS)
		-@$(RM) -f $@ >nul 2>&1
		$(AR) $(ARFLAGS) /OUT:$@ $(subst /,\,$(MC_LIBVFS))

.PHONY:		$(LW)w32$(A)
$(LW)w32$(A):
		$(MAKE) -C libw32 $(MAKEFLAGS)

.PHONY:		$(LW)regex$(A)
$(LW)regex$(A):
		$(MAKE) -C libregex $(MAKEFLAGS)

.PHONY:		$(LW)magic$(A)
$(LW)magic$(A):
		$(MAKE) -C libmagic $(MAKEFLAGS)


#########################################################################################
# directories

$(DIRECTORIES) :
		-@$(MKDIR) $(@D)
		@echo Midnight Commander working files -- do not delete > $@


#########################################################################################
# source rules

$(D_OBJ)/%$(O) :		./%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJ)/%$(O) :		$(MCSOURCE)/src/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJFM)/%$(O) :		$(MCSOURCE)/src/filemanager/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJFM)/%$(O) :		%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVW)/viewer_%$(O) :	$(MCSOURCE)/src/viewer/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVW)/diffviewer_%$(O) :	$(MCSOURCE)/src/diffviewer/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJED)/%$(O) :		$(MCSOURCE)/src/editor/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

# .. libmcutil
$(D_OBJMC)/%$(O)	:	./%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/lib_%$(O)	:	$(MCSOURCE)/lib/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/event_%$(O)	:	$(MCSOURCE)/lib/event/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/hilite_%$(O) :	$(MCSOURCE)/lib/filehighlight/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/mcconfig_%$(O) :	$(MCSOURCE)/lib/mcconfig/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/search_%$(O) :	$(MCSOURCE)/lib/search/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/skin_%$(O) :		$(MCSOURCE)/lib/skin/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/strutil_%$(O) :	$(MCSOURCE)/lib/strutil/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/tty_%$(O) :		$(MCSOURCE)/lib/tty/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/vfs_%$(O) :		$(MCSOURCE)/lib/vfs/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJMC)/widget_%$(O) :	$(MCSOURCE)/lib/widget/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

# .. libmcvfs
$(D_OBJVFS)/%$(O) :		$(MCSOURCE)/src/vfs/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/cpio_%$(O) :	$(MCSOURCE)/src/vfs/cpio/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/extfs_%$(O) :	$(MCSOURCE)/src/vfs/extfs/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/fish_%$(O) :	$(MCSOURCE)/src/vfs/fish/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/ftpfs_%$(O) :	$(MCSOURCE)/src/vfs/ftpfs/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/local_%$(O) :	$(MCSOURCE)/src/vfs/local/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/sfs_%$(O) :		$(MCSOURCE)/src/vfs/sfs/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/sftpfs_%$(O) :	$(MCSOURCE)/src/vfs/sftpfs/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/smbfs_%$(O) :	$(MCSOURCE)/src/vfs/smbfs/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/tar_%$(O) :		$(MCSOURCE)/src/vfs/tar/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

$(D_OBJVFS)/undelfs_%$(O) :	$(MCSOURCE)/src/vfs/undelfs/%.c
		$(CC) $(CFLAGS) $(OSWITCH)$(subst /,\,$@) -c $(subst /,\,$<)

#end

