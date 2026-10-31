/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
/* Independent, literal capability lifetime and split-span checks. */
#include <stdio.h>
#include <string.h>
#include "../probes/pc286/span-cap.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
static const cs_bm_bank bank[]={{7,64}};
static const cs_bm_region region[]={{0,64,3,0,1}};
static const cs_bm_map map={region,bank,1,1};
static const cs_bc_bank cat[]={{7,0x10000,0}};
static unsigned char a[64], b[64];
static const cs_bs_span sa[]={{7,0,17,a},{7,17,47,a+17}};
static const cs_bs_span sb[]={{7,0,17,b},{7,17,47,b+17}};
static const cs_bx_env ea={&map,cat,sa,0x10000,64,1,1,1,2};
static const cs_bx_env eb={&map,cat,sb,0x10000,64,1,1,1,2};
int main(void) {
 cs_bq_registry r={0},r2={0}; cs_bq_owner owner,other; cs_bq_cap cap,old,saved; cs_bq_run run,before; uint16_t emitted; unsigned i;
 CHECK(cs_bq_registry_init(&r,2)==CS_BQ_OK); CHECK(cs_bq_owner_open(&r,&ea,1,&owner)==CS_BQ_OK);
 CHECK(cs_bq_open(&owner,15,4,3,&cap)==CS_BQ_OK); CHECK(cs_bq_borrow(&cap,3,4,&run,&emitted)==CS_BQ_OK && emitted==1 && run.length==2 && run.write==a+15);
 run.write[0]=0x31;run.write[1]=0x32; CHECK(cs_bq_borrow(&cap,3,4,&run,&emitted)==CS_BQ_OK && emitted==1 && run.length==2 && run.write==a+17); run.write[0]=0x33;run.write[1]=0x34;
 CHECK(cs_bq_borrow(&cap,3,4,&run,&emitted)==CS_BQ_OK && emitted==0); CHECK(a[15]==0x31 && a[18]==0x34);
 CHECK(cs_bq_open(&owner,4,4,1,&old)==CS_BQ_OK); memset(&run,0xA5,sizeof run); before=run; emitted=0x5A5A; saved=old;
 CHECK(cs_bq_borrow(&old,3,4,&run,&emitted)==CS_BQ_PERMISSION); CHECK(memcmp(&run,&before,sizeof run)==0 && emitted==0x5A5A && memcmp(&saved,&old,sizeof old)==0);
 CHECK(cs_bq_owner_rebind(&owner,&eb)==CS_BQ_OK); CHECK(cs_bq_borrow(&old,1,4,&run,&emitted)==CS_BQ_STALE); CHECK(memcmp(&run,&before,sizeof run)==0 && emitted==0x5A5A);
 CHECK(cs_bq_open(&owner,4,4,3,&cap)==CS_BQ_OK); CHECK(cs_bq_borrow(&cap,3,4,&run,&emitted)==CS_BQ_OK && emitted==1 && run.write==b+4); run.write[0]=0x91; CHECK(a[4]==0 && b[4]==0x91);
 CHECK(cs_bq_owner_rebind(&owner,&ea)==CS_BQ_EXHAUSTED); CHECK(cs_bq_open(&owner,4,4,1,&cap)==CS_BQ_OK); CHECK(cs_bq_borrow(&cap,1,4,&run,&emitted)==CS_BQ_OK && run.read==b+4 && run.write==NULL);
 CHECK(cs_bq_registry_init(&r2,2)==CS_BQ_OK); CHECK(cs_bq_owner_open(&r2,&ea,1,&other)==CS_BQ_OK); CHECK(cs_bq_open(&other,0,4,1,&old)==CS_BQ_OK);
 CHECK(cs_bq_owner_open(&r2,&eb,1,&other)==CS_BQ_OK); CHECK(cs_bq_borrow(&old,1,4,&run,&emitted)==CS_BQ_STALE);
 CHECK(cs_bq_registry_init(&r2,2)==CS_BQ_STATE); CHECK(cs_bq_open(&other,0,4,1,&cap)==CS_BQ_OK); CHECK(cs_bq_release(&cap)==CS_BQ_OK); CHECK(cs_bq_borrow(&cap,1,4,&run,&emitted)==CS_BQ_CAPABILITY);
 for(i=0;i<64;i++) if(i!=15 && i!=16 && i!=17 && i!=18) CHECK(a[i]==0);
 puts("PASS independent split, rights, lifetime, exhaustion and preservation checks"); return 0;
}
