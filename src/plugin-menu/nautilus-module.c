/*
 * Nautilus-Actions
 * A Nautilus extension which offers configurable context menu actions.
 *
 * Copyright (C) 2005 The GNOME Foundation
 * Copyright (C) 2006, 2007, 2008 Frederic Ruaudel and others (see AUTHORS)
 * Copyright (C) 2009, 2010, 2011 Pierre Wieser and others (see AUTHORS)
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

#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include <libnautilus-extension/nautilus-extension-types.h>

#include <core/na-gconf-migration.h>

#include "nautilus-actions.h"

static void set_log_handler( void );
static void log_handler( const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data );

static GLogFunc st_default_log_func = NULL;

/*
 * A nautilus extension must implement three functions :
 *
 * - nautilus_module_initialize
 * - nautilus_module_list_types
 * - nautilus_module_shutdown
 *
 * The first two functions are called at nautilus startup.
 *
 * The prototypes for these functions are defined in nautilus-extension-types.h
 */

void
nautilus_module_initialize( GTypeModule *module )
{
	static const gchar *thisfn = "nautilus_module_initialize";

	syslog( LOG_USER | LOG_INFO, "[N-A] %s Menu Extender %s initializing...", PACKAGE_NAME, PACKAGE_VERSION );

	set_log_handler();

	g_debug( "%s: module=%p", thisfn, ( void * ) module );

	g_type_module_set_name( module, PACKAGE_STRING );

	/* pwi 2011-01-05
	 * run GConf migration tools before doing anything else
	 * above all before allocating a new NAPivot
	 */
	na_gconf_migration_run();

	nautilus_actions_register_type( module );
}

void
nautilus_module_list_types( const GType **types, int *num_types )
{
	static const gchar *thisfn = "nautilus_module_list_types";
	static GType type_list[1];

	g_debug( "%s: types=%p, num_types=%p", thisfn, ( void * ) types, ( void * ) num_types );

	type_list[0] = NAUTILUS_ACTIONS_TYPE;
	*types = type_list;
	*num_types = 1;

	/* this may let us some time to attach nautilus to the debugger :) */
	/*sleep( 60 ); */
}

void
nautilus_module_shutdown( void )
{
	static const gchar *thisfn = "nautilus_module_shutdown";

	g_debug( "%s", thisfn );

	/* remove the log handler
	 * almost useless as the process is nonetheless terminating at this time
	 * but this is the art of coding...
	 */
	if( st_default_log_func ){
		g_log_set_default_handler( st_default_log_func, NULL );
		st_default_log_func = NULL;
	}
}

/*
 * a log handler that we install when in development mode in order to be
 * able to log plugin runtime
 */
static void
set_log_handler( void )
{
	st_default_log_func = g_log_set_default_handler(( GLogFunc ) log_handler, NULL );
}

/*
 * we used to install a log handler for each and every log domain used
 * in Nautilus-Actions ; this led to a fastidious enumeration
 * instead we install a default log handler which will receive all
 * debug messages, i.e. not only from N-A, but also from other code
 * in the Nautilus process
 */
static void
log_handler( const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data )
{
	gchar *tmp;

	tmp = g_strdup( "" );
	if( log_domain && strlen( log_domain )){
		g_free( tmp );
		tmp = g_strdup_printf( "[%s] ", log_domain );
	}

#ifdef NA_MAINTAINER_MODE
	/*( *st_default_log_func )( log_domain, log_level, message, user_data );*/
	syslog( LOG_USER | LOG_DEBUG, "%s%s", tmp, message );
#else
	if( g_getenv( NAUTILUS_ACTIONS_DEBUG )){
		syslog( LOG_USER | LOG_DEBUG, "%s%s", tmp, message );
	}
#endif

	g_free( tmp );
}
