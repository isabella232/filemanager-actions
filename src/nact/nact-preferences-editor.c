/*
 * Nautilus Actions
 * A Nautilus extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
 * Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009 Pierre Wieser and others (see AUTHORS)
 *
 * This Program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This Program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this Library; see the file COPYING.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors:
 *   Frederic Ruaudel <grumz@grumz.net>
 *   Rodrigo Moya <rodrigo@gnome-db.org>
 *   Pierre Wieser <pwieser@trychlos.org>
 *   ... and many others (see AUTHORS)
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n.h>

#include <common/na-iprefs.h>

#include "base-iprefs.h"
#include "nact-application.h"
#include "nact-preferences-editor.h"

/* private class data
 */
struct NactPreferencesEditorClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NactPreferencesEditorPrivate {
	gboolean         dispose_has_run;
	BaseWindow      *parent;
};

static GObjectClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( NactPreferencesEditorClass *klass );
static void     iprefs_na_iface_init( NAIPrefsInterface *iface );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *dialog );
static void     instance_finalize( GObject *dialog );

static NactPreferencesEditor *preferences_editor_new( NactApplication *application );

static gchar   *base_get_iprefs_window_id( BaseWindow *window );
static gchar   *base_get_dialog_name( BaseWindow *window );
static void     on_base_initial_load_dialog( NactPreferencesEditor *editor, gpointer user_data );
static void     on_base_runtime_init_dialog( NactPreferencesEditor *editor, gpointer user_data );
/*static void     setup_buttons( NactPreferencesEditor *dialog, gboolean is_modified );
static void     on_modified_field( NactWindow *dialog );*/
static void     on_sort_alpha_toggled( GtkToggleButton *button, NactPreferencesEditor *editor );
static void     on_add_about_toggled( GtkToggleButton *button, NactPreferencesEditor *editor );
static void     on_cancel_clicked( GtkButton *button, NactPreferencesEditor *editor );
static void     on_ok_clicked( GtkButton *button, NactPreferencesEditor *editor );
static void     save_preferences( NactPreferencesEditor *editor );

static gboolean base_dialog_response( GtkDialog *dialog, gint code, BaseWindow *window );

GType
nact_preferences_editor_get_type( void )
{
	static GType dialog_type = 0;

	if( !dialog_type ){
		dialog_type = register_type();
	}

	return( dialog_type );
}

static GType
register_type( void )
{
	static const gchar *thisfn = "nact_preferences_editor_register_type";
	GType type;

	static GTypeInfo info = {
		sizeof( NactPreferencesEditorClass ),
		( GBaseInitFunc ) NULL,
		( GBaseFinalizeFunc ) NULL,
		( GClassInitFunc ) class_init,
		NULL,
		NULL,
		sizeof( NactPreferencesEditor ),
		0,
		( GInstanceInitFunc ) instance_init
	};

	static const GInterfaceInfo iprefs_na_iface_info = {
		( GInterfaceInitFunc ) iprefs_na_iface_init,
		NULL,
		NULL
	};

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_DIALOG_TYPE, "NactPreferencesEditor", &info, 0 );

	g_type_add_interface_static( type, NA_IPREFS_TYPE, &iprefs_na_iface_info );

	return( type );
}

static void
class_init( NactPreferencesEditorClass *klass )
{
	static const gchar *thisfn = "nact_preferences_editor_class_init";
	GObjectClass *object_class;
	BaseWindowClass *base_class;

	g_debug( "%s: klass=%p", thisfn, ( void * ) klass );

	st_parent_class = g_type_class_peek_parent( klass );

	object_class = G_OBJECT_CLASS( klass );
	object_class->dispose = instance_dispose;
	object_class->finalize = instance_finalize;

	klass->private = g_new0( NactPreferencesEditorClassPrivate, 1 );

	base_class = BASE_WINDOW_CLASS( klass );
	base_class->dialog_response = base_dialog_response;
	base_class->get_toplevel_name = base_get_dialog_name;
	base_class->get_iprefs_window_id = base_get_iprefs_window_id;
}

static void
iprefs_na_iface_init( NAIPrefsInterface *iface )
{
	static const gchar *thisfn = "nact_preferences_editor_iprefs_na_iface_init";

	g_debug( "%s: iface=%p", thisfn, ( void * ) iface );
}

static void
instance_init( GTypeInstance *instance, gpointer klass )
{
	static const gchar *thisfn = "nact_preferences_editor_instance_init";
	NactPreferencesEditor *self;

	g_debug( "%s: instance=%p, klass=%p", thisfn, ( void * ) instance, ( void * ) klass );
	g_return_if_fail( NACT_IS_PREFERENCES_EDITOR( instance ));
	self = NACT_PREFERENCES_EDITOR( instance );

	self->private = g_new0( NactPreferencesEditorPrivate, 1 );

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_WINDOW_SIGNAL_INITIAL_LOAD,
			G_CALLBACK( on_base_initial_load_dialog ));

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_WINDOW_SIGNAL_RUNTIME_INIT,
			G_CALLBACK( on_base_runtime_init_dialog ));

	self->private->dispose_has_run = FALSE;
}

static void
instance_dispose( GObject *dialog )
{
	static const gchar *thisfn = "nact_preferences_editor_instance_dispose";
	NactPreferencesEditor *self;

	g_debug( "%s: dialog=%p", thisfn, ( void * ) dialog );
	g_return_if_fail( NACT_IS_PREFERENCES_EDITOR( dialog ));
	self = NACT_PREFERENCES_EDITOR( dialog );

	if( !self->private->dispose_has_run ){

		self->private->dispose_has_run = TRUE;

		/* chain up to the parent class */
		if( G_OBJECT_CLASS( st_parent_class )->dispose ){
			G_OBJECT_CLASS( st_parent_class )->dispose( dialog );
		}
	}
}

static void
instance_finalize( GObject *dialog )
{
	static const gchar *thisfn = "nact_preferences_editor_instance_finalize";
	NactPreferencesEditor *self;

	g_debug( "%s: dialog=%p", thisfn, ( void * ) dialog );
	g_return_if_fail( NACT_IS_PREFERENCES_EDITOR( dialog ));
	self = NACT_PREFERENCES_EDITOR( dialog );

	g_free( self->private );

	/* chain call to parent class */
	if( G_OBJECT_CLASS( st_parent_class )->finalize ){
		G_OBJECT_CLASS( st_parent_class )->finalize( dialog );
	}
}

/*
 * Returns a newly allocated NactPreferencesEditor object.
 *
 * @parent: the BaseWindow parent of this dialog (usually, the main
 * toplevel window of the application).
 */
static NactPreferencesEditor *
preferences_editor_new( NactApplication *application )
{
	return( g_object_new( NACT_PREFERENCES_EDITOR_TYPE, BASE_WINDOW_PROP_APPLICATION, application, NULL ));
}

/**
 * nact_preferences_editor_run:
 * @parent: the BaseWindow parent of this dialog
 * (usually the NactMainWindow).
 *
 * Initializes and runs the dialog.
 */
void
nact_preferences_editor_run( BaseWindow *parent )
{
	static const gchar *thisfn = "nact_preferences_editor_run";
	NactApplication *application;
	NactPreferencesEditor *editor;

	g_debug( "%s: parent=%p", thisfn, ( void * ) parent );
	g_return_if_fail( BASE_IS_WINDOW( parent ));

	application = NACT_APPLICATION( base_window_get_application( parent ));
	g_assert( NACT_IS_APPLICATION( application ));

	editor = preferences_editor_new( application );
	editor->private->parent = parent;

	base_window_run( BASE_WINDOW( editor ));
}

static gchar *
base_get_iprefs_window_id( BaseWindow *window )
{
	return( g_strdup( "preferences-editor" ));
}

static gchar *
base_get_dialog_name( BaseWindow *window )
{
	return( g_strdup( "PreferencesDialog" ));
}

static void
on_base_initial_load_dialog( NactPreferencesEditor *editor, gpointer user_data )
{
	static const gchar *thisfn = "nact_preferences_editor_on_initial_load_dialog";
	/*GtkWindow *toplevel;
	GtkWindow *parent_toplevel;*/

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );
}

static void
on_base_runtime_init_dialog( NactPreferencesEditor *editor, gpointer user_data )
{
	static const gchar *thisfn = "nact_preferences_editor_on_runtime_init_dialog";
	gboolean sort_alpha, add_about_item;
	gboolean relabel;
	GtkWidget *button;

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );

	sort_alpha = na_iprefs_is_alphabetical_order( NA_IPREFS( editor ));
	button = base_window_get_widget( BASE_WINDOW( editor ), "SortAlphabeticalButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), sort_alpha );
	base_window_signal_connect_by_name(
			BASE_WINDOW( editor ),
			"SortAlphabeticalButton",
			"toggled",
			G_CALLBACK( on_sort_alpha_toggled ));

	add_about_item = na_iprefs_should_add_about_item( NA_IPREFS( editor ));
	button = base_window_get_widget( BASE_WINDOW( editor ), "AddAboutButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), add_about_item );
	base_window_signal_connect_by_name(
			BASE_WINDOW( editor ),
			"AddAboutButton",
			"toggled",
			G_CALLBACK( on_add_about_toggled ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( editor ),
			"CancelButton",
			"clicked",
			G_CALLBACK( on_cancel_clicked ));

	base_window_signal_connect_by_name(
			BASE_WINDOW( editor ),
			"OKButton",
			"clicked",
			G_CALLBACK( on_ok_clicked ));

	relabel = base_iprefs_get_bool( BASE_IPREFS( editor ), BASE_IPREFS_RELABEL_MENUS );
	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelMenuButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), relabel );

	relabel = base_iprefs_get_bool( BASE_IPREFS( editor ), BASE_IPREFS_RELABEL_ACTIONS );
	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelActionButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), relabel );

	relabel = base_iprefs_get_bool( BASE_IPREFS( editor ), BASE_IPREFS_RELABEL_PROFILES );
	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelProfileButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), relabel );
}

/*
 * rationale:
 * - while the preferences are not modified, only the cancel
 *   button is activated (showed as close)
 * - when at least one preference has been modified, we have a OK and a
 *   cancel buttons
 */
/*static void
setup_buttons( NactPreferencesEditor *editor, gboolean is_modified )
{
	GtkWidget *cancel_stock = gtk_button_new_from_stock( GTK_STOCK_CANCEL );
	GtkWidget *close_stock = gtk_button_new_from_stock( GTK_STOCK_CLOSE );

	GtkWidget *cancel_button = base_window_get_widget( BASE_WINDOW( editor ), "CancelButton" );
	gtk_button_set_label( GTK_BUTTON( cancel_button ), is_modified ? _( "_Cancel" ) : _( "_Close" ));
	gtk_button_set_image( GTK_BUTTON( cancel_button ), is_modified ? gtk_button_get_image( GTK_BUTTON( cancel_stock )) : gtk_button_get_image( GTK_BUTTON( close_stock )));

	gtk_widget_destroy( close_stock );
	gtk_widget_destroy( cancel_stock );

	GtkWidget *ok_button = base_window_get_widget( BASE_WINDOW( editor ), "OKButton" );
	gtk_widget_set_sensitive( ok_button, is_modified );
}*/

/*static void
on_modified_field( NactWindow *window )
{
	static const gchar *thisfn = "nact_preferences_editor_on_modified_field";

	g_assert( NACT_IS_PREFERENCES_EDITOR( window ));
	NactPreferencesEditor *editor = ( NACT_PREFERENCES_EDITOR( window ));

	gboolean is_modified = is_edited_modified( editor );
	setup_dialog_title( editor, is_modified );

	gboolean can_save = is_modified &&
		(( editor->private->show_profile_item && nact_iprofile_item_has_label( window )) ||
				nact_imenu_item_has_label( window ));

	setup_buttons( editor, can_save );
}*/

static void
on_sort_alpha_toggled( GtkToggleButton *button, NactPreferencesEditor *editor )
{
}

static void
on_add_about_toggled( GtkToggleButton *button, NactPreferencesEditor *editor )
{
}

static void
on_cancel_clicked( GtkButton *button, NactPreferencesEditor *editor )
{
	GtkWindow *toplevel = base_window_get_toplevel( BASE_WINDOW( editor ));

	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_CLOSE );
}

static void
on_ok_clicked( GtkButton *button, NactPreferencesEditor *editor )
{
	GtkWindow *toplevel = base_window_get_toplevel( BASE_WINDOW( editor ));

	gtk_dialog_response( GTK_DIALOG( toplevel ), GTK_RESPONSE_OK );
}

static void
save_preferences( NactPreferencesEditor *editor )
{
	GtkWidget *button;
	gboolean enabled;
	gboolean relabel;

	button = base_window_get_widget( BASE_WINDOW( editor ), "SortAlphabeticalButton" );
	enabled = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	na_iprefs_set_alphabetical_order( NA_IPREFS( editor ), enabled );

	button = base_window_get_widget( BASE_WINDOW( editor ), "AddAboutButton" );
	enabled = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	na_iprefs_set_add_about_item( NA_IPREFS( editor ), enabled );

	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelMenuButton" );
	relabel = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	base_iprefs_set_bool( BASE_IPREFS( editor ), BASE_IPREFS_RELABEL_MENUS, relabel );

	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelActionButton" );
	relabel = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	base_iprefs_set_bool( BASE_IPREFS( editor ), BASE_IPREFS_RELABEL_ACTIONS, relabel );

	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelProfileButton" );
	relabel = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	base_iprefs_set_bool( BASE_IPREFS( editor ), BASE_IPREFS_RELABEL_PROFILES, relabel );
}

static gboolean
base_dialog_response( GtkDialog *dialog, gint code, BaseWindow *window )
{
	static const gchar *thisfn = "nact_preferences_editor_on_dialog_response";
	NactPreferencesEditor *editor;

	g_debug( "%s: dialog=%p, code=%d, window=%p", thisfn, ( void * ) dialog, code, ( void * ) window );
	g_assert( NACT_IS_PREFERENCES_EDITOR( window ));
	editor = NACT_PREFERENCES_EDITOR( window );

	/*gboolean is_modified = is_edited_modified( editor );*/

	switch( code ){
		case GTK_RESPONSE_NONE:
		case GTK_RESPONSE_DELETE_EVENT:
		case GTK_RESPONSE_CLOSE:
		case GTK_RESPONSE_CANCEL:

			g_object_unref( editor );
			return( TRUE );
			break;

		case GTK_RESPONSE_OK:
			save_preferences( editor );
			g_object_unref( editor );
			return( TRUE );
			break;
	}

	return( FALSE );
}
