/*
 * Copyright (C) 1997-2000, Michael Jennings
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies of the Software, its documentation and marketing & publicity
 * materials, and acknowledgment shall be given in the documentation, materials
 * and software packages that this Software was used.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _FEATURE_H_
# define _FEATURE_H_

/********************* Miscellaneous OS fixings *********************/

# if defined(hpux) && !defined(_HPUX_SOURCE)
#  define _HPUX_SOURCE
# endif

/*
# if defined(_HPUX_SOURCE) && !defined(SVR4)
#  define SVR4
# endif
 */

# if defined(SVR4) || defined(__SVR4)
#  ifndef __svr4__
#   define __svr4__
#  endif
# endif

# if defined(sun) && !defined(__sun__)
#  define __sun__
# endif
# if !defined(sun) && defined(__sun__)
#  define sun
# endif

# if defined (sun)
#  undef HAVE_SYS_IOCTL_H
# endif

/********************* Debugging stuff *********************/
/* As Keith Bunge would say, don't crap with the debugging stuff below
 * unless you develop this mess. :^)   -- mej
 */

# ifndef DEBUG
#   define DEBUG 0
# endif

/********************* Random development stuff ***************************/
#ifdef ENABLE_PROFILE
# define PROFILE_SCREEN
# define PROFILE_X_EVENTS
# define COUNT_X_EVENTS
#endif

#define OPTIMIZE_HACKS
#define USE_EFFECTS

/* For Pablo I/O Trace Library */
#ifdef IOTRACE
# include <IOTrace.h>
# include <PabloTrace.h>
# define PABLO_START_TRACING()  do {setTraceProcessorNumber(0); setTraceFileName("Eterm-pablo-trace.sddf"); initIOTrace(); \
                                    enableLifetimeSummaries(); enableTimeWindowSummaries(0.1); enableFileRegionSummaries(8192); \
                                   } while (0)
# define PABLO_STOP_TRACING()   do {endIOTrace(); endTracing();} while (0)
#else
# define PABLO_START_TRACING()  ((void) 0)
# define PABLO_STOP_TRACING()   ((void) 0)
#endif

# include <stdio.h>
# include <stdlib.h>

# include <libast.h>
# include "eterm_debug.h"

/********************* Color, screen, and image stuff *********************/

/* Support for background pixmap cycling */
#define BACKGROUND_CYCLING_SUPPORT

/* The environment variable in which Eterm looks for a search path for
   config files and pixmaps */
#define PATH_ENV	"ETERMPATH"

/* Disable simulation of bold font using an "overstrike" technique.  This technique
 has been known to cause pixel droppings.  See also FORCE_CLEAR_CHARS. */
/* #define NO_BOLDOVERSTRIKE */

/* Disable the secondary screen ("\E[?47h" / "\E[?47l") */
/* #define NO_SECONDARY_SCREEN */

/* The number of screenfuls between refreshes. */
# define REFRESH_PERIOD  5

/* This will force clearing of characters before writing new ones on top of
 * them. This is experimental - added in order to try and fix pixel dropping
 * problems some people have had. See also NO_BOLDOVERSTRIKE. */
/*# define FORCE_CLEAR_CHARS*/

/* The command through which to pipe print-screen requests */
#define PRINTPIPE	"lp"

/* If the screen can handle 24-bit graphics, force them */
/* #define PREFER_24BIT */

/* Offer some support for the Offix DND (Drag 'n' Drop) protocol (untested) */
/* #define OFFIX_DND */

/* Allows the -w and --border-width options for specifying the width of the
 * border (in pixels) between the actual X client window and the program-useable
 * terminal window.                                                 -- mej
 */
# define BORDER_WIDTH_OPTION

/********************* Key and key-bindings options *********************/

/* Pick the hotkey for changing the font size */
# define HOTKEY_CTRL
/* #define HOTKEY_META */

/* Use Home = "\E[1~" and End = "\E[4~" instead of Home = "\E[7~" and End = "\E[8~" */
/* #define LINUX_KEYS */

/* Allow the "keysym" attribute in config files for remapping keysyms */
#define KEYSYM_ATTRIBUTE

/* Allow unshifted Next and Prior keys to scroll, in addition to their shifted
 * counterparts */
/* #define UNSHIFTED_SCROLLKEYS */

/********************* Mouse, scrollbar, and menu bar options *********************/

/* Disable sending escape sequences from the scrollbar when mouse reporting
 * is enabled */
/* #define NO_SCROLLBAR_REPORT */

/* Set the default number of lines in the scrollback buffer */
/* #define SAVELINES 256 */

/* Set the default separator characters for double-click word selection */
#define CUTCHARS "\"&'()*,;<=>?@[\\]^`{|}~ \t"

/* Make it an option */
#define CUTCHAR_OPTION

/* To activate double-click reporting for button 1 */
/* #define MOUSE_REPORT_DOUBLECLICK */

/* The delay in milliseconds between multiple clicks */
/* #define MULTICLICK_TIME 500 */

/* Support for the old xterm-style scrollbar */
#define XTERM_SCROLLBAR

/* Support for the traditional motif-style scrollbar */
#define MOTIF_SCROLLBAR

/* Support for a NeXT-style scrollbar */
#define NEXT_SCROLLBAR

/* Default scrollbar type */
#define SCROLLBAR_DEFAULT_TYPE SCROLLBAR_MOTIF

/* The default width (in pixels) of the scrollbar. */
#define SB_WIDTH 10

/* Continuous scrolling by pressing the scrollbar arrow buttons */
#define SCROLLBAR_BUTTON_CONTINUAL_SCROLLING

/* Delay periods for continuous scrolling */
/* #define SCROLLBAR_INITIAL_DELAY 40 */
/* #define SCROLLBAR_CONTINUOUS_DELAY 2 */

/* How many lines of context to keep on screen when paging up/down */
#define CONTEXT_LINES 1

/********************* Multi-lingual support options *********************/

/* Allow option/attribute for Meta to set the 8th bit */
#define META8_OPTION

/********************* Miscellaneous options *********************/

/* To have $DISPLAY and the "\E[7n" response be IP addresses rather than FQDN's */
/* #define DISPLAY_IS_IP */

/* To have "\E[7n" reply with the display name.  This is a potential security risk,
 * so its use is discouraged and unsupported. */
/* #define ENABLE_DISPLAY_ANSWER */

/* To control what the Eterm detection sequence, ESC-Z, replies with */
/* #define ESCZ_ANSWER	"\033[?1;2C" */

/* Comment this out to allow printing of the VT100_ANS sequence. See
 * command.c. I have no idea what this is supposed to do, but disabling
 * it will prevent your terminal from getting garbage when ^E (ctrl-E)
 * is printed on it.
 */
#define NO_VT100_ANS

/* Allow changing of the foreground and background colors with "\E]39;color^G" */
#define XTERM_COLOR_CHANGE

/* Disable automatic de-iconify on bell altogether */
/* #define NO_MAPALERT */

/* Make it an option */
#define MAPALERT_OPTION

/********************* Config file parser options *********************/

/* Allow evaluation of expressions like `echo hello` in config files.  The
 * security-paranoid will not want to define this, but I have replaced the 
 * OS system() call with a secure one that I have tested and verified, so
 * child processes run in this way will not run with any privileges, active
 * or attainable.
 */
#define ALLOW_BACKQUOTE_EXEC

/* This causes Eterm to warn you if a config file it's about to parse was
 * designed for an older version of Eterm. */
/* #define WARN_OLDER */

/********************* Anti-cl00bie protection (sigh) *********************/
/* EDITING THIS FILE BELOW THIS LINE IS UNSUPPORTED!  YOU HAVE BEEN WARNED! */

#ifdef MULTI_CHARSET
# undef GREEK_SUPPORT
# undef XTERM_FONT_CHANGE
# undef DEFINE_XTERM_COLOR
#endif	/* MULTI_CHARSET */
#define FONT0_IDX 2

#ifndef PIXMAP_SUPPORT
# undef PIXMAP_OFFSET
# undef IMLIB_TRANS
# undef BACKGROUND_CYCLING_SUPPORT
# undef WATCH_PIXMAP_OPTION
#endif

#ifndef PIXMAP_OFFSET
# undef WATCH_DESKTOP_OPTION
#endif

#ifndef HAVE_MEMMOVE
inline void *memmove(void *, const void *, size_t);
#endif

#define APL_NAME	"Eterm"

/* COLORTERM, TERM environment variables */
#define TERMENV       "Eterm"
#define COLORTERMENV  "Eterm"

#ifdef NO_MOUSE_REPORT
# ifndef NO_MOUSE_REPORT_SCROLLBAR
#  define NO_MOUSE_REPORT_SCROLLBAR
# endif
#endif

#ifndef DEFAULT_BORDER_WIDTH
# define DEFAULT_BORDER_WIDTH 5
#endif

#ifndef SB_WIDTH
# define SB_WIDTH 10
#endif

#ifndef MENUBAR_MAX
# define MENUBAR_MAX 0
#endif

#ifndef SAVELINES
# define SAVELINES 256
#endif

#ifdef NO_SECONDARY_SCREEN
# define NSCREENS       0
#else
# define NSCREENS       1
#endif

#ifndef DEFAULT_REFRESH
# define DEFAULT_REFRESH FAST_REFRESH
#endif

#ifndef CUTCHARS
# define CUTCHARS "\"&'()*,;<=>?@[\\]^`{|}~"
#endif

#if defined (__sun__) || defined (__svr4__)
# define NO_DELETE_KEY		/* These systems seem to be anal this way*/
#endif

#ifndef PATH_ENV
# define PATH_ENV "ETERMPATH"
#endif

/* utmp doesn't work on CygWin32 */
#ifdef __CYGWIN32__
# undef UTMP_SUPPORT
#endif

#endif	/* _FEATURE_H_ */
