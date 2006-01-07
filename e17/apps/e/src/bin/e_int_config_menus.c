#include "e.h"

typedef struct _CFData CFData;

static void *_create_data(E_Config_Dialog *cfd);
static void _free_data(E_Config_Dialog *cfd, CFData *cfdata);
static int _basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata);
static int _advanced_apply_data(E_Config_Dialog *cfd, CFData *cfdata);
static Evas_Object *_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata);
static Evas_Object *_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata);

struct _CFData 
{
   int menu_eap_name_show;
   int menu_eap_generic_show;
   int menu_eap_comment_show;

   /* Advanced */
   double menus_scroll_speed;
   double menus_fast_mouse_move_threshhold;
   double menus_click_drag_timeout;
   int menu_autoscroll_margin;
   int menu_autoscroll_cursor_margin;
};

EAPI E_Config_Dialog *
e_int_config_menus(E_Container *con) 
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View v;
   
   v.create_cfdata = _create_data;
   v.free_cfdata = _free_data;
   v.basic.apply_cfdata = _basic_apply_data;
   v.basic.create_widgets = _basic_create_widgets;
   v.advanced.apply_cfdata = _advanced_apply_data;
   v.advanced.create_widgets = _advanced_create_widgets;
   
   cfd = e_config_dialog_new(con, _("Menu Settings"), NULL, 0, &v, NULL);
   return cfd;
}

static void 
_fill_data(CFData *cfdata) 
{
   cfdata->menu_eap_name_show = e_config->menu_eap_name_show;
   cfdata->menu_eap_generic_show = e_config->menu_eap_generic_show;
   cfdata->menu_eap_comment_show = e_config->menu_eap_comment_show;
   cfdata->menus_scroll_speed = e_config->menus_scroll_speed;
   cfdata->menus_fast_mouse_move_threshhold = e_config->menus_fast_mouse_move_threshhold;
   cfdata->menus_click_drag_timeout = e_config->menus_click_drag_timeout;
   cfdata->menu_autoscroll_margin = e_config->menu_autoscroll_margin;
   cfdata->menu_autoscroll_cursor_margin = e_config->menu_autoscroll_cursor_margin;
}

static void *
_create_data(E_Config_Dialog *cfd) 
{
   CFData *cfdata;
   
   cfdata = E_NEW(CFData, 1);
   _fill_data(cfdata);
   return cfdata;
}

static void 
_free_data(E_Config_Dialog *cfd, CFData *cfdata) 
{
   free(cfdata);
}

static int 
_basic_apply_data(E_Config_Dialog *cfd, CFData *cfdata) 
{
   e_border_button_bindings_ungrab_all();
   e_config->menu_eap_name_show = cfdata->menu_eap_name_show;
   e_config->menu_eap_generic_show = cfdata->menu_eap_generic_show;
   e_config->menu_eap_comment_show = cfdata->menu_eap_comment_show;
   e_border_button_bindings_grab_all();
   e_config_save_queue();
   return 1;
}

static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata) 
{
   Evas_Object *o, *of, *ob;
   
   _fill_data(cfdata);
   
   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Menu Settings"), 0);
   ob = e_widget_check_add(evas, _("Show Name In Menu"), &(cfdata->menu_eap_name_show));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Show Comment In Menu"), &(cfdata->menu_eap_comment_show));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Show Generic In Menu"), &(cfdata->menu_eap_generic_show));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   return o;
}

static int 
_advanced_apply_data(E_Config_Dialog *cfd, CFData *cfdata) 
{
   e_border_button_bindings_ungrab_all();
   e_config->menu_eap_name_show = cfdata->menu_eap_name_show;
   e_config->menu_eap_generic_show = cfdata->menu_eap_generic_show;
   e_config->menu_eap_comment_show = cfdata->menu_eap_comment_show;
   
   if (cfdata->menus_scroll_speed == 0) 
     {
	e_config->menus_scroll_speed = 1.0;
     }
   else 
     {
	e_config->menus_scroll_speed = cfdata->menus_scroll_speed;
     }
   
   if (cfdata->menus_fast_mouse_move_threshhold == 0) 
     {
	e_config->menus_fast_mouse_move_threshhold = 1.0;
     }
    else 
     {
	e_config->menus_fast_mouse_move_threshhold = cfdata->menus_fast_mouse_move_threshhold;
     }
   
   e_config->menus_click_drag_timeout = cfdata->menus_click_drag_timeout;
   e_config->menu_autoscroll_margin = cfdata->menu_autoscroll_margin;
   e_config->menu_autoscroll_cursor_margin = cfdata->menu_autoscroll_cursor_margin;
   e_border_button_bindings_grab_all();
   e_config_save_queue();
   return 1;
}

static Evas_Object *
_advanced_create_widgets(E_Config_Dialog *cfd, Evas *evas, CFData *cfdata) 
{
   Evas_Object *o, *of, *ob;
   
   _fill_data(cfdata);
   
   o = e_widget_list_add(evas, 0, 0);
   of = e_widget_framelist_add(evas, _("Menu Settings"), 0);
   ob = e_widget_check_add(evas, _("Show Name In Menu"), &(cfdata->menu_eap_name_show));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Show Comment In Menu"), &(cfdata->menu_eap_comment_show));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_check_add(evas, _("Show Generic In Menu"), &(cfdata->menu_eap_generic_show));
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);
   
   of = e_widget_framelist_add(evas, _("Autoscroll Settings"), 0);
   ob = e_widget_label_add(evas, _("Autoscroll Margin"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%2.0f pixels"), 0, 50.0, 1.0, 0, NULL, &(cfdata->menu_autoscroll_margin), 200);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Autoscroll Cursor Margin"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%2.0f pixels"), 0, 50.0, 1.0, 0, NULL, &(cfdata->menu_autoscroll_cursor_margin), 200);
   e_widget_framelist_object_append(of, ob);
   e_widget_list_object_append(o, of, 1, 1, 0.5);   

   of = e_widget_framelist_add(evas, _("Miscellaneous Options"), 0);
   ob = e_widget_label_add(evas, _("Menu Scroll Speed"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%5.0f pixels/sec"), 0.0, 20000.0, 100.0, 0, &(cfdata->menus_scroll_speed), NULL, 200);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Fast Mouse Move Threshhold"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%4.0f pixels/sec"), 0.0, 2000.0, 10.0, 0, &(cfdata->menus_fast_mouse_move_threshhold), NULL, 200);
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_label_add(evas, _("Click Drag Timeout"));
   e_widget_framelist_object_append(of, ob);
   ob = e_widget_slider_add(evas, 1, 0, _("%2.1f seconds"), 0.0, 10.0, 0.5, 0, &(cfdata->menus_click_drag_timeout), NULL, 200);
   e_widget_framelist_object_append(of, ob);
   
   e_widget_list_object_append(o, of, 1, 1, 0.5);   
   
 return o;
}
