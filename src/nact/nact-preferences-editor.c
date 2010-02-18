/*
 * Nautilus Actions
 * A Nautilus extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
 * Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009, 2010 Pierre Wieser and others (see AUTHORS)
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

#include <core/na-iprefs.h>

#include "nact-application.h"
#include "nact-iprefs.h"
#include "nact-export-format.h"
#include "nact-schemes-list.h"
#include "nact-providers-list.h"
#include "nact-preferences-editor.h"

/* private class data
 */
struct NactPreferencesEditorClassPrivate {
	void *empty;						/* so that gcc -pedantic is happy */
};

/* private instance data
 */
struct NactPreferencesEditorPrivate {
	gboolean dispose_has_run;
};

static GObjectClass *st_parent_class = NULL;

static GType    register_type( void );
static void     class_init( NactPreferencesEditorClass *klass );
static void     instance_init( GTypeInstance *instance, gpointer klass );
static void     instance_dispose( GObject *dialog );
static void     instance_finalize( GObject *dialog );

static NactPreferencesEditor *preferences_editor_new( BaseWindow *parent );

static gchar   *base_get_iprefs_window_id( const BaseWindow *window );
static gchar   *base_get_dialog_name( const BaseWindow *window );
static gchar   *base_get_ui_filename( const BaseWindow *dialog );
static void     on_base_initial_load_dialog( NactPreferencesEditor *editor, gpointer user_data );
static void     on_base_runtime_init_dialog( NactPreferencesEditor *editor, gpointer user_data );
static void     on_base_all_widgets_showed( NactPreferencesEditor *editor, gpointer user_data );
static void     on_esc_quit_toggled( GtkToggleButton *button, NactPreferencesEditor *editor );
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

	g_debug( "%s", thisfn );

	type = g_type_register_static( BASE_DIALOG_TYPE, "NactPreferencesEditor", &info, 0 );

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
	base_class->get_ui_filename = base_get_ui_filename;
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

	base_window_signal_connect(
			BASE_WINDOW( instance ),
			G_OBJECT( instance ),
			BASE_WINDOW_SIGNAL_ALL_WIDGETS_SHOWED,
			G_CALLBACK( on_base_all_widgets_showed));

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

		nact_schemes_list_dispose( BASE_WINDOW( self ));
		nact_providers_list_dispose( BASE_WINDOW( self ));

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
preferences_editor_new( BaseWindow *parent )
{
	return( g_object_new( NACT_PREFERENCES_EDITOR_TYPE, BASE_WINDOW_PROP_PARENT, parent, NULL ));
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
	NactPreferencesEditor *editor;

	g_debug( "%s: parent=%p", thisfn, ( void * ) parent );
	g_return_if_fail( BASE_IS_WINDOW( parent ));

	editor = preferences_editor_new( parent );

	base_window_run( BASE_WINDOW( editor ));
}

static gchar *
base_get_iprefs_window_id( const BaseWindow *window )
{
	return( g_strdup( "preferences-editor" ));
}

static gchar *
base_get_dialog_name( const BaseWindow *window )
{
	return( g_strdup( "PreferencesDialog" ));
}

static gchar *
base_get_ui_filename( const BaseWindow *dialog )
{
	return( g_strdup( PKGDATADIR "/nact-preferences.ui" ));
}

static void
on_base_initial_load_dialog( NactPreferencesEditor *editor, gpointer user_data )
{
	static const gchar *thisfn = "nact_preferences_editor_on_initial_load_dialog";
	NactApplication *application;
	NAUpdater *updater;
	GtkWidget *container;
	GtkTreeView *listview;

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );
	g_return_if_fail( NACT_IS_PREFERENCES_EDITOR( editor ));

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( editor )));
	updater = nact_application_get_updater( application );
	container = base_window_get_widget( BASE_WINDOW( editor ), "PreferencesExportFormatVBox" );
	nact_export_format_display( NA_PIVOT( updater ), container, EXPORT_FORMAT_DISPLAY_PREFERENCES );

	listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "SchemesTreeView" ));
	nact_schemes_list_create_model( listview, FALSE );

	listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ProvidersTreeView" ));
	nact_providers_list_create_model( listview );
}

static void
on_base_runtime_init_dialog( NactPreferencesEditor *editor, gpointer user_data )
{
	static const gchar *thisfn = "nact_preferences_editor_on_runtime_init_dialog";
	NactApplication *application;
	NAUpdater *updater;
	gint order_mode;
	gboolean add_about_item;
	gboolean create_root_menu;
	gboolean relabel;
	gint import_mode;
	GQuark export_format;
	GtkWidget *container;
	GtkWidget *button;
	gboolean esc_quit, esc_confirm;
	GtkTreeView *listview;

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( editor )));
	updater = nact_application_get_updater( application );

	/* first tab: runtime preferences
	 */
	order_mode = na_iprefs_get_order_mode( NA_IPREFS( updater ));
	switch( order_mode ){
		case IPREFS_ORDER_ALPHA_ASCENDING:
			button = base_window_get_widget( BASE_WINDOW( editor ), "OrderAlphaAscButton" );
			break;

		case IPREFS_ORDER_ALPHA_DESCENDING:
			button = base_window_get_widget( BASE_WINDOW( editor ), "OrderAlphaDescButton" );
			break;

		case IPREFS_ORDER_MANUAL:
		default:
			button = base_window_get_widget( BASE_WINDOW( editor ), "OrderManualButton" );
			break;
	}
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), TRUE );

	create_root_menu = na_iprefs_read_bool( NA_IPREFS( updater ), IPREFS_CREATE_ROOT_MENU, FALSE );
	button = base_window_get_widget( BASE_WINDOW( editor ), "CreateRootMenuButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), create_root_menu );

	add_about_item = na_iprefs_read_bool( NA_IPREFS( updater ), IPREFS_ADD_ABOUT_ITEM, TRUE );
	button = base_window_get_widget( BASE_WINDOW( editor ), "AddAboutButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), add_about_item );

	/* second tab: ui preferences
	 */
	relabel = na_iprefs_read_bool( NA_IPREFS( updater ), IPREFS_RELABEL_MENUS, FALSE );
	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelMenuButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), relabel );

	relabel = na_iprefs_read_bool( NA_IPREFS( updater ), IPREFS_RELABEL_ACTIONS, FALSE );
	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelActionButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), relabel );

	relabel = na_iprefs_read_bool( NA_IPREFS( updater ), IPREFS_RELABEL_PROFILES, FALSE );
	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelProfileButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), relabel );

	button = base_window_get_widget( BASE_WINDOW( editor ), "EscCloseButton" );

	base_window_signal_connect(
			BASE_WINDOW( editor ),
			G_OBJECT( button ),
			"toggled",
			G_CALLBACK( on_esc_quit_toggled ));

	esc_quit = na_iprefs_read_bool( NA_IPREFS( updater ), IPREFS_ASSIST_ESC_QUIT, TRUE );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), esc_quit );

	esc_confirm = na_iprefs_read_bool( NA_IPREFS( updater ), IPREFS_ASSIST_ESC_CONFIRM, TRUE );
	button = base_window_get_widget( BASE_WINDOW( editor ), "EscConfirmButton" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), esc_confirm );

	/* third tab: import tool
	 */
	import_mode = nact_iprefs_get_import_mode( BASE_WINDOW( editor ), IPREFS_IMPORT_ITEMS_IMPORT_MODE );
	switch( import_mode ){
		case IPREFS_IMPORT_ASK:
			button = base_window_get_widget( BASE_WINDOW( editor ), "PrefsAskButton" );
			break;

		case IPREFS_IMPORT_RENUMBER:
			button = base_window_get_widget( BASE_WINDOW( editor ), "PrefsRenumberButton" );
			break;

		case IPREFS_IMPORT_OVERRIDE:
			button = base_window_get_widget( BASE_WINDOW( editor ), "PrefsOverrideButton" );
			break;

		case IPREFS_IMPORT_NO_IMPORT:
		default:
			button = base_window_get_widget( BASE_WINDOW( editor ), "PrefsNoImportButton" );
			break;
	}
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), TRUE );

	/* fourth tab: export tool
	 */
	export_format = nact_iprefs_get_export_format( BASE_WINDOW( editor ), IPREFS_EXPORT_FORMAT );
	container = base_window_get_widget( BASE_WINDOW( editor ), "PreferencesExportFormatVBox" );
	nact_export_format_select( container, export_format );

	/* fifth tab: default schemes
	 */
	listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "SchemesTreeView" ));
	nact_schemes_list_init_view( listview, BASE_WINDOW( editor ));

	/* sixth tab: I/O providers priorities
	 */
	listview = GTK_TREE_VIEW( base_window_get_widget( BASE_WINDOW( editor ), "ProvidersTreeView" ));
	nact_providers_list_init_view( listview, BASE_WINDOW( editor ));

	/* dialog buttons
	 */
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
}

static void
on_base_all_widgets_showed( NactPreferencesEditor *editor, gpointer user_data )
{
	static const gchar *thisfn = "nact_preferences_editor_on_all_widgets_showed";
	GtkNotebook *notebook;

	g_debug( "%s: editor=%p, user_data=%p", thisfn, ( void * ) editor, ( void * ) user_data );
	notebook = GTK_NOTEBOOK( base_window_get_widget( BASE_WINDOW( editor ), "PreferencesNotebook" ));
	gtk_notebook_set_current_page( notebook, 0 );
}

static void
on_esc_quit_toggled( GtkToggleButton *button, NactPreferencesEditor *editor )
{
	gboolean is_active;
	GtkWidget *toggle;

	is_active = gtk_toggle_button_get_active( button );
	toggle = base_window_get_widget( BASE_WINDOW( editor ), "EscConfirmButton" );
	gtk_widget_set_sensitive( toggle, is_active );
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
	NactApplication *application;
	NAUpdater *updater;
	GtkWidget *button;
	gint order_mode;
	gboolean enabled;
	gboolean relabel;
	gint import_mode;
	GtkWidget *container;
	GQuark export_format;
	gboolean esc_quit, esc_confirm;

	application = NACT_APPLICATION( base_window_get_application( BASE_WINDOW( editor )));
	updater = nact_application_get_updater( application );

	/* first tab: runtime preferences
	 */
	order_mode = IPREFS_ORDER_ALPHA_ASCENDING;
	button = base_window_get_widget( BASE_WINDOW( editor ), "OrderAlphaAscButton" );
	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
		order_mode = IPREFS_ORDER_ALPHA_ASCENDING;
	} else {
		button = base_window_get_widget( BASE_WINDOW( editor ), "OrderAlphaDescButton" );
		if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
			order_mode = IPREFS_ORDER_ALPHA_DESCENDING;
		} else {
			button = base_window_get_widget( BASE_WINDOW( editor ), "OrderManualButton" );
			if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
				order_mode = IPREFS_ORDER_MANUAL;
			}
		}
	}
	na_iprefs_set_order_mode( NA_IPREFS( updater ), order_mode );

	button = base_window_get_widget( BASE_WINDOW( editor ), "CreateRootMenuButton" );
	enabled = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	nact_iprefs_write_bool( BASE_WINDOW( editor ), IPREFS_CREATE_ROOT_MENU, enabled );

	button = base_window_get_widget( BASE_WINDOW( editor ), "AddAboutButton" );
	enabled = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	nact_iprefs_write_bool( BASE_WINDOW( editor ), IPREFS_ADD_ABOUT_ITEM, enabled );

	/* second tab: runtime preferences
	 */
	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelMenuButton" );
	relabel = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	nact_iprefs_write_bool( BASE_WINDOW( editor ), IPREFS_RELABEL_MENUS, relabel );

	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelActionButton" );
	relabel = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	nact_iprefs_write_bool( BASE_WINDOW( editor ), IPREFS_RELABEL_ACTIONS, relabel );

	button = base_window_get_widget( BASE_WINDOW( editor ), "RelabelProfileButton" );
	relabel = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	nact_iprefs_write_bool( BASE_WINDOW( editor ), IPREFS_RELABEL_PROFILES, relabel );

	button = base_window_get_widget( BASE_WINDOW( editor ), "EscCloseButton" );
	esc_quit = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	nact_iprefs_write_bool( BASE_WINDOW( editor ), IPREFS_ASSIST_ESC_QUIT, esc_quit );

	button = base_window_get_widget( BASE_WINDOW( editor ), "EscConfirmButton" );
	esc_confirm = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ));
	nact_iprefs_write_bool( BASE_WINDOW( editor ), IPREFS_ASSIST_ESC_CONFIRM, esc_confirm );

	/* third tab: import tool
	 */
	import_mode = IPREFS_IMPORT_NO_IMPORT;
	button = base_window_get_widget( BASE_WINDOW( editor ), "PrefsRenumberButton" );
	if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
		import_mode = IPREFS_IMPORT_RENUMBER;
	} else {
		button = base_window_get_widget( BASE_WINDOW( editor ), "PrefsOverrideButton" );
		if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
			import_mode = IPREFS_IMPORT_OVERRIDE;
		} else {
			button = base_window_get_widget( BASE_WINDOW( editor ), "PrefsAskButton" );
			if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( button ))){
				import_mode = IPREFS_IMPORT_ASK;
			}
		}
	}
	nact_iprefs_set_import_mode( BASE_WINDOW( editor ), IPREFS_IMPORT_ITEMS_IMPORT_MODE, import_mode );

	/* fourth tab: export tool
	 */
	container = base_window_get_widget( BASE_WINDOW( editor ), "PreferencesExportFormatVBox" );
	export_format = nact_export_format_get_selected( container );
	nact_iprefs_set_export_format( BASE_WINDOW( editor ), IPREFS_EXPORT_FORMAT, export_format );

	/* fifth tab: list of default schemes
	 */
	nact_schemes_list_save_defaults( BASE_WINDOW( editor ));

	/* sixth tab: priorities of I/O providers
	 */
	nact_providers_list_save( BASE_WINDOW( editor ));
}

static gboolean
base_dialog_response( GtkDialog *dialog, gint code, BaseWindow *window )
{
	static const gchar *thisfn = "nact_preferences_editor_on_dialog_response";
	NactPreferencesEditor *editor;

	g_debug( "%s: dialog=%p, code=%d, window=%p", thisfn, ( void * ) dialog, code, ( void * ) window );
	g_assert( NACT_IS_PREFERENCES_EDITOR( window ));
	editor = NACT_PREFERENCES_EDITOR( window );

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
