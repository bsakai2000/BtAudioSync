// Big thanks to https://github.com/wware/stuff/blob/master/dbus-example/dbus-example.c
#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define MAC "08_DF_1F_9A_66_6C"
#define VOLUME_MAX 127

// Set the volume to (volume / VOLUME_MAX)%
void set_volume(uint16_t volume)
{
	char command[127];
	sprintf(command, "pactl set-sink-volume bluez_sink." MAC ".a2dp_sink %d%%", volume * 100 / VOLUME_MAX);
	system(command);
	return;
}

int main()
{
	// Setup error
	DBusError err;
	dbus_error_init(&err);

	// Connect to the system bus (MUST USE SYSTEM BUS)
	DBusConnection* conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if(dbus_error_is_set(&err))
	{
		fprintf(stderr, "Failed to connect to bus!\n%s\n", err.message);
		dbus_error_free(&err);
		return 1;
	}

	// Add the signal match for volume changes
	dbus_bus_add_match(conn, "type='signal',interface='org.freedesktop.DBus.Properties',path='/org/bluez/hci0/dev_" MAC "/fd1',arg0='org.bluez.MediaTransport1'", &err);
	if(dbus_error_is_set(&err))
	{
		fprintf(stderr, "Could not add signal match!\n%s\n", err.message);
		dbus_error_free(&err);
		return 1;
	}

	// Listen for signals
	while (1)
	{
		dbus_connection_read_write(conn, 0);
		DBusMessage* msg = dbus_connection_pop_message(conn);

		// If the message is null, try again
		if(msg == NULL)
		{
			usleep(100000);
			continue;
		}

		// Check that the signal matches what we expect
		if (dbus_message_is_signal(msg, "org.freedesktop.DBus.Properties", "PropertiesChanged")) {
			DBusMessageIter args;

			// Get args
			if (!dbus_message_iter_init(msg, &args))
			{
				fprintf(stderr, "Message Has No Parameters\n");
				continue;
			}

			// Move to array
			if(!dbus_message_iter_next(&args))
			{
				fprintf(stderr, "No second argument\n");
				continue;
			}

			// Check that it's an array
			if(dbus_message_iter_get_arg_type(&args) != DBUS_TYPE_ARRAY)
			{
				fprintf(stderr, "Type is not array\n");
				continue;
			}

			// Recurse into the array
			DBusMessageIter arrIter;
			dbus_message_iter_recurse(&args, &arrIter);

			// Check that the array contains a dict
			if(dbus_message_iter_get_arg_type(&arrIter) != DBUS_TYPE_DICT_ENTRY)
			{
				fprintf(stderr, "Array doesn't contain a dict\n");
				continue;
			}

			// Recurse into the dict
			DBusMessageIter dictIter;
			dbus_message_iter_recurse(&arrIter, &dictIter);

			// Check for the volume entry
			if(!dbus_message_iter_next(&dictIter))
			{
				fprintf(stderr, "No dictionary entry\n");
				continue;
			}

			// Recurse into the variant
			DBusMessageIter variantIter;
			dbus_message_iter_recurse(&dictIter, &variantIter);

			// Check for the volume value
			if(dbus_message_iter_get_arg_type(&variantIter) != DBUS_TYPE_UINT16)
			{
				fprintf(stderr, "No volume value\n");
				continue;
			}

			// Get the volume
			uint16_t volume;
			dbus_message_iter_get_basic(&variantIter, &volume);
			set_volume(volume);
		}

		// free the message
		dbus_message_unref(msg);
	}

	return 0;
}
