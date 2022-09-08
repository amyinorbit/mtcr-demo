/*===--------------------------------------------------------------------------------------------===
 * xplane.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2022 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include <acfutils/glew.h>
#include <acfutils/log.h>
#include <acfutils/mt_cairo_render.h>
#include <XPLMDisplay.h>
#include <XPLMGraphics.h>
#include <XPLMPlugin.h>
#include <XPLMUtilities.h>

#define PLUGIN_NAME "MT Cairo Render Demo"
#define PLUGIN_SIG "com.amyinorbit.mtcr-demo"
#define PLUGIN_DESC "what it says on the tin"

#define PANEL_X (100)
#define PANEL_Y (100)

#define DISPLAY_FPS (25)

#define DISPLAY_W   (400)
#define DISPLAY_H   (400)


static mt_cairo_render_t *render = NULL;

// The cairo drawing callback. This is called in the brackground thread by
// mt_cairo_render_t.
static void display_render_cb(cairo_t *cr, unsigned w, unsigned h, void *data) {
    // In a real use scenerio, this would be a pointer to whatever data you need to
    // have for drawing. Here, we don't *really* care
    UNUSED(data);
    
    // This clears the surface, so we have a clean slate to work from
    cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    
    // Draw some stuff!
    cairo_set_source_rgb(cr, 1, 0, 1);
    cairo_arc(cr, w/2, h/2, MIN(w, h)/2, 0, 2 * M_PI);
    cairo_fill(cr);
}

// Here we draw the cairo-generated image onto the panel texture.
//
// You do not have to have a draw loop -- you could for example draw on a window (using the
// window's draw callback)
static int draw_loop(XPLMDrawingPhase phase, int is_before, void *refcon) {
    UNUSED(phase);
    UNUSED(is_before);
    UNUSED(refcon);
    
    // This is the coordinates at which we want to draw on the panel texture.
    // !!warning!! This is OpenGL coordinates, so from the bottom of the texture
    vect2_t panel_loc = VECT2(PANEL_X, PANEL_Y);

    mt_cairo_render_draw(render, panel_loc, VECT2(DISPLAY_W, DISPLAY_H));
    return 1;
}

// This is the actual XP machinery

PLUGIN_API int XPluginStart(char *name, char *sig, char *desc) {
	XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);
    
	strcpy(name, PLUGIN_NAME);
	strcpy(sig, PLUGIN_SIG);
	strcpy(desc, PLUGIN_DESC);

    // Initialise all of the data required by mt_cairo_render_t
    log_init(XPLMDebugString, "mt_cairo_render");
    glewInit();
    mt_cairo_render_glob_init();
    
    return 1;
}


PLUGIN_API void XPluginStop(void) {
    
}

PLUGIN_API int XPluginEnable(void) {
    // Create our actual renderer
    render = mt_cairo_render_init(
        DISPLAY_W, DISPLAY_H,           // The size of our renderer/display
        DISPLAY_FPS,
        NULL, display_render_cb, NULL,  // callback. init, render, fini. only render is required
        NULL                            // arbitrary data pointer, passed to callbacks
    );
        
    // Fire up a display loop to draw our rendered image on the panel
    XPLMRegisterDrawCallback(draw_loop, xplm_Phase_Gauges, 1, NULL);
    
    return 1;
}

PLUGIN_API void XPluginDisable(void) {
    // Spin down the draw loop & destroy the renderer
    XPLMUnregisterDrawCallback(draw_loop, xplm_Phase_Gauges, 1, NULL);
    mt_cairo_render_fini(render);
    render = NULL;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID from, int msg, void *param) {
    UNUSED(from);
    UNUSED(msg);
    UNUSED(param);
}

#if	IBM
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID resvd)
{
	UNUSED(hinst);
	UNUSED(resvd);
	lacf_glew_dllmain_hook(reason);
	return (TRUE);
}
#endif	/* IBM */

