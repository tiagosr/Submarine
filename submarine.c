//
//  submarine.c
//  submarine
//
//  Created by Tiago Rezende on 3/13/13.
//  Copyright (c) 2013 Tiago Rezende. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "m_pd.h"
#include "pdutils.h"

static t_class *sub_class = NULL, *sub_cargo_bay = NULL, *radar_class = NULL;
typedef struct _torpedo {
    t_symbol *sym;
    int argc;
    t_atom *argv;
} torpedo;

typedef struct _sub_cargo_bay_obj {
    t_object x_obj;
    torpedo t;
} t_sub_cargo_bay_obj;

typedef struct _sub_obj {
    t_object x_obj;
    t_sub_cargo_bay_obj *cargo;
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
    sub->cargo = (t_sub_cargo_bay_obj *)pd_new(sub_cargo_bay);
    if (argc < 1) {
        sub->cargo->t.sym = &s_bang;
        sub->cargo->t.argc = 0;
        sub->cargo->t.argv = NULL;
    } else {
        sub->cargo->t.sym = atom_getsymbol(argv);
        sub->cargo->t.argc = argc-1;
        sub->cargo->t.argv = argv+1;
    }
    
    sub->inlet = inlet_new(&sub->x_obj, &sub->cargo->x_obj.ob_pd, NULL, gensym("sub-load"));
    sub->out = outlet_new(&sub->x_obj, &s_anything);
    return (void *)sub;
}

static void *sub_cargo_new(t_symbol *sym, int argc, t_atom *argv)
{
    t_sub_cargo_bay_obj *cargo = (t_sub_cargo_bay_obj *)pd_new(sub_cargo_bay);
    return (void *)cargo;
}

static void sub_any(t_sub_obj *sub, t_symbol *sym, int argc, t_atom *argv)
{
    char *liststr = list_to_string(argc, argv);
    //post("sub_any: %s", liststr);
    free(liststr);
    torpedo *prev = c_torpedo;
    c_torpedo = &sub->cargo->t;
    outlet_anything(sub->out, sym, argc, argv);
    c_torpedo = prev;
}

static void sub_float(t_sub_obj *sub, t_float f)
{
    torpedo *prev = c_torpedo;
    c_torpedo = &sub->cargo->t;
    outlet_float(sub->out, f);
    c_torpedo = prev;
}

static void sub_load(t_sub_obj *sub, t_symbol *sym, int argc, t_atom *argv)
{
    //char *liststr = list_to_string(argc, argv);
    //post("sub_load: %s", liststr);
    //free(liststr);
    sub->cargo->t.sym = atom_getsymbol(argv);
    sub->cargo->t.argc = argc-1;
    sub->cargo->t.argv = argv+1;
}

static void cargo_load(t_sub_cargo_bay_obj *cargo, t_symbol *sym, int argc, t_atom *argv)
{
    //char *liststr = list_to_string(argc, argv);
    //post("cargo_load: %s", liststr);
    //free(liststr);
    cargo->t.sym = atom_getsymbol(argv);
    cargo->t.argc = argc-1;
    cargo->t.argv = argv+1;
}


static void sub_destroy(t_sub_obj *sub)
{
    inlet_free(sub->inlet);
    outlet_free(sub->out);
    pd_free((t_pd *)sub->cargo);
}

static void *radar_new(void)
{
    t_radar_obj *radar = (t_radar_obj *)pd_new(radar_class);
    radar->default_out = outlet_new(&radar->x_obj, &s_anything);
    radar->torpedo_out = outlet_new(&radar->x_obj, &s_anything);
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
        sub_cargo_bay = class_new(gensym("kind-of-necessarily-long-name-for-the-sub-cargo-bay"),
                                  (t_newmethod)sub_cargo_new,
                                  NULL, sizeof(t_sub_cargo_bay_obj),
                                  CLASS_DEFAULT, 0);
        class_addanything(sub_cargo_bay, (t_method)cargo_load);
        sub_class = class_new(gensym("sub"),
                              (t_newmethod)sub_new,
                              (t_method)sub_destroy,
                              sizeof(t_sub_obj), CLASS_DEFAULT,
                              A_GIMME, 0);
        class_addanything(sub_class, (t_method)sub_any);
        
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
