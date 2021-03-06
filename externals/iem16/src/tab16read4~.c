/* copyleft (c) 2003 forum::f�r::uml�ute -- IOhannes m zm�lnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16write~, tab16play~, tab16read~, tab16read4~, tab16send~, tab16receive~ */

#include "iem16_table.h"

/* the following two functions are copied from table16.c so that each class can
   be instantiated without a dependency on table16 already existing */
static int table16_getarray16(t_table16*x, int*size,t_iem16_16bit**vec){
  *size=x->x_size;
  *vec =x->x_table;
  return 1;
}

static void table16_usedindsp(t_table16*x){
  x->x_usedindsp=1;
}

/******************** tab16read4~ ***********************/

static t_class *tab16read4_tilde_class;

typedef struct _tab16read4_tilde{
  t_object x_obj;
  int x_npoints;
  t_iem16_16bit *x_vec;
  t_symbol *x_arrayname;
  float x_f;
} t_tab16read4_tilde;

static void *tab16read4_tilde_new(t_symbol *s){
  t_tab16read4_tilde *x = (t_tab16read4_tilde *)pd_new(tab16read4_tilde_class);
  x->x_arrayname = s;
  x->x_vec = 0;
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_f = 0;
  return (x);
}

static t_int *tab16read4_tilde_perform(t_int *w){
  t_tab16read4_tilde *x = (t_tab16read4_tilde *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);    
  int maxindex;
  t_iem16_16bit *buf = x->x_vec;
  t_iem16_16bit *fp;
  int i;
    
  maxindex = x->x_npoints - 3;

  if (!buf) goto zero;

  for (i = 0; i < n; i++)    {
    float findex = *in++;
    int index = findex;
    float frac,  a,  b,  c,  d, cminusb;
    if (index < 1)   index = 1, frac = 0;
    else if (index > maxindex)  index = maxindex, frac = 1;
    else frac = findex - index;
    fp = buf + index;
    a = fp[-1]*IEM16_SCALE_DOWN;
    b = fp[0]*IEM16_SCALE_DOWN;
    c = fp[1]*IEM16_SCALE_DOWN;
    d = fp[2]*IEM16_SCALE_DOWN;
    cminusb = c-b;
    *out++ = b + frac * (
			 cminusb - 0.5f * (frac-1.) * (
				(a - d + 3.0f * cminusb) * frac + (b - a - cminusb)
				)
			 );
  }
  return (w+5);
 zero:
  while (n--) *out++ = 0;
  
  return (w+5);
}

void tab16read4_tilde_set(t_tab16read4_tilde *x, t_symbol *s){
  t_table16 *a;
    
  x->x_arrayname = s;
  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class))) {
    if (*s->s_name)
      error("tab16read4~: %s: no such array", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else if (!table16_getarray16(a, &x->x_npoints, &x->x_vec))  {
    error("%s: bad template for tab16read4~", x->x_arrayname->s_name);
    x->x_vec = 0;
  }
  else table16_usedindsp(a);
}

static void tab16read4_tilde_dsp(t_tab16read4_tilde *x, t_signal **sp){
  tab16read4_tilde_set(x, x->x_arrayname);

  dsp_add(tab16read4_tilde_perform, 4, x,
	  sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void tab16read4_tilde_setup(void){
  tab16read4_tilde_class = class_new(gensym("tab16read4~"),
				     (t_newmethod)tab16read4_tilde_new, 0,
				     sizeof(t_tab16read4_tilde), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(tab16read4_tilde_class, t_tab16read4_tilde, x_f);
  class_addmethod(tab16read4_tilde_class, (t_method)tab16read4_tilde_dsp,
		  gensym("dsp"), A_CANT, 0);
  class_addmethod(tab16read4_tilde_class, (t_method)tab16read4_tilde_set,
		  gensym("set"), A_SYMBOL, 0);
}
