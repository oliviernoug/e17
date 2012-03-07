//Compile with:
//gcc -g `pkg-config --cflags --libs elementary` popup_example_02.c -o popup_example_02

#include <Elementary.h>
#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#else
# define __UNUSED__ __attribute__((unused))
# define PACKAGE_DATA_DIR "../../data"
#endif

static void _response_cb(void *data, Evas_Object *obj, void *event_info);

EAPI_MAIN int
elm_main(int argc __UNUSED__, char **argv __UNUSED__)
{
   Evas_Object *win, *bg, *popup, *btn1, *btn2, *btn3, *icon1;
   char buf[256];

   win = elm_win_add(NULL, "popup", ELM_WIN_BASIC);
   elm_win_title_set(win, "Popup");
   elm_win_autodel_set(win, EINA_TRUE);
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

   bg = elm_bg_add(win);
   elm_win_resize_object_add(win, bg);
   evas_object_show(bg);

   popup = elm_popup_add(win);

   // Setting popup content-text
   elm_object_text_set(popup, "This is the Content-Text for popup. The wrap"
            "for the content-text is character wrapping");
   // Setting the wrapping type to character wrapping
   elm_popup_content_text_wrap_type_set(popup, ELM_WRAP_CHAR);

   // Seting popup title-text
   elm_object_part_text_set(popup, "title,text", "Title");

   icon1 = elm_icon_add(popup);
   snprintf(buf, sizeof(buf), "%s/images/logo_small.png", PACKAGE_DATA_DIR);
   elm_icon_file_set(icon1, buf, NULL);
   //Setting popup title-icon
   elm_object_part_content_set(popup, "title,icon", icon1);

   // Creating the first action button
   btn1 = elm_button_add(popup);
   elm_object_text_set(btn1, "OK");

   // Setting the fist action button
   elm_object_part_content_set(popup, "button1", btn1);
   evas_object_smart_callback_add(btn1, "clicked", _response_cb, popup);

   // Creating the second action button
   btn2 = elm_button_add(popup);
   elm_object_text_set(btn2, "Cancel");

   // Setting the second action button
   elm_object_part_content_set(popup, "button2", btn2);
   evas_object_smart_callback_add(btn2, "clicked", _response_cb, popup);

   btn3 = elm_button_add(popup);
   elm_object_text_set(btn3, "Close");
   // Setting this action button
   elm_object_part_content_set(popup, "button3", btn3);
   // Setting the orientation of popup to Top
   elm_popup_orient_set(popup, ELM_POPUP_ORIENT_TOP);
   // Display the popup object
   evas_object_show(popup);

   evas_object_resize(win, 480, 800);
   evas_object_show(win);

   elm_run();

   return 0;
}
ELM_MAIN()
static void
_response_cb(void *data, Evas_Object *obj __UNUSED__,
             void *event_info __UNUSED__)
{
   evas_object_del(data);
}
