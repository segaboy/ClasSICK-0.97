/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "window.h"
#include <stddef.h>

static int span(const void *p,size_t n)
{
    return p!=NULL && (uintptr_t)p<=UINTPTR_MAX-(n-1u);
}
static int overlap(const void *a,size_t an,const void *b,size_t bn)
{
    return (uintptr_t)a<=(uintptr_t)b+bn-1u && (uintptr_t)b<=(uintptr_t)a+an-1u;
}
static int geometry(int32_t x,int32_t y,int32_t width,int32_t height,int32_t title)
{
    if (width<2 || width>CS_POINTER_WIDTH || height<2 || height>CS_POINTER_HEIGHT) return 0;
    return title>=1 && title<height && x>=0 && x<=CS_POINTER_WIDTH-width
        && y>=0 && y<=CS_POINTER_HEIGHT-height;
}
static int canonical(const cs_window *w)
{
    if (w->valid!=CS_WINDOW_VALID || !geometry(w->x,w->y,w->width,w->height,w->title)
        || w->down>1u || w->dragging>1u || w->pattern>1u) return 0;
    if (!w->dragging) return w->grab_x==0 && w->grab_y==0;
    return w->down==1u && w->grab_x>=0 && w->grab_x<w->width
        && w->grab_y>=0 && w->grab_y<w->title;
}
static int event_valid(const cs_pointer_event *e)
{
    uint32_t f=e->flags;
    if (e->x<0 || e->x>=CS_POINTER_WIDTH || e->y<0 || e->y>=CS_POINTER_HEIGHT
        || (e->buttons&~7u)!=0u || (f&~255u)!=0u) return 0;
    if (f&CS_POINTER_CANCELLED) {
        return e->buttons==0u && (f== (CS_POINTER_CANCELLED|CS_POINTER_SUPPRESSED)
            || f== (CS_POINTER_CANCELLED|CS_POINTER_SUPPRESSED|CS_POINTER_RELEASE));
    }
    if ((f&(CS_POINTER_PRESS|CS_POINTER_RELEASE))==(CS_POINTER_PRESS|CS_POINTER_RELEASE)) return 0;
    if ((f&CS_POINTER_PRESS) && (!(f&CS_POINTER_DOWN) || (f&CS_POINTER_SUPPRESSED))) return 0;
    if ((f&CS_POINTER_RELEASE) && (f&(CS_POINTER_DOWN|CS_POINTER_SUPPRESSED))) return 0;
    if (f&CS_POINTER_SUPPRESSED) {
        return !(f&(CS_POINTER_DOWN|CS_POINTER_PRESS|CS_POINTER_RELEASE)) && (e->buttons&CS_POINTER_LEFT)!=0u;
    }
    return ((f&CS_POINTER_DOWN)!=0u)==((e->buttons&CS_POINTER_LEFT)!=0u);
}
static int32_t clamp(int32_t n,int32_t limit) { return n<0?0:n>limit?limit:n; }

int cs_window_begin(cs_window *w,const cs_window_config *c)
{
    if (!span(w,sizeof(*w)) || !span(c,sizeof(*c)) || overlap(w,sizeof(*w),c,sizeof(*c)))
        return CS_WINDOW_ARGUMENT;
    if (!geometry(c->x,c->y,c->width,c->height,c->title)) return CS_WINDOW_ARGUMENT;
    w->x=c->x; w->y=c->y; w->width=c->width; w->height=c->height; w->title=c->title;
    w->grab_x=0; w->grab_y=0; w->down=0u; w->dragging=0u; w->pattern=0u; w->valid=CS_WINDOW_VALID;
    return CS_WINDOW_OK;
}
int cs_window_apply(cs_window *w,const cs_pointer_event *e,cs_window_effect *out)
{
    uint32_t f,down,flags=0u;
    int changed;
    int32_t ox,oy;
    if (!span(w,sizeof(*w)) || !span(e,sizeof(*e)) || !span(out,sizeof(*out))
        || overlap(w,sizeof(*w),e,sizeof(*e)) || overlap(w,sizeof(*w),out,sizeof(*out))
        || overlap(e,sizeof(*e),out,sizeof(*out))) return CS_WINDOW_ARGUMENT;
    if (!canonical(w)) return CS_WINDOW_STATE;
    if (!event_valid(e)) return CS_WINDOW_ARGUMENT;
    f=e->flags; down=(f&CS_POINTER_DOWN)?1u:0u;
    if (f&CS_POINTER_CANCELLED) {
        if (((f&CS_POINTER_RELEASE)!=0u)!=(w->down!=0u)) return CS_WINDOW_STATE;
    } else if (f&CS_POINTER_PRESS) {
        if (w->down) return CS_WINDOW_STATE;
    } else if (f&CS_POINTER_RELEASE) {
        if (!w->down) return CS_WINDOW_STATE;
    } else if (down!=w->down) return CS_WINDOW_STATE;
    ox=w->x; oy=w->y; changed=down!=w->down;
    if (w->dragging && !(f&CS_POINTER_CANCELLED) && (f&CS_POINTER_MOVED)) {
        w->x=clamp(e->x-w->grab_x,CS_POINTER_WIDTH-w->width);
        w->y=clamp(e->y-w->grab_y,CS_POINTER_HEIGHT-w->height);
    }
    if (f&(CS_POINTER_RELEASE|CS_POINTER_CANCELLED)) {
        if (w->dragging) flags|=CS_WINDOW_DRAG_ENDED;
        w->dragging=0u; w->grab_x=0; w->grab_y=0;
    }
    if ((f&CS_POINTER_PRESS) && e->x>=w->x && e->x<w->x+w->width
        && e->y>=w->y && e->y<w->y+w->height) {
        if (e->y<w->y+w->title) {
            w->dragging=1u; w->grab_x=e->x-w->x; w->grab_y=e->y-w->y;
            flags|=CS_WINDOW_DRAG_BEGAN;
        } else { w->pattern^=1u; flags|=CS_WINDOW_TOGGLED; }
    }
    w->down=down;
    if (w->x!=ox || w->y!=oy) flags|=CS_WINDOW_MOVED;
    out->old_x=ox; out->old_y=oy; out->x=w->x; out->y=w->y; out->flags=flags;
    return changed || flags?CS_WINDOW_CHANGED:CS_WINDOW_OK;
}
