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

#include <glib/gi18n.h>

#include "na-exporter.h"
#include "na-export-format.h"

extern gboolean iexporter_initialized;
extern gboolean iexporter_finalized;

static GList       *exporter_get_formats( const NAIExporter *exporter );
static void         exporter_free_formats( const NAIExporter *exporter, GList * str_list );
static gchar       *exporter_get_name( const NAIExporter *exporter );
static NAIExporter *find_exporter_for_format( const NAPivot *pivot, GQuark format );

/*
 * na_exporter_get_formats:
 * @pivot: the #NAPivot instance.
 *
 * Returns: a list of #NAExportFormat objects, each of them addressing an
 * available export format, i.e. a format provided by a module which
 * implement the #NAIExporter interface.
 *
 * The returned list should later be na_exporter_free_formats() by the caller.
 */
GList *
na_exporter_get_formats( const NAPivot *pivot )
{
	GList *iexporters, *imod;
	GList *formats;
	GList *str_list, *is;
	NAExportFormat *format;

	g_return_val_if_fail( NA_IS_PIVOT( pivot ), NULL );

	formats = NULL;

	if( iexporter_initialized && !iexporter_finalized ){

		iexporters = na_pivot_get_providers( pivot, NA_IEXPORTER_TYPE );

		for( imod = iexporters ; imod ; imod = imod->next ){
			str_list = exporter_get_formats( NA_IEXPORTER( imod->data ));

			for( is = str_list ; is ; is = is->next ){
				format = na_export_format_new(( NAIExporterFormatExt * ) is->data );
				formats = g_list_prepend( formats, format );
			}

			exporter_free_formats( NA_IEXPORTER( imod->data ), str_list );
		}

		na_pivot_free_providers( iexporters );
	}

	return( formats );
}

/*
 * na_exporter_free_formats:
 * @formats: a list of available export formats, as returned by
 *  na_exporter_get_formats().
 *
 * Release the @formats #GList.
 */
void
na_exporter_free_formats( GList *formats )
{
	g_list_foreach( formats, ( GFunc ) g_object_unref, NULL );
	g_list_free( formats );
}

/*
 * na_exporter_to_buffer:
 * @pivot: the #NAPivot pivot for the running application.
 * @item: a #NAObjectItem-derived object.
 * @format: the #GQuark target format.
 * @messages: a pointer to a #GSList list of strings; the provider
 *  may append messages to this list, but shouldn't reinitialize it.
 *
 * Exports the specified @item in the required @format.
 *
 * Returns: the output buffer, as a newly allocated string which should
 * be g_free() by the caller, or %NULL if an error has been detected.
 */
gchar *
na_exporter_to_buffer( const NAPivot *pivot, const NAObjectItem *item, GQuark format, GSList **messages )
{
	static const gchar *thisfn = "na_exporter_to_buffer";
	gchar *buffer;
	NAIExporterBufferParms parms;
	NAIExporter *exporter;
	gchar *name;
	gchar *msg;

	g_return_val_if_fail( NA_IS_PIVOT( pivot ), NULL );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), NULL );

	buffer = NULL;

	if( iexporter_initialized && !iexporter_finalized ){

		g_debug( "%s: pivot=%p, item=%p (%s), format=%u (%s), messages=%p",
				thisfn,
				( void * ) pivot,
				( void * ) item, G_OBJECT_TYPE_NAME( item ),
				( guint ) format, g_quark_to_string( format ),
				( void * ) messages );

		exporter = find_exporter_for_format( pivot, format );
		g_debug( "%s: exporter=%p (%s)", thisfn, ( void * ) exporter, G_OBJECT_TYPE_NAME( exporter ));

		if( exporter ){
			parms.version = 1;
			parms.exported = ( NAObjectItem * ) item;
			parms.format = format;
			parms.buffer = NULL;
			parms.messages = messages ? *messages : NULL;

			if( NA_IEXPORTER_GET_INTERFACE( exporter )->to_buffer ){
				NA_IEXPORTER_GET_INTERFACE( exporter )->to_buffer( exporter, &parms );

				if( parms.buffer ){
					buffer = parms.buffer;
				}

			} else {
				name = exporter_get_name( exporter );
				msg = g_strdup_printf( _( "NAIExporter %s doesn't implement 'to_buffer' interface." ), name );
				*messages = g_slist_append( *messages, msg );
				g_free( name );
			}

		} else {
			msg = g_strdup_printf(
					_( "No NAIExporter implementation found for %s format." ), g_quark_to_string( format ));
			*messages = g_slist_append( *messages, msg );
		}
	}

	return( buffer );
}

/*
 * na_exporter_to_file:
 * @pivot: the #NAPivot pivot for the running application.
 * @item: a #NAObjectItem-derived object.
 * @folder_uri: the URI of the target folder.
 * @format: the #GQuark target format.
 * @messages: a pointer to a #GSList list of strings; the provider
 *  may append messages to this list, but shouldn't reinitialize it.
 *
 * Exports the specified @item to the target @uri in the required @format.
 *
 * Returns: the URI of the exported file, as a newly allocated string which
 * should be g_free() by the caller, or %NULL if an error has been detected.
 */
gchar *
na_exporter_to_file( const NAPivot *pivot, const NAObjectItem *item, const gchar *folder_uri, GQuark format, GSList **messages )
{
	static const gchar *thisfn = "na_exporter_to_file";
	gchar *export_uri;
	NAIExporterFileParms parms;
	NAIExporter *exporter;
	gchar *msg;
	gchar *name;

	g_return_val_if_fail( NA_IS_PIVOT( pivot ), NULL );
	g_return_val_if_fail( NA_IS_OBJECT_ITEM( item ), NULL );

	export_uri = NULL;

	if( iexporter_initialized && !iexporter_finalized ){

		g_debug( "%s: pivot=%p, item=%p (%s), folder_uri=%s, format=%u (%s), messages=%p",
				thisfn,
				( void * ) pivot,
				( void * ) item, G_OBJECT_TYPE_NAME( item ),
				folder_uri,
				( guint ) format, g_quark_to_string( format ),
				( void * ) messages );

		exporter = find_exporter_for_format( pivot, format );

		if( exporter ){
			parms.version = 1;
			parms.exported = ( NAObjectItem * ) item;
			parms.folder = ( gchar * ) folder_uri;
			parms.format = format;
			parms.basename = NULL;
			parms.messages = messages ? *messages : NULL;

			if( NA_IEXPORTER_GET_INTERFACE( exporter )->to_file ){
				NA_IEXPORTER_GET_INTERFACE( exporter )->to_file( exporter, &parms );

				if( parms.basename ){
					export_uri = g_strdup_printf( "%s%s%s", folder_uri, G_DIR_SEPARATOR_S, parms.basename );
				}

			} else {
				name = exporter_get_name( exporter );
				msg = g_strdup_printf( _( "NAIExporter %s doesn't implement 'to_file' interface." ), name );
				*messages = g_slist_append( *messages, msg );
				g_free( name );
			}

		} else {
			msg = g_strdup_printf(
					_( "No NAIExporter implementation found for %s format." ), g_quark_to_string( format ));
			*messages = g_slist_append( *messages, msg );
		}
	}

	return( export_uri );
}

/*
 * Returns a GList of NAIExporterFormatExt structures which describes
 * the export formats provided by the exporter
 * If the provider only implements the v1 interface, we dynamically
 * allocate a new structure and convert the v1 to the v2.
 */
static GList *
exporter_get_formats( const NAIExporter *exporter )
{
	GList *str_list;
	guint version;

	str_list = NULL;

	version = 1;
	if( NA_IEXPORTER_GET_INTERFACE( exporter )->get_version ){
		version = NA_IEXPORTER_GET_INTERFACE( exporter )->get_version( exporter );
	}

	if( NA_IEXPORTER_GET_INTERFACE( exporter )->get_formats ){
		if( version == 1 ){
#ifndef NA_DISABLE_DEPRECATED
			( const NAIExporterFormat * ) strv1 = NA_IEXPORTER_GET_INTERFACE( exporter )->get_formats( exporter );
			while( strv1->format ){
				NAIExporterFormatExt *strv2 = g_new0( NAIExporterFormatExt, 1 );
				strv2->version = 1;
				strv2->provider = exporter;
				strv2->format = strv1->format;
				strv2->label = strv1->label;
				strv2->description = strv1->description;
				strv2->pixbuf = NULL;
				str_list = g_list_prepend( str_list, strv2 );
				strv1++;
			}
#else
			;
#endif
		} else {
			str_list = NA_IEXPORTER_GET_INTERFACE( exporter )->get_formats( exporter );
		}
	}

	return( str_list );
}

/*
 * Free the list returned by exporter_get_formats() for this provider
 */
static void
exporter_free_formats( const NAIExporter *exporter, GList *str_list )
{
	guint version;

	version = 1;
	if( NA_IEXPORTER_GET_INTERFACE( exporter )->get_version ){
		version = NA_IEXPORTER_GET_INTERFACE( exporter )->get_version( exporter );
	}

	if( version == 1 ){
		g_list_foreach( str_list, ( GFunc ) g_free, NULL );
		g_list_free( str_list );

	} else {
		g_return_if_fail( !NA_IEXPORTER_GET_INTERFACE( exporter )->free_formats );
		NA_IEXPORTER_GET_INTERFACE( exporter )->free_formats( exporter, str_list );
	}
}

static gchar *
exporter_get_name( const NAIExporter *exporter )
{
	gchar *name;

	name = NULL;

	if( NA_IEXPORTER_GET_INTERFACE( exporter )->get_name ){
		name = NA_IEXPORTER_GET_INTERFACE( exporter )->get_name( exporter );
	}

	return( name );
}

static NAIExporter *
find_exporter_for_format( const NAPivot *pivot, GQuark format )
{
	NAIExporter *exporter;
	GList *formats, *ifmt;

	exporter = NULL;
	formats = na_exporter_get_formats( pivot );

	for( ifmt = formats ; ifmt && !exporter ; ifmt = ifmt->next ){

		if( na_export_format_get_quark( NA_EXPORT_FORMAT( ifmt->data )) == format ){
			exporter = na_export_format_get_exporter( NA_EXPORT_FORMAT( ifmt->data ));
		}
	}

	na_exporter_free_formats( formats );

	return( exporter );
}
