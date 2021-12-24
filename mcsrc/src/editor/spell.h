#ifndef MC__EDIT_ASPELL_H
#define MC__EDIT_ASPELL_H

#include "lib/global.h"         /* include <glib.h> */

/*** typedefs(not structures) and defined constants **********************************************/

/*** enums ***************************************************************************************/

/*** structures declarations (and typedefs of structures)*****************************************/

/*** global variables defined in .c file *********************************************************/

/*** declarations of public functions ************************************************************/

void aspell_init (void);
void aspell_clean (void);
gboolean aspell_check (const char *word, const int word_size);
unsigned int aspell_suggest (GArray * suggest, const char *word, const int word_size);
void aspell_array_clean (GArray * array);
unsigned int aspell_get_lang_list (GArray * lang_list);
const char *aspell_get_lang (void);
gboolean aspell_set_lang (const char *lang);
gboolean aspell_add_to_dict (const char *word, const int word_size);

int edit_suggest_current_word (WEdit * edit);
void edit_spellcheck_file (WEdit * edit);
void edit_set_spell_lang (void);

int spell_dialog_spell_suggest_show (WEdit * edit, const char *word, char **new_word,
                                     GArray * suggest);
char *spell_dialog_lang_list_show (GArray * languages);

/*** inline functions ****************************************************************************/

#endif /* MC__EDIT_ASPELL_H */
