/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (C) 2026 Dean Howell. */
#include "uart-fixture.h"
#include <stdio.h>
static unsigned failures;
#define CHECK(x) do { if(!(x)) { fprintf(stderr,"line %d: %s\n",__LINE__,#x); ++failures; } } while(0)
static void boot(cs_uart *s,uart_fixture *f,unsigned char *bytes,uint32_t divisor)
{
    cs_uart_io io; uf_init(f); io=uf_io(f); CHECK(cs_uart_begin(s,bytes,0x3F8,divisor)==0);
    for(unsigned i=0;i<20;++i) {
        unsigned before=f->operations; CHECK(cs_uart_poll(s,&io,i)==(i==19?1u:0u));
        CHECK(f->operations==before+1 && f->violations==0);
    }
}
static void startup(void)
{
    static const uint8_t offsets[]={3,1,2,4,7,7,7,7,7,7,3,0,1,0,1,3,3,1,4,2};
    static const uint8_t directions[]={1,1,1,1,0,1,0,1,0,1,1,1,1,0,0,1,0,0,0,0};
    static const uint8_t values[]={3,0,7,3,0xD3,0xA5,0xA5,0x5A,0x5A,0xD3,0x83,0x34,0x12,0x34,0x12,3,3,0,3,0xC1};
    cs_uart s; uart_fixture f; unsigned char bytes[514]; memset(bytes,0xE7,sizeof bytes);
    boot(&s,&f,bytes+1,0x1234);
    CHECK(memcmp(f.offsets,offsets,20)==0 && memcmp(f.directions,directions,20)==0 && memcmp(f.values,values,20)==0);
    CHECK(f.regs[7]==0xD3 && f.regs[3]==3 && f.regs[1]==0 && f.regs[4]==3);
    for(unsigned i=0;i<sizeof bytes;++i) CHECK(bytes[i]==0xE7);
    for(unsigned i=0;i<256;++i) {
        unsigned divisor=i==0?1u:i*257u;
        boot(&s,&f,bytes+1,divisor); CHECK(f.dll==(uint8_t)divisor && f.dlm==(uint8_t)(divisor>>8));
    }
}
static void errors(void)
{
    static const unsigned phases[]={6,8,13,14,16,17,18,19};
    cs_uart s; uart_fixture f; cs_uart_io io; unsigned char bytes[512];
    for(unsigned j=0;j<8;++j) {
        unsigned read_count=0; uf_init(&f); io=uf_io(&f); (void)cs_uart_begin(&s,bytes,0x3F8,12);
        for(unsigned i=0;i<=phases[j];++i) {
            if(i==phases[j]) { f.bad_offset=i==6||i==8?7u:i==13?0u:i==14||i==17?1u:i==16?3u:i==18?4u:2u; f.bad_read=read_count+1u; }
            CHECK(cs_uart_poll(&s,&io,i)==(i==phases[j]?6u:0u)); read_count=f.reads;
        }
        unsigned ops=f.operations;
        CHECK(cs_uart_poll(&s,&io,100)==6 && f.operations==ops);
        CHECK(cs_uart_enqueue(&s,(const unsigned char *)"x",1)==6 && cs_uart_finish(&s)==6);
    }
    for(unsigned lsr=0;lsr<256;++lsr) {
        boot(&s,&f,bytes,12); io=uf_io(&f); f.lsr=(uint8_t)lsr;
        CHECK(cs_uart_enqueue(&s,(const unsigned char *)"x",1)==1);
        CHECK(cs_uart_poll(&s,&io,20)==((lsr&0x9Eu)?7u:1u));
        CHECK(s.lsr==lsr && s.line_bits==(lsr&0x9Eu));
        CHECK(f.output_count==(((lsr&0x9Eu)==0 && (lsr&0x20u)!=0)?1u:0u));
    }
}
static void queue(void)
{
    cs_uart s; uart_fixture f; cs_uart_io io; unsigned char bytes[514],message[512];
    for(unsigned i=0;i<512;++i) message[i]=(unsigned char)i;
    memset(bytes,0xE7,sizeof bytes); boot(&s,&f,bytes+1,12); io=uf_io(&f);
    CHECK(cs_uart_enqueue(&s,NULL,0)==1 && cs_uart_enqueue(&s,message,512)==1);
    CHECK(cs_uart_enqueue(&s,message,1)==8 && s.count==512 && s.dropped_records==1 && s.dropped_bytes==1);
    CHECK(memcmp(bytes+1,message,512)==0); f.lsr=0;
    CHECK(cs_uart_poll(&s,&io,20)==1 && s.count==512 && f.output_count==0);
    f.lsr=0x20;
    for(unsigned i=0;i<300;++i) CHECK(cs_uart_poll(&s,&io,21u+i)==1);
    CHECK(cs_uart_enqueue(&s,message,300)==1 && s.count==512);
    s.sent=UINT32_MAX; s.dropped_records=UINT32_MAX; s.dropped_bytes=UINT32_MAX-1;
    CHECK(cs_uart_enqueue(&s,message,2)==8 && s.dropped_records==UINT32_MAX && s.dropped_bytes==UINT32_MAX);
    CHECK(cs_uart_finish(&s)==1 && cs_uart_enqueue(&s,message,1)==4);
    for(unsigned i=0;i<512;++i) CHECK(cs_uart_poll(&s,&io,321u+i)==1);
    CHECK(f.output_count==812 && memcmp(f.output,message,512)==0 && memcmp(f.output+512,message,300)==0);
    CHECK(s.sent==UINT32_MAX && s.count==0 && s.temt==0);
    CHECK(cs_uart_poll(&s,&io,833)==1); f.lsr=0x60; CHECK(cs_uart_poll(&s,&io,834)==2 && s.temt==1);
    unsigned ops=f.operations; CHECK(cs_uart_poll(&s,&io,835)==2 && f.operations==ops);
    CHECK(bytes[0]==0xE7 && bytes[513]==0xE7);
}
static void bounds(void)
{
    cs_uart s; uart_fixture f; cs_uart_io io; unsigned char bytes[512];
    uf_init(&f); io=uf_io(&f); memset(&s,0xA5,sizeof s); cs_uart before=s;
    CHECK(cs_uart_begin(NULL,bytes,0x3F8,12)==3 && cs_uart_begin(&s,NULL,0x3F8,12)==3);
    CHECK(cs_uart_begin(&s,bytes,0,12)==3 && cs_uart_begin(&s,bytes,65529,12)==3);
    CHECK(cs_uart_begin(&s,bytes,0x3F8,0)==3 && cs_uart_begin(&s,bytes,0x3F8,65536)==3);
    CHECK(memcmp(&s,&before,sizeof s)==0 && f.operations==0);
    boot(&s,&f,bytes,12); CHECK(cs_uart_enqueue(&s,NULL,1)==3 && cs_uart_enqueue(&s,bytes,513)==3);
    before=s; CHECK(cs_uart_poll(&s,NULL,20)==3 && memcmp(&s,&before,sizeof s)==0);
    CHECK(cs_uart_poll(&s,&io,18)==4); unsigned ops=f.operations;
    CHECK(cs_uart_poll(&s,&io,20)==4 && f.operations==ops);
    for(unsigned stage=0;stage<19;++stage) {
        uf_init(&f); (void)cs_uart_begin(&s,bytes,0x3F8,12);
        for(unsigned i=0;i<=stage;++i) (void)cs_uart_poll(&s,&io,i);
        ops=f.operations; CHECK(cs_uart_poll(&s,&io,CS_UART_TICKS)==5 && f.operations==ops);
    }
    boot(&s,&f,bytes,12); CHECK(cs_uart_enqueue(&s,(const unsigned char *)"x",1)==1); f.lsr=0;
    for(unsigned i=0;i<CS_UART_POLLS;++i) CHECK(cs_uart_poll(&s,&io,20)==1);
    ops=f.operations; CHECK(cs_uart_poll(&s,&io,20)==5 && f.operations==ops && s.count==1);
    boot(&s,&f,bytes,12); f.lsr=0x20; (void)cs_uart_finish(&s);
    CHECK(cs_uart_poll(&s,&io,20)==1); ops=f.operations;
    CHECK(cs_uart_poll(&s,&io,20+CS_UART_TICKS)==5 && f.operations==ops);
    boot(&s,&f,bytes,12); s.count=513; ops=f.operations;
    CHECK(cs_uart_poll(&s,&io,20)==4 && f.operations==ops);
    boot(&s,&f,bytes,12); s.phase=19; ops=f.operations;
    CHECK(cs_uart_poll(&s,&io,20)==4 && f.operations==ops);
    CHECK(cs_uart_begin(&s,bytes,65528,65535)==0);
    boot(&s,&f,bytes,12); CHECK(cs_uart_poll(&s,&io,UINT64_MAX)==1);
    (void)cs_uart_finish(&s); CHECK(cs_uart_poll(&s,&io,UINT64_MAX)==2);
}
int main(int argc,char **argv)
{
    const char *suite=argc>1?argv[1]:"";
    if(strcmp(suite,"startup")==0) startup(); else if(strcmp(suite,"errors")==0) errors();
    else if(strcmp(suite,"queue")==0) queue(); else if(strcmp(suite,"bounds")==0) bounds(); else return 2;
    if(failures) return 1;
    printf("UART %s: PASS\n",suite); return 0;
}
