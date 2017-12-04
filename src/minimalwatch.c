#include <tizen.h>
#include "minimalwatch.h"
#include "data.h"
#include <device/battery.h>
#include "app_i18n.h"
#include "app.h"

typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *label;
} appdata_s;

#define TEXT_BUF_SIZE 256

/**
 * @brief Gets the battery charge percentage.
 */
int data_get_battery_percent(void)
{
	int ret = 0;
	int percentage = 0;

	ret = device_battery_get_percent(&percentage);
	if (ret != DEVICE_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "Failed to get battery information(%d)", ret);
		return 0;
	}

	return percentage;
}

static void
update_watch(appdata_s *ad, watch_time_h watch_time, int ambient)
{
	char watch_text[TEXT_BUF_SIZE];
	int hour24, minute, second, day, month, day_of_week, battery_percentage;

	if (watch_time == NULL)
		return;

	watch_time_get_month(watch_time, &month);
	watch_time_get_day(watch_time, &day);
	watch_time_get_day_of_week(watch_time, &day_of_week);

	watch_time_get_hour24(watch_time, &hour24);
	watch_time_get_minute(watch_time, &minute);
	watch_time_get_second(watch_time, &second);

	/* Calculate */
	char *month_word[12] = { ("Jan"), _("Feb"), _("Mar"), _("Apr"), _("May"), _("Jun"), _("Jul"), _("Aug"), _("Sep"), _("Oct"), _("Nov"), _("Dec") };
	char *month_name = month_word[month-1];

	battery_percentage = data_get_battery_percent();

	char *	week_word[7] = {_("Sun"), _("Mon"), _("Tue"), _("Wed"), _("Thu"), _("Fri"), _("Sat")};
	char *day_name = week_word[day_of_week-1];

	if (!ambient) {
		snprintf(watch_text, TEXT_BUF_SIZE, "<font=TizenSans font_size=34 color=#8e8e8e>"
				"<align=center>%s %d %s</align>"
				"</font><br/>"
				"<font=Tizen:style=Regular font_size=70 color=#8e8e8e>"
				"<align=center>%02d:%02d:%02d</align>"
				"</font><br/>"
				"<font=TizenSans font_size=30 color=#8e8e8e>"
				"<align=center>%d%%</align>"
				"</font>",
				day_name, day, month_name, hour24, minute, second, battery_percentage);
	} else {
		snprintf(watch_text, TEXT_BUF_SIZE, "<font=TizenSans font_size=34>"
						"<align=center>%s %d %s</align>"
						"</font><br/>"
						"<font=Tizen:style=Regular font_size=70>"
						"<align=center>%02d:%02d</align>"
						"</font><br/>"
						"<font=TizenSans font_size=30>"
						"<align=center>%d%%</align>"
						"</font>",
						day_name, day, month_name, hour24, minute, battery_percentage);
	}

	elm_object_text_set(ad->label, watch_text);
}

static void
create_base_gui(appdata_s *ad, int width, int height)
{
	int ret;
	watch_time_h watch_time = NULL;

	/* Window */
	ret = watch_app_get_elm_win(&ad->win);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get window. err = %d", ret);
		return;
	}

	evas_object_resize(ad->win, width, height);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Label*/
	ad->label = elm_label_add(ad->conform);
	evas_object_resize(ad->label, width, height);
	evas_object_move(ad->label, 0, height/4);
	evas_object_show(ad->label);

	ret = watch_time_get_current_time(&watch_time);
	if (ret != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get current time. err = %d", ret);

	update_watch(ad, watch_time, 0);
	watch_time_delete(watch_time);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);
}

static bool
app_create(int width, int height, void *data)
{
	/* Hook to take necessary actions before main event loop starts
		Initialize UI resources and application's data
		If this function returns true, the main loop of application starts
		If this function returns false, the application is terminated */
	appdata_s *ad = data;

	create_base_gui(ad, width, height);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	/* Release all resources. */
}

static void
app_time_tick(watch_time_h watch_time, void *data)
{
	/* Called at each second while your app is visible. Update watch UI. */
	appdata_s *ad = data;
	update_watch(ad, watch_time, 0);
}

static void
app_ambient_tick(watch_time_h watch_time, void *data)
{
	/* Called at each minute while the device is in ambient mode. Update watch UI. */
	appdata_s *ad = data;
	update_watch(ad, watch_time, 1);
}

static void
app_ambient_changed(bool ambient_mode, void *data)
{
	/* Update your watch UI to conform to the ambient mode */
}

static void
watch_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	app_event_get_language(event_info, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
watch_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	watch_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;
	event_callback.time_tick = app_time_tick;
	event_callback.ambient_tick = app_ambient_tick;
	event_callback.ambient_changed = app_ambient_changed;

	watch_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
		APP_EVENT_LANGUAGE_CHANGED, watch_app_lang_changed, &ad);
	watch_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
		APP_EVENT_REGION_FORMAT_CHANGED, watch_app_region_changed, &ad);

	ret = watch_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_main() is failed. err = %d", ret);
	}

	return ret;
}

