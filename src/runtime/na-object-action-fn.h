/*
 * Nautilus ObjectActions
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

#ifndef __NA_RUNTIME_OBJECT_ACTION_FN_H__
#define __NA_RUNTIME_OBJECT_ACTION_FN_H__

/**
 * SECTION: na_object_action
 * @short_description: #NAObjectAction public function declaration.
 * @include: runtime/na-object-action.h
 */

#include <glib/gi18n.h>

#include "na-object-action-class.h"
#include "na-object-profile-class.h"

G_BEGIN_DECLS

/* #NAObjectAction properties
 */
#define NAACTION_PROP_VERSION			"na-action-version"
#define NAACTION_PROP_READONLY			"na-action-read-only"
#define NAACTION_PROP_LAST_ALLOCATED	"na-action-last-allocated"

/* i18n: default label for a newly created action */
#define NA_OBJECT_ACTION_DEFAULT_LABEL	_( "New Nautilus action" )

NAObjectAction *na_object_action_new( void );

gchar          *na_object_action_get_version( const NAObjectAction *action );

void            na_object_action_set_version( NAObjectAction *action, const gchar *version );
void            na_object_action_set_readonly( NAObjectAction *action, gboolean readonly );

gchar          *na_object_action_get_new_profile_name( const NAObjectAction *action );
void            na_object_action_attach_profile( NAObjectAction *action, NAObjectProfile *profile );

G_END_DECLS

#endif /* __NA_RUNTIME_OBJECT_ACTION_FN_H__ */
