/*
 * Screensaver code
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* ifdef HAVE_CONFIG_H */

#include "Ecore.h"
#include "ecore_x_private.h"
#include "Ecore_X.h"
#include "Ecore_X_Atoms.h"

static int _screensaver_available = -1;

EAPI Eina_Bool
ecore_x_screensaver_event_available_get(void)
{
   if (_screensaver_available >= 0)
     return _screensaver_available;

#ifdef ECORE_XSS
   int _screensaver_major, _screensaver_minor;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   _screensaver_major = 1;
   _screensaver_minor = 0;

   if (XScreenSaverQueryVersion(_ecore_x_disp, &_screensaver_major,
                                &_screensaver_minor))
     _screensaver_available = 1;
   else
     _screensaver_available = 0;

#else /* ifdef ECORE_XSS */
   _screensaver_available = 0;
#endif /* ifdef ECORE_XSS */
   return _screensaver_available;
}

EAPI int
ecore_x_screensaver_idle_time_get(void)
{
#ifdef ECORE_XSS
   XScreenSaverInfo *xss;
   int idle;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   xss = XScreenSaverAllocInfo();
   XScreenSaverQueryInfo(_ecore_x_disp,
                         RootWindow(_ecore_x_disp, DefaultScreen(
                                      _ecore_x_disp)), xss);
   idle = xss->idle / 1000;
   XFree(xss);

   return idle;
#else
   return 0;
#endif /* ifdef ECORE_XSS */
}

EAPI void
ecore_x_screensaver_set(int timeout,
                        int interval,
                        int prefer_blanking,
                        int allow_exposures)
{
   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XSetScreenSaver(_ecore_x_disp,
                   timeout,
                   interval,
                   prefer_blanking,
                   allow_exposures);
}

EAPI void
ecore_x_screensaver_timeout_set(int timeout)
{
   int pto, pint, pblank, pexpo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   XSetScreenSaver(_ecore_x_disp, timeout, pint, pblank, pexpo);
}

EAPI int
ecore_x_screensaver_timeout_get(void)
{
   int pto, pint, pblank, pexpo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return pto;
}

EAPI void
ecore_x_screensaver_blank_set(int blank)
{
   int pto, pint, pblank, pexpo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   XSetScreenSaver(_ecore_x_disp, pto, pint, blank, pexpo);
}

EAPI int
ecore_x_screensaver_blank_get(void)
{
   int pto, pint, pblank, pexpo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return pblank;
}

EAPI void
ecore_x_screensaver_expose_set(int expose)
{
   int pto, pint, pblank, pexpo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   XSetScreenSaver(_ecore_x_disp, pto, pint, pblank, expose);
}

EAPI int
ecore_x_screensaver_expose_get(void)
{
   int pto, pint, pblank, pexpo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return pexpo;
}

EAPI void
ecore_x_screensaver_interval_set(int interval)
{
   int pto, pint, pblank, pexpo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   XSetScreenSaver(_ecore_x_disp, pto, interval, pblank, pexpo);
}

EAPI int
ecore_x_screensaver_interval_get(void)
{
   int pto, pint, pblank, pexpo;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   XGetScreenSaver(_ecore_x_disp, &pto, &pint, &pblank, &pexpo);
   return pint;
}

EAPI void
ecore_x_screensaver_event_listen_set(Eina_Bool on)
{
#ifdef ECORE_XSS
   Ecore_X_Window root;

   LOGFN(__FILE__, __LINE__, __FUNCTION__);
   root = DefaultRootWindow(_ecore_x_disp);
   if (on)
     XScreenSaverSelectInput(_ecore_x_disp, root,
                             ScreenSaverNotifyMask | ScreenSaverCycle);
   else
     XScreenSaverSelectInput(_ecore_x_disp, root, 0);
#else
   return;
   on = EINA_FALSE;
#endif /* ifdef ECORE_XSS */
}


EAPI Eina_Bool
ecore_x_screensaver_custom_blanking_enable(void)
{
#ifdef ECORE_XSS
   XSetWindowAttributes attr;
   
   XScreenSaverSetAttributes(_ecore_x_disp,
                             DefaultRootWindow(_ecore_x_disp),
                             -9999, -9999, 1, 1, 0,
                             CopyFromParent, InputOnly, CopyFromParent,
                             0, &attr);
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif /* ifdef ECORE_XSS */
}

EAPI Eina_Bool
ecore_x_screensaver_custom_blanking_disable(void)
{
#ifdef ECORE_XSS
   XScreenSaverUnsetAttributes(_ecore_x_disp,
                               DefaultRootWindow(_ecore_x_disp));
   return EINA_TRUE;
#else
   return EINA_FALSE;
#endif /* ifdef ECORE_XSS */
}

