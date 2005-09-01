#include <string.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libgnomevfs/gnome-vfs-file-info.h>
#include <libgnomevfs/gnome-vfs-ops.h>
#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-file-info.h>
#include <libnautilus-extension/nautilus-menu-provider.h>
#include "nautilus-actions.h"
#include "nautilus-actions-config.h"
#include "nautilus-actions-test.h"
#include "nautilus-actions-utils.h"

static GObjectClass *parent_class;
static GType actions_type = 0;

GType nautilus_actions_get_type (void) 
{
	return actions_type;
}

static void nautilus_actions_execute (NautilusMenuItem *item, ConfigAction *action)
{
	GList *files;
	GString *cmd;
	gchar* param;

	files = g_object_get_data (G_OBJECT (item), "files");

	cmd = g_string_new (action->command->path);

	param = nautilus_actions_utils_parse_parameter (action->command->parameters, files);

	if (param != NULL)
	{
		g_string_append_printf (cmd, " %s", param);
		g_free (param);
	}
	
	g_spawn_command_line_async (cmd->str, NULL);
	
	g_string_free (cmd, TRUE);
}

static NautilusMenuItem *nautilus_actions_create_menu_item (ConfigAction *action, GList *files)
{
	NautilusMenuItem *item;
	gchar* name;

	name = g_strdup_printf ("NautilusActions::%s", action->name);
	
	item = nautilus_menu_item_new (name, 
				action->menu_item->label, 
				action->menu_item->tooltip, 
				NULL);
	
	g_signal_connect_data (item, 
				"activate", 
				G_CALLBACK (nautilus_actions_execute),
				action, 
				(GClosureNotify)nautilus_actions_config_free_action, 
				0);
	
	g_object_set_data_full (G_OBJECT (item),
			"files",
			nautilus_file_info_list_copy (files),
			(GDestroyNotify) nautilus_file_info_list_free);
	
	g_free (name);
	
	return item;
}

static GList *nautilus_actions_get_file_items (NautilusMenuProvider *provider, GtkWidget *window, GList *files)
{
	GList *items = NULL;
	GList *config_actions = nautilus_actions_config_get_list ();
	GList *iter;
	NautilusMenuItem *item;

	for (iter = config_actions; iter; iter = iter->next)
	{
		/* Foreach configured action, check if we add a menu item */
		ConfigAction *action = nautilus_actions_config_action_dup ((ConfigAction*)iter->data);

		if (nautilus_actions_test_validate (action->test, files))
		{
			item = nautilus_actions_create_menu_item (action, files);
			items = g_list_append (items, item);
		}
	}
	
	nautilus_actions_config_free_list (config_actions);
	
	return items;
}

static void nautilus_actions_class_init (NautilusActions *actions)
{
}

static void nautilus_actions_instance_init (NautilusActions *actions)
{
}

static void nautilus_actions_menu_provider_iface_init (NautilusMenuProviderIface *iface)
{
	iface->get_file_items = nautilus_actions_get_file_items;
}

void nautilus_actions_register_type (GTypeModule *module)
{
	static const GTypeInfo info = {
		sizeof (NautilusActionsClass),
		(GBaseInitFunc) NULL,
		(GBaseFinalizeFunc) NULL,
		(GClassInitFunc) nautilus_actions_class_init,
		NULL,
		NULL,
		sizeof (NautilusActions),
		0,
		(GInstanceInitFunc)nautilus_actions_instance_init,
	};

	static const GInterfaceInfo menu_provider_iface_info = {
		(GInterfaceInitFunc) nautilus_actions_menu_provider_iface_init,
		NULL,
		NULL
	};

	actions_type = g_type_module_register_type (module,
								G_TYPE_OBJECT,
								"NautilusActions",
								&info, 0);

	g_type_module_add_interface (module,
								actions_type,
								NAUTILUS_TYPE_MENU_PROVIDER,
								&menu_provider_iface_info);
}

// vim:ts=3:sw=3:tw=1024
