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

#ifndef __NAUTILUS_ACTIONS_API_NA_OBJECT_API_H__
#define __NAUTILUS_ACTIONS_API_NA_OBJECT_API_H__

/**
 * SECTION: na_object
 * @short_description: #NAObject public API.
 * @include: nautilus-actions/na-object-api.h
 *
 * We define here a common API which makes easier to write (and read)
 * the code ; all object functions are named na_object ; all arguments
 * are casted directly in the macro.
 */

#include "na-idata-factory.h"
#include "na-idata-factory-enum.h"
#include "na-iduplicable.h"
#include "na-object-action.h"
#include "na-object-profile.h"
#include "na-object-menu.h"

G_BEGIN_DECLS

/* NAIDuplicable
 */
#define na_object_duplicate( obj )						na_iduplicable_duplicate( NA_IDUPLICABLE( obj ))
#define na_object_check_status( obj )					na_object_object_check_status( NA_OBJECT( obj ))
#define na_object_get_origin( obj )						na_iduplicable_get_origin( NA_IDUPLICABLE( obj ))
#define na_object_is_valid( obj )						na_iduplicable_is_valid( NA_IDUPLICABLE( obj ))

/* NAObject
 */
#define na_object_dump( obj )							na_object_object_dump( NA_OBJECT( obj ))
#define na_object_dump_norec( obj )						na_object_object_dump_norec( NA_OBJECT( obj ))
#define na_object_dump_tree( tree )						na_object_object_dump_tree( tree )
#define na_object_get_hierarchy( obj )					na_object_object_get_hierarchy( NA_OBJECT( obj ))
#define na_object_unref( obj )							na_object_object_unref( NA_OBJECT( obj ))

/* NAObjectId
 */
#define na_object_get_id( obj )							(( gchar * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_ID ))
#define na_object_get_label( obj )						(( gchar * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_LABEL ))
#define na_object_get_parent( obj )						(( NAObjectItem * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_PARENT ))

#define na_object_set_id( obj, id )						na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_ID, ( const void * )( id ))
#define na_object_set_label( obj, label )				na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_LABEL, ( const void * )( label ))
#define na_object_set_parent( obj, parent )				na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_PARENT, ( const void * )( parent ))

#define na_object_sort_alpha_asc( a, b )				na_object_id_sort_alpha_asc( NA_OBJECT_ID( a ), NA_OBJECT_ID( b ))
#define na_object_sort_alpha_desc( a, b )				na_object_id_sort_alpha_desc( NA_OBJECT_ID( a ), NA_OBJECT_ID( b ))

/* NAObjectItem
 */
#define na_object_get_tooltip( obj )					(( gchar * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_TOOLTIP ))
#define na_object_get_icon( obj )						(( gchar * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_ICON ))
#define na_object_get_items( obj )						(( GList * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_SUBITEMS ))
#define na_object_get_items_slist( obj )				(( GSList * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_SUBITEMS_SLIST ))
#define na_object_is_enabled( obj )						(( gboolean ) GPOINTER_TO_UINT( na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_ENABLED )))
#define na_object_is_readonly( obj )					(( gboolean ) GPOINTER_TO_UINT( na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_READONLY )))
#define na_object_get_provider( obj )					na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_PROVIDER )
#define na_object_get_provider_data( obj )				na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_PROVIDER_DATA )

#define na_object_set_tooltip( obj, tooltip )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_TOOLTIP, ( const void * )( tooltip ))
#define na_object_set_icon( obj, icon )					na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_ICON, ( const void * )( icon ))
#define na_object_set_items( obj, list )				na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_SUBITEMS, ( const void * )( list ))
#define na_object_set_items_slist( obj, slist )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_SUBITEMS_SLIST, ( const void * )( slist ))
#define na_object_set_enabled( obj, enabled )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_ENABLED, ( const void * ) GUINT_TO_POINTER( enabled ))
#define na_object_set_readonly( obj, readonly )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_READONLY, ( const void * ) GUINT_TO_POINTER( readonly ))
#define na_object_set_provider( obj, provider )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_PROVIDER, ( const void * )( provider ))
#define na_object_set_provider_data( obj, data )		na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_PROVIDER_DATA, ( const void * )( data ))

#define na_object_get_item( obj, id )					na_object_item_get_item( NA_OBJECT_ITEM( obj ),( const gchar * )( id ))
#define na_object_append_item( obj, child )				na_object_item_append_item( NA_OBJECT_ITEM( obj ), NA_OBJECT_ID( child ))
#define na_object_build_items_slist( obj )				na_object_item_build_items_slist( NA_OBJECT_ITEM( obj ))
#define na_object_unref_items( tree )					na_object_item_unref_items( tree )

/* NAObjectAction
 */
#define na_object_get_version( obj )					(( gchar * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_VERSION ))
#define na_object_is_target_selection( obj )			(( gboolean ) GPOINTER_TO_UINT( na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_TARGET_SELECTION )))
#define na_object_is_target_background( obj )			(( gboolean ) GPOINTER_TO_UINT( na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_TARGET_BACKGROUND )))
#define na_object_is_target_toolbar( obj )				(( gboolean ) GPOINTER_TO_UINT( na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_TARGET_TOOLBAR )))
#define na_object_get_toolbar_label( obj )				(( gchar * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_TOOLBAR_LABEL ))

#define na_object_set_version( obj, version )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_VERSION, ( const void * )( version ))
#define na_object_set_target_selection( obj, target )	na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_TARGET_SELECTION, ( const void * ) GUINT_TO_POINTER( target ))
#define na_object_set_target_background( obj, target )	na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_TARGET_BACKGROUND, ( const void * ) GUINT_TO_POINTER( target ))
#define na_object_set_target_toolbar( obj, target )		na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_TARGET_TOOLBAR, ( const void * ) GUINT_TO_POINTER( target ))
#define na_object_set_toolbar_label( obj, label )		na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_TOOLBAR_LABEL, ( const void * )( label ))
#define na_object_set_toolbar_same_label( obj, same )	na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_TOOLBAR_SAME_LABEL, ( const void * ) GUINT_TO_POINTER( same ))

#define na_object_attach_profile( obj, profile )		na_object_action_attach_profile( NA_OBJECT_ACTION( obj ), NA_OBJECT_PROFILE( profile ))

/* NAObjectProfile
 */
#define na_object_get_path( obj )						(( gchar * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_PATH ))
#define na_object_get_parameters( obj )					(( gchar * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_PARAMETERS ))
#define na_object_get_basenames( obj )					(( GSList * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_BASENAMES ))
#define na_object_is_matchcase( obj )					(( gboolean ) GPOINTER_TO_UINT( na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_MATCHCASE )))
#define na_object_get_mimetypes( obj )					(( GSList * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_MIMETYPES ))
#define na_object_is_file( obj )						(( gboolean ) GPOINTER_TO_UINT( na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_ISFILE )))
#define na_object_is_dir( obj )							(( gboolean ) GPOINTER_TO_UINT( na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_ISDIR )))
#define na_object_is_multiple( obj )					(( gboolean ) GPOINTER_TO_UINT( na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_MULTIPLE )))
#define na_object_get_schemes( obj )					(( GSList * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_SCHEMES ))
#define na_object_get_folders( obj )					(( GSList * ) na_idata_factory_get( NA_IDATA_FACTORY( obj ), NADF_DATA_FOLDERS ))

#define na_object_set_path( obj, path )					na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_PATH, ( const void * )( path ))
#define na_object_set_parameters( obj, parms )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_PARAMETERS, ( const void * )( parms ))
#define na_object_set_basenames( obj, bnames )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_BASENAMES, ( const void * )( bnames ))
#define na_object_set_matchcase( obj, match )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_MATCHCASE, ( const void * ) GUINT_TO_POINTER( match ))
#define na_object_set_mimetypes( obj, types )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_MIMETYPES, ( const void * )( types ))
#define na_object_set_isfile( obj, isfile )				na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_ISFILE, ( const void * ) GUINT_TO_POINTER( isfile ))
#define na_object_set_isdir( obj, isdir )				na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_ISDIR, ( const void * ) GUINT_TO_POINTER( isdir ))
#define na_object_set_multiple( obj, multiple )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_MULTIPLE, ( const void * ) GUINT_TO_POINTER( multiple ))
#define na_object_set_schemes( obj, schemes )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_SCHEMES, ( const void * )( schemes ))
#define na_object_set_folders( obj, folders )			na_idata_factory_set( NA_IDATA_FACTORY( obj ), NADF_DATA_FOLDERS, ( const void * )( folders ))

G_END_DECLS

#endif /* __NAUTILUS_ACTIONS_API_NA_OBJECT_API_H__ */
