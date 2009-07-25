#include "e.h"
#include "e_mod_main.h"

struct _E_Config_Dialog_Data
{
   int show_alert;
   int poll_interval;
   int alert_time;
   int alert_percent;
   int dismiss_alert;
   int alert_timeout;
   int force_mode; // 0 == auto, 1 == batget, 2 == hal
};

/* Protos */
static void          *_create_data(E_Config_Dialog *cfd);
static void          _free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object   *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int           _basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);
static Evas_Object   *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata);
static int           _advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata);

EAPI E_Config_Dialog *
e_int_config_battery_module(E_Container *con, const char *params __UNUSED__) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;
   char buf[4096];
   
   v = E_NEW(E_Config_Dialog_View, 1);
   
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;
   v->advanced.apply_cfdata = _advanced_apply_data;
   v->advanced.create_widgets = _advanced_create_widgets;

   snprintf(buf, sizeof(buf), "%s/e-module-battery.edj", e_module_dir_get(battery_config->module));
   cfd = e_config_dialog_new(con,
			     _("Battery Monitor Settings"), 
			     "E", "_e_mod_battery_config_dialog",
			     buf, 0, v, NULL);
   battery_config->config_dialog = cfd;
   return cfd;
}

static void
_fill_data(E_Config_Dialog_Data *cfdata) 
{
   if (!battery_config) return;
   cfdata->alert_time = battery_config->alert;
   cfdata->alert_percent = battery_config->alert_p;
   cfdata->poll_interval = battery_config->poll_interval;
   cfdata->alert_timeout = battery_config->alert_timeout;
   cfdata->force_mode = battery_config->force_mode;

   if (cfdata->alert_time > 0 || cfdata->alert_percent > 0) 
     cfdata->show_alert = 1;
   else 
     cfdata->show_alert = 0;
   
   if (cfdata->alert_timeout > 0)
     cfdata->dismiss_alert = 1;
   else
     cfdata->dismiss_alert = 0;
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   E_Config_Dialog_Data *cfdata;
   
   cfdata = E_NEW(E_Config_Dialog_Data, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void
_free_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if (!battery_config) return;
   battery_config->config_dialog = NULL;
   free(cfdata);
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *of, *ob;
      
   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Basic Settings"), 0);
   ob = e_widget_check_add(evas, _("Show alert when battery is low"), &(cfdata->show_alert));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   return o;
}

static int 
_basic_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if (!battery_config) return 0;

   if (cfdata->show_alert)
     { 
        battery_config->alert = cfdata->alert_time;
        battery_config->alert_p = cfdata->alert_percent;
     }
   else
     { 
        battery_config->alert = 0;
        battery_config->alert_p = 0;
     }

   if (cfdata->dismiss_alert)
     battery_config->alert_timeout = cfdata->alert_timeout;
   else
     battery_config->alert_timeout = 0;

   _battery_config_updated();
   e_config_save_queue();
   return 1;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, E_Config_Dialog_Data *cfdata) 
{
   Evas_Object *o, *ob, *of, *otb;
   E_Radio_Group *rg;

   otb = e_widget_toolbook_add(evas, 48 * e_scale, 48 * e_scale);
   
   /* Use Sliders for both cfg options */
   o = e_widget_table_add(evas, 0);
   
   ob = e_widget_label_add(evas, _("Check every:"));
   e_widget_table_object_append(o, ob, 0, 0, 1, 1, 1, 0, 1, 0);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f ticks"), 1, 256, 4, 0, NULL, &(cfdata->poll_interval), 180);
   e_widget_table_object_append(o, ob, 0, 1, 1, 1, 1, 0, 1, 0);
   e_widget_toolbook_page_append(otb, NULL, _("Polling"), o, 0, 0, 0, 0, 0.5, 0.0);

   o = e_widget_table_add(evas, 0);
   ob = e_widget_check_add(evas, _("Show alert on low battery"), &(cfdata->show_alert));
   e_widget_table_object_append(o, ob, 0, 0, 1, 1, 1, 1, 1, 0);   
   ob = e_widget_label_add(evas, _("Alert when at:"));
   e_widget_table_object_append(o, ob, 0, 1, 1, 1, 1, 0, 1, 1);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f min"), 0, 60, 1, 0, NULL, &(cfdata->alert_time), 180);
   e_widget_table_object_append(o, ob, 0, 2, 1, 1, 1, 0, 1, 0);
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f %%"), 0, 100, 1, 0, NULL, &(cfdata->alert_percent), 180);
   e_widget_table_object_append(o, ob, 0, 3, 1, 1, 1, 0, 1, 0);
   ob = e_widget_check_add(evas, _("Auto dismiss in..."), &(cfdata->dismiss_alert));
   e_widget_table_object_append(o, ob, 0, 4, 1, 1, 1, 1, 1, 0);   
   ob = e_widget_slider_add(evas, 1, 0, _("%1.0f sec"), 1, 300, 1, 0, NULL, &(cfdata->alert_timeout), 180);
   e_widget_table_object_append(o, ob, 0, 5, 1, 1, 1, 0, 1, 0);
   e_widget_toolbook_page_append(otb, NULL, _("Alert"), o, 0, 0, 0, 0, 0.5, 0.0);
   
   o = e_widget_list_add(evas, 0, 0);
   
   rg = e_widget_radio_group_new(&(cfdata->force_mode));
   ob = e_widget_radio_add(evas, _("Auto Detect"), 0, rg);
   e_widget_list_object_append(o, ob, 1, 0, 0.0);
   ob = e_widget_radio_add(evas, _("Internal"), 1, rg);
   e_widget_list_object_append(o, ob, 1, 0, 0.0);
   ob = e_widget_radio_add(evas, _("HAL"), 2, rg);
   e_widget_list_object_append(o, ob, 1, 0, 0.0);

   e_widget_toolbook_page_append(otb, NULL, _("Hardware"), o, 0, 0, 0, 0, 0.5, 0.0);

   e_widget_toolbook_page_show(otb, 0);
   return otb;
}

static int 
_advanced_apply_data(E_Config_Dialog *cfd, E_Config_Dialog_Data *cfdata) 
{
   if (!battery_config) return 0;

   battery_config->poll_interval = cfdata->poll_interval;

   if (cfdata->show_alert)
     { 
        battery_config->alert = cfdata->alert_time;
        battery_config->alert_p = cfdata->alert_percent;
     }
   else 
     {
        battery_config->alert = 0;
        battery_config->alert_p = 0;
     }

   if (cfdata->dismiss_alert)
     battery_config->alert_timeout = cfdata->alert_timeout;
   else
     battery_config->alert_timeout = 0;

   battery_config->force_mode = cfdata->force_mode;
   
   _battery_config_updated();
   e_config_save_queue();
   return 1;
}

