/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. Original contract model; implementation withheld. */
#include <stdio.h>
#include <stdlib.h>
#include "../core/ui/window.h"
static unsigned checks;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); exit(1); } } while(0)
static int32_t bound(int32_t n,int32_t max) { return n<0?0:n>max?max:n; }
static void compare(cs_window *actual,cs_window *model,const cs_pointer_event *event)
{
    cs_window_effect out;
    int32_t ox=model->x,oy=model->y;
    uint32_t od=model->down,og=model->dragging,op=model->pattern,flags=0u;
    int cancel=(event->flags&CS_POINTER_CANCELLED)!=0u;
    if (!cancel && model->dragging && (event->flags&CS_POINTER_MOVED)) {
        model->x=bound(event->x-model->grab_x,CS_POINTER_WIDTH-model->width);
        model->y=bound(event->y-model->grab_y,CS_POINTER_HEIGHT-model->height);
    }
    if (cancel || (event->flags&CS_POINTER_RELEASE)) {
        if (model->dragging) flags|=CS_WINDOW_DRAG_ENDED;
        model->dragging=0u; model->grab_x=0; model->grab_y=0;
    }
    if (event->flags&CS_POINTER_PRESS) {
        int32_t rx=event->x-model->x,ry=event->y-model->y;
        if (rx>=0 && rx<model->width && ry>=0 && ry<model->height) {
            if (ry<model->title) {
                model->dragging=1u; model->grab_x=rx; model->grab_y=ry;
                flags|=CS_WINDOW_DRAG_BEGAN;
            } else { model->pattern^=1u; flags|=CS_WINDOW_TOGGLED; }
        }
    }
    model->down=(event->flags&CS_POINTER_DOWN)?1u:0u;
    if (ox!=model->x || oy!=model->y) flags|=CS_WINDOW_MOVED;
    CHECK(cs_window_apply(actual,event,&out)==((flags || od!=model->down)?CS_WINDOW_CHANGED:CS_WINDOW_OK));
    CHECK(out.old_x==ox && out.old_y==oy && out.x==model->x && out.y==model->y && out.flags==flags);
    CHECK(actual->x==model->x && actual->y==model->y && actual->width==model->width && actual->height==model->height);
    CHECK(actual->title==model->title && actual->down==model->down && actual->dragging==model->dragging);
    CHECK(actual->pattern==model->pattern && actual->grab_x==model->grab_x && actual->grab_y==model->grab_y && actual->valid==model->valid);
    CHECK(og==actual->dragging || (flags&(CS_WINDOW_DRAG_BEGAN|CS_WINDOW_DRAG_ENDED)));
    CHECK(op==actual->pattern || (flags&CS_WINDOW_TOGGLED));
}
int main(void)
{
    cs_window a,m;
    cs_pointer pointer;
    cs_pointer_event event;
    cs_window_config config={80,70,224,144,18};
    uint32_t seed=0x120974u;
    CHECK(cs_window_begin(&a,&config)==CS_WINDOW_OK);
    m.x=80;m.y=70;m.width=224;m.height=144;m.title=18;m.grab_x=0;m.grab_y=0;
    m.down=0u;m.dragging=0u;m.pattern=0u;m.valid=CS_WINDOW_VALID;
    CHECK(cs_pointer_begin(&pointer,0u)==CS_POINTER_OK);
    for (unsigned n=0u;n<10000u;++n) {
        seed=seed*1664525u+1013904223u;
        if (n%97u==0u) { CHECK(cs_pointer_cancel(&pointer,&event)==CS_POINTER_CHANGED); }
        else {
            int32_t x=(int32_t)((seed>>8)%512u),y=(int32_t)((seed>>19)%342u);
            uint32_t buttons=(seed>>25)&7u;
            cs_pointer_sample sample;
            if (n%12u<4u) { x=m.x+8; y=m.y+4; buttons=(n%4u==1u || n%4u==2u)?1u:0u; }
            if (n%12u>=8u) { x=m.x+8;y=m.y+m.title+4; buttons=(n%4u==1u || n%4u==2u)?1u:0u; }
            sample.dx=x-pointer.x;sample.dy=y-pointer.y;sample.buttons=buttons;
            CHECK(cs_pointer_sample_apply(&pointer,&sample,&event)<=CS_POINTER_CHANGED);
        }
        compare(&a,&m,&event);
    }
    printf("Window reference PASS:10000 authentic transitions,%u checks\n",checks);
    return 0;
}
