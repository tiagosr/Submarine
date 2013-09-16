//
//  submarine.c
//  submarine
//
//  Created by Tiago Rezende on 3/13/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include <stdio.h>
#include "m_pd.h"

static t_class *sub_class = NULL, *radar_class = NULL;
typedef struct _torpedo {
    t_symbol *sym;
    int argc;
    t_atom *argv;
} torpedo;

typedef struct _sub_obj {
    t_object x_obj;
    torpedo t;
    t_inlet *inlet;
    t_outlet *out;
} t_sub_obj;

typedef struct _radar_obj {
    t_object x_obj;
    t_outlet *torpedo_out, *default_out;
} t_radar_obj;

// should Pd ever become multi-threaded, this is ready already
//static __thread torpedo *c_torpedo = NULL;
// Xcode 4.6 crapped out on the line above, and as Pd is not multithreaded, remove TLS attribute
static torpedo *c_torpedo = NULL;

static void *sub_new(t_symbol *sym, int argc, t_atom *argv)
{
    t_sub_obj *sub = (t_sub_obj *)pd_new(sub_class);
    sub->t.sym = sym;
    sub->t.argc = argc;
    sub->t.argv = argv;
    sub->inlet = inlet_new(&sub->x_obj, &sub->x_obj.ob_pd, &s_anything, gensym("sub-load"));
    sub->out = outlet_new(&sub->x_obj, &s_list);
    return (void *)sub;
}


static void sub_any(t_sub_obj *sub, t_symbol *sym, int argc, t_atom *argv)
{
    torpedo *prev = c_torpedo;
    c_torpedo = &sub->t;
    outlet_anything(sub->out, sym, argc, argv);
    c_torpedo = prev;
}

static void sub_load(t_sub_obj *sub, t_symbol *sym, int argc, t_atom *argv)
{
    sub->t.sym = sym;
    sub->t.argc = argc;
    sub->t.argv = argv;
}

static void sub_destroy(t_sub_obj *sub)
{
    inlet_free(sub->inlet);
    outlet_free(sub->out);
}

static void *radar_new(void)
{
    t_radar_obj *radar = (t_radar_obj *)pd_new(radar_class);
    radar->torpedo_out = outlet_new(&radar->x_obj, &s_anything);
    radar->default_out = outlet_new(&radar->x_obj, &s_anything);
    return (void *)radar;
}

static void radar_any(t_radar_obj *radar, t_symbol *sym, int argc, t_atom *argv)
{
    if (c_torpedo) {
        outlet_anything(radar->torpedo_out,
                        c_torpedo->sym, c_torpedo->argc, c_torpedo->argv);
    }
    outlet_anything(radar->default_out, sym, argc, argv);
}

static void radar_destroy(t_radar_obj *radar)
{
    outlet_free(radar->default_out);
    outlet_free(radar->torpedo_out);
}


void submarine_setup(void)
{
    if (!sub_class) {
        sub_class = class_new(gensym("sub"),
                              (t_newmethod)sub_new,
                              (t_method)sub_destroy,
                              sizeof(t_sub_obj), CLASS_DEFAULT,
                              A_GIMME, 0);
        class_addanything(sub_class, (t_method)sub_any);
        class_addmethod(sub_class, (t_method)sub_load,
                        gensym("sub-load"), A_GIMME, 0);
        
        radar_class = class_new(gensym("radar"),
                                (t_newmethod)radar_new,
                                0, sizeof(t_radar_obj),
                                CLASS_DEFAULT, 0);
        class_addanything(radar_class, (t_method)radar_any);
        
    }
}

void sub_setup(void) { submarine_setup(); }
void radar_setup(void) { submarine_setup(); }
void radarroute_setup(void) { submarine_setup(); }
void radarselect_setup(void) { submarine_setup(); }
