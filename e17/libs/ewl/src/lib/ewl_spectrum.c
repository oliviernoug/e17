#include <Ewl.h>
#include "ewl_debug.h"
#include "ewl_macros.h"
#include "ewl_private.h"

/*
 * TODO
 *  - may need to optimize this to only redraw if we have a resize, or a
 *    type/mode change
 *  - implement a triangle selection mode
 *
 *  - Add a theme item for the cross hairs ...
 */

static void ewl_spectrum_hsv_from_rgb(Ewl_Spectrum *sp);
static void ewl_spectrum_rgb_from_hsv(Ewl_Spectrum *sp);
static void ewl_spectrum_draw(Ewl_Spectrum *sp);
static void ewl_spectrum_mouse_process(Ewl_Spectrum *sp, unsigned int x, 
							unsigned int y);

static void ewl_spectrum_hsv_to_rgb(double h, double s, double v,
				unsigned int *r, unsigned int *g, unsigned *b);
static void ewl_spectrum_color_coord_map(Ewl_Spectrum *sp, int x, int y, 
					int w, int h, unsigned int *r, 
					unsigned int *g, unsigned int *b);
static void ewl_spectrum_color_coord_map_vertical(Ewl_Spectrum *sp, int y, 
				int img_h, unsigned int *r, 
				unsigned int *g, unsigned int *b);
static void ewl_spectrum_color_coord_map_square(Ewl_Spectrum *sp, int x, 
				int y, int img_w, int img_h, unsigned int *r, 
				unsigned int *g, unsigned int *b);

/**
 * @return Returns a new Ewl_Spectrum widget or NULL on failure
 */
Ewl_Widget *
ewl_spectrum_new(void)
{
	Ewl_Widget *sp;

	DENTER_FUNCTION(DLEVEL_STABLE);

	sp = NEW(Ewl_Spectrum, 1);
	if (!sp)
	{
		DRETURN_PTR(NULL, DLEVEL_STABLE);
	}

	if (!ewl_spectrum_init(EWL_SPECTRUM(sp)))
	{
		ewl_widget_destroy(sp);
		sp = NULL;
	}

	DRETURN_PTR(sp, DLEVEL_STABLE);
}

/**
 * @param sp: The Ewl_Spectrum to init
 * @return Returns TRUEE on success or FALSE on failure
 */
int
ewl_spectrum_init(Ewl_Spectrum *sp)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("sp", sp, FALSE);

	if (!ewl_overlay_init(EWL_OVERLAY(sp)))
	{
		DRETURN_INT(FALSE, DLEVEL_STABLE);
	}

	ewl_widget_appearance_set(EWL_WIDGET(sp), "spectrum");
	ewl_widget_inherit(EWL_WIDGET(sp), "spectrum");
	ewl_object_fill_policy_set(EWL_OBJECT(sp), EWL_FLAG_FILL_FILL);
	ewl_container_callback_intercept(EWL_CONTAINER(sp),
					EWL_CALLBACK_MOUSE_MOVE);
	ewl_container_callback_intercept(EWL_CONTAINER(sp),
					EWL_CALLBACK_MOUSE_DOWN);
	ewl_container_callback_intercept(EWL_CONTAINER(sp),
					EWL_CALLBACK_MOUSE_UP);

	ewl_callback_append(EWL_WIDGET(sp), EWL_CALLBACK_MOUSE_DOWN, 
				ewl_spectrum_cb_mouse_down, NULL);
	ewl_callback_append(EWL_WIDGET(sp), EWL_CALLBACK_MOUSE_UP, 
				ewl_spectrum_cb_mouse_up, NULL);

	sp->type = EWL_SPECTRUM_TYPE_SQUARE;

	sp->canvas = ewl_image_new();
	ewl_container_child_append(EWL_CONTAINER(sp), sp->canvas);
	ewl_object_fill_policy_set(EWL_OBJECT(sp->canvas), EWL_FLAG_FILL_FILL);
	ewl_widget_internal_set(sp->canvas, TRUE);
	ewl_widget_show(sp->canvas);

	/* create the cross hairs to draw on the spectrum */
	sp->cross_hairs.horizontal = ewl_hseparator_new();
	ewl_container_child_append(EWL_CONTAINER(sp), sp->cross_hairs.horizontal);
	ewl_widget_internal_set(sp->cross_hairs.horizontal, TRUE);

	sp->cross_hairs.vertical = ewl_vseparator_new();
	ewl_container_child_append(EWL_CONTAINER(sp), sp->cross_hairs.vertical);
	ewl_widget_internal_set(sp->cross_hairs.vertical, TRUE);

	ewl_callback_append(EWL_WIDGET(sp), EWL_CALLBACK_CONFIGURE, 
			ewl_spectrum_cb_configure, NULL);

	ewl_spectrum_rgb_set(sp, 255, 255, 255);

	DRETURN_INT(TRUE, DLEVEL_STABLE);
}

/**
 * @param sp: The Ewl_Spectrum to set the type on
 * @param type: The type to set the spectrum too
 * @return Returns no value
 */
void
ewl_spectrum_type_set(Ewl_Spectrum *sp, Ewl_Spectrum_Type type)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	sp->type = type;
	ewl_widget_configure(EWL_WIDGET(sp));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param sp: The Ewl_Spectrum to get the type from
 * @return Returns the spectrum type
 */
Ewl_Spectrum_Type
ewl_spectrum_type_get(Ewl_Spectrum *sp)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("sp", sp, EWL_SPECTRUM_TYPE_SQUARE);
	DCHECK_TYPE_RET("sp", sp, "spectrum", EWL_SPECTRUM_TYPE_SQUARE);

	DRETURN_INT(sp->type, DLEVEL_STABLE);
}

/**
 * @param sp: The Ewl_Spectrum to set the mode on
 * @param mode: The mode to set the spectrum too
 * @return Returns no value.
 */
void
ewl_spectrum_mode_set(Ewl_Spectrum *sp, Ewl_Color_Mode mode)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	sp->mode = mode;
	ewl_widget_configure(EWL_WIDGET(sp));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param sp: The Ewl_Spectrum to get the mode from
 * @param Returns the mode of the spectrum */
Ewl_Color_Mode
ewl_spectrum_mode_get(Ewl_Spectrum *sp)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR_RET("sp", sp, EWL_COLOR_MODE_HSV_HUE);
	DCHECK_TYPE_RET("sp", sp, "spectrum", EWL_COLOR_MODE_HSV_HUE);

	DRETURN_INT(sp->mode, DLEVEL_STABLE);
}

/**
 * @param sp: The Ewl_Spectrum to set the colour into
 * @param r: The red value to set
 * @param g: The green value to set
 * @param b: The blue value to set
 * @return Returns no value
 */
void
ewl_spectrum_rgb_set(Ewl_Spectrum *sp, unsigned int r, 
			unsigned int g, unsigned int b)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	sp->rgb.r = r;
	sp->rgb.g = g;
	sp->rgb.b = b;

	if (sp->rgb.r > 255) sp->rgb.r = 255;
	if (sp->rgb.g > 255) sp->rgb.g = 255;
	if (sp->rgb.b > 255) sp->rgb.b = 255;

	ewl_spectrum_hsv_from_rgb(sp);
	ewl_widget_configure(EWL_WIDGET(sp));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param sp: The Ewl_Spectrum to get the rgb values from
 * @param r: Where to store the red value
 * @param g: Where to store the green value
 * @param b: Where to store the blue value
 * @return Returns no value.
 */
void
ewl_spectrum_rgb_get(Ewl_Spectrum *sp, unsigned int *r, 
			unsigned int *g, unsigned int *b)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	if (r) *r = sp->rgb.r;
	if (g) *g = sp->rgb.g;
	if (b) *b = sp->rgb.b;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param sp: The Ewl_Spectrum to set the hsv value into
 * @param h: The hue to set
 * @param s: The saturation to set
 * @param v: The value to set
 * @return Returns no value
 */
void
ewl_spectrum_hsv_set(Ewl_Spectrum *sp, double h, double s, double v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	sp->hsv.h = h;
	sp->hsv.s = s;
	sp->hsv.v = v;

	if (sp->hsv.h > 360) sp->hsv.h = 360.0;
	if (sp->hsv.h <= 0) sp->hsv.h = 360.0;

	if (sp->hsv.s > 1.0) sp->hsv.s = 1.0;
	if (sp->hsv.s < 0.0) sp->hsv.s = 0.0;

	if (sp->hsv.v > 1.0) sp->hsv.v = 1.0;
	if (sp->hsv.v < 0.0) sp->hsv.v = 0.0;

	ewl_spectrum_rgb_from_hsv(sp);
	ewl_widget_configure(EWL_WIDGET(sp));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

/**
 * @param sp: The Ewl_Spectrum to get the hsv values from
 * @param h: Where to place the hue
 * @param s: Where to place the saturation
 * @param v: Where to place the value
 * @return Returns no value.
 */
void
ewl_spectrum_hsv_get(Ewl_Spectrum *sp, double *h, double *s, double *v)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	if (h) *h = sp->hsv.h;
	if (s) *s = sp->hsv.s;
	if (v) *v = sp->hsv.v;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_spectrum_cb_configure(Ewl_Widget *w, void *ev __UNUSED__,
					void *data __UNUSED__)
{
	Ewl_Spectrum *sp;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "widget");

	sp = EWL_SPECTRUM(w);
	if (!REALIZED(sp))
	{
		DRETURN(DLEVEL_STABLE);
	}

	ewl_object_position_request(EWL_OBJECT(sp->canvas), 
					CURRENT_X(sp), CURRENT_Y(sp));
	ewl_object_size_request(EWL_OBJECT(sp->canvas), 
					CURRENT_W(sp), CURRENT_H(sp));

	ewl_spectrum_draw(sp);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_spectrum_cb_mouse_down(Ewl_Widget *w, void *ev, void *data)
{
	Ewl_Spectrum *sp;
	Ewl_Event_Mouse_Down *e;
	unsigned int x, y;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "spectrum");

	sp = EWL_SPECTRUM(w);
	e = ev;

	ewl_callback_append(w, EWL_CALLBACK_MOUSE_MOVE, 
					ewl_spectrum_cb_mouse_move, NULL);

	x = e->x - CURRENT_X(w);
	y = e->y - CURRENT_Y(w);

	if (x > (unsigned int)(CURRENT_X(sp->canvas) + CURRENT_W(sp->canvas)))
		x = (CURRENT_W(sp->canvas) - CURRENT_X(sp->canvas));
	if (y > (unsigned int)(CURRENT_Y(sp->canvas) + CURRENT_H(sp->canvas)))
		y = (CURRENT_H(sp->canvas) - CURRENT_Y(sp->canvas));

	ewl_spectrum_mouse_process(sp, x, y);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_spectrum_cb_mouse_move(Ewl_Widget *w, void *ev, void *data)
{
	Ewl_Spectrum *sp;
	Ewl_Event_Mouse_Move *e;
	int x, y;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "spectrum");

	sp = EWL_SPECTRUM(w);
	e = ev;

	x = e->x - CURRENT_X(sp);
	y = e->y - CURRENT_Y(sp);

	if (x > (CURRENT_X(sp->canvas) + CURRENT_W(sp->canvas)))
		x = (CURRENT_W(sp->canvas) - CURRENT_X(sp->canvas));
	if (y > (CURRENT_Y(sp->canvas) + CURRENT_H(sp->canvas)))
		y = (CURRENT_H(sp->canvas) - CURRENT_Y(sp->canvas));

	ewl_spectrum_mouse_process(sp, x, y);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

void
ewl_spectrum_cb_mouse_up(Ewl_Widget *w __UNUSED__, void *ev __UNUSED__,
							void *data)
{
	Ewl_Spectrum *sp;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("w", w);
	DCHECK_TYPE("w", w, "spectrum");

	sp = EWL_SPECTRUM(w);
	ewl_callback_del(w, EWL_CALLBACK_MOUSE_MOVE,
			 ewl_spectrum_cb_mouse_move);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_spectrum_mouse_process(Ewl_Spectrum *sp, unsigned int x, unsigned int y)
{
	Evas_Coord img_w, img_h;
	unsigned int r, g, b;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	evas_object_image_size_get(EWL_IMAGE(sp->canvas)->image, &img_w, &img_h);
	ewl_spectrum_color_coord_map(sp, x, y, img_w, img_h, &r, &g, &b);
	ewl_spectrum_rgb_set(sp, r, g, b);

	/* place the horizontal cross hair */
	ewl_object_position_request(EWL_OBJECT(sp->cross_hairs.horizontal), 
							CURRENT_X(sp), y);
	ewl_object_w_request(EWL_OBJECT(sp->cross_hairs.horizontal), 
							CURRENT_W(sp));

	if (!VISIBLE(sp->cross_hairs.horizontal))
		ewl_widget_show(sp->cross_hairs.horizontal);

	/* place the vertical cross hair if needed */
	if (sp->type == EWL_SPECTRUM_TYPE_SQUARE)
	{
		ewl_object_position_request(EWL_OBJECT(sp->cross_hairs.vertical), 
							x, CURRENT_Y(sp));
		ewl_object_h_request(EWL_OBJECT(sp->cross_hairs.vertical), 
							CURRENT_H(sp));

		if (!VISIBLE(sp->cross_hairs.vertical))
			ewl_widget_show(sp->cross_hairs.vertical);
	}

	ewl_callback_call(EWL_WIDGET(sp), EWL_CALLBACK_VALUE_CHANGED);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_spectrum_hsv_from_rgb(Ewl_Spectrum *sp)
{
	unsigned int min, max;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	min = MIN(sp->rgb.r, MIN(sp->rgb.g, sp->rgb.b));
	max = MAX(sp->rgb.r, MAX(sp->rgb.g, sp->rgb.b));

	sp->hsv.v = max / 255.0;
	sp->hsv.s = (max != 0) ? ((max - min) / (float)max) : 0.0;

	if (sp->hsv.s == 0.0) 
		sp->hsv.h = 0;
	else
	{
		unsigned int delta;

		delta = max - min;
		if ((unsigned int)sp->rgb.r == max)
			sp->hsv.h = (sp->rgb.g - sp->rgb.b) / (float)delta;
		else if ((unsigned int)sp->rgb.g == max)
			sp->hsv.h = 2.0 + ((sp->rgb.b - sp->rgb.r) / (float)delta);
		else if ((unsigned int)sp->rgb.b == max)
			sp->hsv.h = 4.0 + ((sp->rgb.r - sp->rgb.g) / (float)delta);

		sp->hsv.h *= 60.0;
		if (sp->hsv.h < 0.0) sp->hsv.h += 360.0;
	}

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}
static void
ewl_spectrum_rgb_from_hsv(Ewl_Spectrum *sp)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	ewl_spectrum_hsv_to_rgb(sp->hsv.h, sp->hsv.s, sp->hsv.v,
			&(sp->rgb.r), &(sp->rgb.g), &(sp->rgb.b));

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_spectrum_hsv_to_rgb(double h, double s, double v,
		unsigned int *r, unsigned int *g, unsigned *b)
{
	unsigned int r_tmp = 0, g_tmp = 0, b_tmp = 0;

	DENTER_FUNCTION(DLEVEL_STABLE);

	if (s == 0)
	{
		unsigned int i;

		i = v * 255.0;
		r_tmp = i;
		g_tmp = i;
		b_tmp = i;
	}
	else
	{
		double h_tmp, v_tmp, p, q, t, vs, vsf;
		int i;

		if (h == 360.0) h = 0.0;

		h_tmp = h / 60.0;
		i = h_tmp;

		vs = v * s;
		vsf = vs * (h_tmp - i);

		p = 255.0 * (v - vs);
		q = 255.0 * (v - vsf);
		t = 255.0 * (v - vs + vsf);

		v_tmp = v * 255.0;

		switch(i)
		{
			case 0:
				r_tmp = v_tmp;
				g_tmp = t;
				b_tmp = p;
				break;

			case 1:
				r_tmp = q;
				g_tmp = v_tmp;
				b_tmp = p;
				break;

			case 2:
				r_tmp = p;
				g_tmp = v_tmp;
				b_tmp = t;
				break;

			case 3:
				r_tmp = p;
				g_tmp = q;
				b_tmp = v_tmp;
				break;

			case 4:
				r_tmp = t;
				g_tmp = p;
				b_tmp = v_tmp;
				break;

			case 5:
				r_tmp = v_tmp;
				g_tmp = p;
				b_tmp = q;
				break;
		}	
	}

	if (r) *r = r_tmp;
	if (g) *g = g_tmp;
	if (b) *b = b_tmp;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_spectrum_draw(Ewl_Spectrum *sp)
{
	Evas_Object *img;
	Evas_Coord img_w, img_h;
	int *data;
	int i, k;
	unsigned int r, g, b, a;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);
	DCHECK_TYPE("sp", sp, "spectrum");

	img = EWL_IMAGE(sp->canvas)->image;
	evas_object_image_size_set(img, CURRENT_W(sp), CURRENT_H(sp));
	evas_object_image_size_get(img, &img_w, &img_h);

	data = evas_object_image_data_get(img, 1);
	if (!data)
	{
		DRETURN(DLEVEL_STABLE);
	}

	a = 255 << 24;
	for (i = 0; i < img_h; i++)
	{
		for (k = 0; k < img_w; k++)
		{
			ewl_spectrum_color_coord_map(sp, k, i, img_w, img_h, &r, &g, &b);
			data[(i * img_w) + k] = a | (r << 16) | (g << 8) | b;
		}
	}

	evas_object_image_data_set(img, data);
	evas_object_image_data_update_add(img, 0, 0, img_w, img_h);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_spectrum_color_coord_map(Ewl_Spectrum *sp, int x, int y, int img_w, int img_h,
				unsigned int *r, unsigned int *g, unsigned int *b)
{
	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);

	if (sp->type == EWL_SPECTRUM_TYPE_VERTICAL)
		ewl_spectrum_color_coord_map_vertical(sp, y, img_h, r, g, b);
	else
		ewl_spectrum_color_coord_map_square(sp, x, y, img_w, 
							img_h, r, g, b);

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_spectrum_color_coord_map_vertical(Ewl_Spectrum *sp, int y, int img_h,
					unsigned int *r, unsigned int *g, unsigned int *b)
{
	unsigned int r_tmp, g_tmp, b_tmp;
	double h, s, v;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);

	r_tmp = 0;
	g_tmp = 0;
	b_tmp = 0;

	h = 360.0;
	s = 0.0;
	v = 1.0;
	switch (sp->mode)
	{
		case EWL_COLOR_MODE_RGB_RED:
			r_tmp = sp->rgb.r - ((sp->rgb.r * y) / img_h);
			break;

		case EWL_COLOR_MODE_RGB_GREEN:
			g_tmp = sp->rgb.g - ((sp->rgb.g * y) / img_h);
			break;
		case EWL_COLOR_MODE_RGB_BLUE:
			b_tmp = sp->rgb.b - ((sp->rgb.b * y) / img_h);
			break;

		case EWL_COLOR_MODE_HSV_HUE:
			h = (360.0 * y) / (double)img_h;
			s = 1.0;
			ewl_spectrum_hsv_to_rgb(h, s, v, &r_tmp, &g_tmp, &b_tmp);
			break;

		case EWL_COLOR_MODE_HSV_SATURATION:
			s = 1.0 - (y / (double)img_h);
			ewl_spectrum_hsv_to_rgb(h, s, v, &r_tmp, &g_tmp, &b_tmp);
			break;

		case EWL_COLOR_MODE_HSV_VALUE:
			v = 1.0 - (y / (double)img_h);
			ewl_spectrum_hsv_to_rgb(h, s, v, &r_tmp, &g_tmp, &b_tmp);
			break;

		default:
			break;
	}

	if (r) *r = r_tmp;
	if (g) *g = g_tmp;
	if (b) *b = b_tmp;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}

static void
ewl_spectrum_color_coord_map_square(Ewl_Spectrum *sp, int x, int y, int img_w, int img_h,
					unsigned int *r, unsigned int *g, unsigned int *b)
{
	unsigned int r_tmp, g_tmp, b_tmp;
	double h, s, v;

	DENTER_FUNCTION(DLEVEL_STABLE);
	DCHECK_PARAM_PTR("sp", sp);

	switch (sp->mode)
	{
		case EWL_COLOR_MODE_RGB_RED:
			r_tmp = sp->rgb.r;
			g_tmp = (1.0 - (y / (double)img_h)) * 255.0;
			b_tmp = (1.0 - (x / (double)img_w)) * 255.0;
			break;

		case EWL_COLOR_MODE_RGB_GREEN:
			r_tmp = (1.0 - (y / (double)img_h)) * 255.0;
			g_tmp = sp->rgb.g;
			b_tmp = (1.0 - (x / (double)img_w)) * 255.0;
			break;

		case EWL_COLOR_MODE_RGB_BLUE:
			r_tmp = (1.0 - (y / (double)img_h)) * 255.0;
			g_tmp = (1.0 - (x / (double)img_w)) * 255.0;
			b_tmp = sp->rgb.b;
			break;

		case EWL_COLOR_MODE_HSV_HUE:
			h = sp->hsv.h;
			s = 1.0 - (y / (double)img_h);
			v = 1.0 - (x / (double)img_w);
			ewl_spectrum_hsv_to_rgb(h, s, v, &r_tmp, &g_tmp, &b_tmp);
			break;

		case EWL_COLOR_MODE_HSV_SATURATION:
			h = (x / (double)img_w) * 360.0;
			s = sp->hsv.s;
			v = 1.0 - (y / (double)img_h);
			ewl_spectrum_hsv_to_rgb(h, s, v, &r_tmp, &g_tmp, &b_tmp);
			break;

		case EWL_COLOR_MODE_HSV_VALUE:
			h = (x / (double)img_w) * 360.0;
			s = 1.0 - (y / (double)img_h);
			v = sp->hsv.v;
			ewl_spectrum_hsv_to_rgb(h, s, v, &r_tmp, &g_tmp, &b_tmp);
			break;

		default:
			break;
	}

	if (r) *r = r_tmp;
	if (g) *g = g_tmp;
	if (b) *b = b_tmp;

	DLEAVE_FUNCTION(DLEVEL_STABLE);
}



