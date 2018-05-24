#include <stdbool.h>
#include <stdlib.h>              
#include <dbus/dbus-glib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dbus-1.0/dbus/dbus.h>
#include <math.h>
#include <gio/gio.h>
#include "alarm-stubs-generated.h"

#define ALARM_MESSAGE "WAKE UP!"
#define MSUINT 60
#define HUNIT 24
#define RESET 0

static gboolean alarmStatus = FALSE;
static guint8 alarm_sec = RESET;
static guint8 alarm_min = RESET;
static guint8 alarm_hour = RESET;
static gint8 delta_sec = RESET;
static gint8 delta_min = RESET;
static gint8 delta_hour = RESET;

dbusAlarmInterface *iAlarmInstance;
guint timerID;

gchar *opt_name = "com.training.dbus";

static gboolean on_timer_elapse()
{
	if (TRUE==alarmStatus)
	{
			dbus_alarm_interface_emit_ring_alarm (iAlarmInstance, ALARM_MESSAGE);
			g_print("ringAlarm signal sent on Dbus after timer elapse\n");
			alarmStatus = FALSE;
			timerID = RESET;
	}

	return FALSE;
}

static gboolean alarm_active(gint8 delta_h, gint8 delta_m, gint8 delta_s)
{
	guint64 time_to_alarm = RESET;
	gint8 alarm_h = alarm_hour;
	gint8 alarm_m = alarm_min;
	gint8 alarm_s = alarm_sec;
	struct tm *systime;
	time_t rawtime;
	rawtime = time(NULL);
	systime = localtime(&rawtime);

	if(alarm_s < (systime->tm_sec + delta_s))
	{
		if(MSUINT <= (systime->tm_sec + delta_s))
		{
			delta_s -= MSUINT;
			delta_m++;
		}
        alarm_s += MSUINT;
		alarm_m--;
	}
	if((alarm_m < (systime->tm_min + delta_m)) || (RESET > alarm_m))
	{
		if(MSUINT <= (systime->tm_min + delta_m))
		{
			delta_m -= MSUINT;
			delta_h++;
		}
		alarm_m += MSUINT;
		alarm_h--;
	}
	if((alarm_h < (systime->tm_hour + delta_h)) || (RESET > alarm_h))
	{
		if(HUNIT <= (systime->tm_hour + delta_h))
		{
			delta_h -= HUNIT;
		}
		alarm_h += HUNIT;
	}

	if(MSUINT <= (alarm_s - (systime->tm_sec + delta_s)))
	{
		alarm_s -= MSUINT;
	}
	if(MSUINT <= (alarm_m - (systime->tm_min + delta_m)))
	{
		alarm_m -= MSUINT;
	}
	if(HUNIT <= (alarm_h - (systime->tm_hour + delta_h)))
	{
		alarm_h -= HUNIT;
	}

	time_to_alarm = (((alarm_h - (systime->tm_hour + delta_h)) * MSUINT) * MSUINT) +
					((alarm_m - (systime->tm_min + delta_m)) * MSUINT) +
					(alarm_s - (systime->tm_sec + delta_s));		

	timerID = g_timeout_add_full (G_PRIORITY_HIGH, (time_to_alarm*1000), on_timer_elapse, NULL, NULL);

	g_print("Alarm enabled. Time till alarm: %d:%d:%d \n", alarm_h-(systime->tm_hour+delta_h), alarm_m-(systime->tm_min+delta_m), alarm_s-(systime->tm_sec+delta_s));
	g_print("Time till alarm: %lu \n", time_to_alarm);

	return TRUE;
}

static gboolean on_set_alarm_status(dbusAlarmInterface *ifc, GDBusMethodInvocation  *invocation, gboolean status)
{
	
	alarmStatus = status;
	if (TRUE == alarmStatus)
	{
		alarm_active(delta_hour,delta_min,delta_sec);	
	}
	else
	{
		if(RESET != timerID)
		{
			g_source_remove (timerID);
		}
		g_print("Alarm disabled \n");
	}

	g_print("Alarm status is %d, set in setAlarmStatus method. \n", alarmStatus);
	
	dbus_alarm_interface_complete_set_alarm_status (ifc, invocation);

	return TRUE;
}

static gboolean on_get_alarm_status(dbusAlarmInterface *ifc, GDBusMethodInvocation  *invocation)
{	
	g_print("Alarm status is %d \n", alarmStatus);

	dbus_alarm_interface_complete_get_alarm_status (ifc, invocation, alarmStatus);
	
	return TRUE;
}

static gboolean on_set_alarm_time(dbusAlarmInterface *ifc, GDBusMethodInvocation  *invocation, guint8 hour, guint8 min,  guint8 sec)
{
	alarm_hour = hour;
	alarm_min = min;
	alarm_sec = sec;
	
	if(HUNIT <= hour)
	{
		alarm_hour=23;	
	}

	if(MSUINT <= min)
	{
		alarm_min=59;	
	}
	
	if(MSUINT <= sec)
	{
		alarm_sec=59;	
	}
	
	g_print("Alarm time has been updated and is %u:%u:%u \n", alarm_hour, alarm_min, alarm_sec);
	
	if(TRUE==alarmStatus)
	{
		g_source_remove (timerID);
		alarm_active(delta_hour,delta_min,delta_sec);
	}

	dbus_alarm_interface_complete_set_alarm_time (ifc, invocation);

	return TRUE;
}
static gboolean on_set_time(dbusAlarmInterface *ifc, GDBusMethodInvocation  *invocation, guint8 hour, guint8 min, guint8 sec)
{
	struct tm *systime;
	time_t rawtime;
	rawtime = time(NULL);
	systime = localtime(&rawtime);

	if(HUNIT <= hour)
	{
		hour=23;	
	}

	if(MSUINT <= min)
	{
		min=59;	
	}
	
	if(MSUINT <= min)
	{
		sec=59;	
	}
	
	g_print("The old time is %d:%d:%d \n", systime->tm_hour, systime->tm_min, systime->tm_sec);
	g_print("The new time is %d:%d:%d \n", hour, min, sec);

	if(sec < systime->tm_sec)
	{
		sec += MSUINT;
		min--;
	}
	if(min < systime->tm_min)
	{
		min += MSUINT;
		hour--;
	}
	if(hour < systime->tm_hour)
	{
		hour += HUNIT;
	}



	delta_hour = hour-systime->tm_hour;
	delta_min = min-systime->tm_min;
	delta_sec = sec-systime->tm_sec;
	

	if(TRUE==alarmStatus)
	{
		g_source_remove (timerID);
		alarm_active(delta_hour,delta_min,delta_sec);
	}
	
	dbus_alarm_interface_complete_set_time (ifc, invocation);
	return TRUE;
}





static void on_bus_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	dbusAlarmObjectSkeleton *object;
    GDBusObjectManagerServer *manager = NULL;
	
    /* create a new server */
    manager = g_dbus_object_manager_server_new("/com/Training/dbus");
    object = dbus_alarm_object_skeleton_new("/com/Training/dbus/Interface");
    iAlarmInstance = dbus_alarm_interface_skeleton_new();

    dbus_alarm_object_skeleton_set_interface(object, iAlarmInstance);
    g_object_unref(iAlarmInstance);

    /* Register callbacks to the exposed DBus methods. */
    g_signal_connect (iAlarmInstance, "handle-set-alarm-status", G_CALLBACK (on_set_alarm_status), NULL);
	g_signal_connect (iAlarmInstance, "handle-get-alarm-status", G_CALLBACK (on_get_alarm_status), NULL);
	g_signal_connect (iAlarmInstance, "handle-set-alarm-time", G_CALLBACK (on_set_alarm_time), NULL);
	g_signal_connect (iAlarmInstance, "handle-set-time", G_CALLBACK (on_set_time), NULL);

    g_dbus_object_manager_server_export(manager, G_DBUS_OBJECT_SKELETON(object));
    g_object_unref(object);

    g_dbus_object_manager_server_set_connection(manager, connection);

    printf("Bus successfully acquired: %s \n", name);
}

static void on_name_acquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	printf("Acquired the name %s on the system bus\n", name);
}

static void on_name_lost(GDBusConnection *connection, const gchar *name, gpointer user_data)
{
	printf("Lost the name %s on the system bus\n", name);
}


int main()
{
	guint owner_id;
	GMainLoop *loop;


	owner_id = g_bus_own_name (G_BUS_TYPE_SYSTEM, opt_name, G_BUS_NAME_OWNER_FLAGS_NONE, on_bus_acquired, on_name_acquired, on_name_lost, NULL, NULL);

	loop = g_main_loop_new (NULL, FALSE);

	g_main_loop_run (loop);
	
	g_bus_unown_name (owner_id);

	return TRUE;
}
