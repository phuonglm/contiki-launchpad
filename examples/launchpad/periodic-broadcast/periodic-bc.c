/*
 * Copyright (c) 2012
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

/**
 * \file
 *         Periodic broadcast example
 *         For this to work you need at least two Launchpads with 2553 and external
 *         32.768 kHz oscillator and cc2500 radios. The nodes will periodically
 *         transmit a broadcast that will be received by any node that "hears" it
 *         and has opened a broadcast on that Rime channel.
 *         For addresses to work, each Launchpad must have an address burnt into
 *         it. To do so you need to connect the Launchpad, have mspdebug installed
 *         and on the path, go to contiki-launchpad/tools/launchpad/burnid and run
 *         the burnid script. Eg for node id 240.0
 *            burnid 240 0
 *         To save RAM, each address is just 2 bytes (compare with IPv6 or even
 *         IPv4 address that are much longer). This makes eg routing tables and
 *         neighbor tables smaller if you want to use/implement such.
 *         
 * \author
 *         Marcus Lunden <marcus.lunden@gmail.com>
 */

#include <stdio.h>
#include "contiki.h"
#include "contiki-net.h"
#include "dev/leds.h"
#include "dev/button.h"
#include "dev/cc2500.h"

/* time between transmissions */
#define TRANSMISSION_INTERVAL     (CLOCK_SECOND/2)

/* Channel; any number from 129..65535 (0..128 are reserved) */
#define BROADCAST_CH              2674

#define MSGMIN  1
#define MSGMAX  30
static const uint8_t msg[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
     14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};
static uint8_t msglen = MSGMIN;
/*---------------------------------------------------------------------------*/
PROCESS(hello_process, "Hello process");
PROCESS(blink_process, "Blink process");
AUTOSTART_PROCESSES(&hello_process);
/*---------------------------------------------------------------------------*/
/* Broadcast receive callback; this is invoked on each broadcast received on the
Rime-channel associated to this broadcast connection, here BROADCAST_CH */

static void
bcr(struct broadcast_conn *c, const rimeaddr_t *f)
{
  uint8_t *buf;
  /* this is unsafe as it opens up for a smash the stack attack, but that's
    quite unlikely to happen in this context. Should sanitize input and have
    an upper bound on size, now it will just print happily until first 0. */
/*  printf("[%u] Received fr %u.%u:%s\n", clock_seconds(), f->u8[0], f->u8[1], buf);*/
  leds_toggle(LEDS_RED);
}
/*---------------------------------------------------------------------------*/
/* the broadcast connection */
static struct broadcast_conn bc;

/* the callback struct; first *receive*-callback, then *sent*-callback */
static struct broadcast_callbacks bccb = {bcr, NULL};
/*---------------------------------------------------------------------------*/
/* This process holds a broadcast connection and transmits a periodic message */

static struct etimer transmission_et;
PROCESS_THREAD(hello_process, ev, data)
{
  PROCESS_POLLHANDLER();
  PROCESS_EXITHANDLER(broadcast_close(&bc););
  PROCESS_BEGIN();

  /* open a connection on a specified channel, much ports in TCP/UDP */
  broadcast_open(&bc, BROADCAST_CH, &bccb);
  if(rimeaddr_node_addr.u8[0] == 2) {
    process_start(&blink_process, NULL);
  }

  while(1) {
    etimer_set(&transmission_et, TRANSMISSION_INTERVAL);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&transmission_et));
    if(msglen < MSGMAX) {
      msglen++;
    } else {
      msglen = MSGMIN;
    }
/*    leds_toggle(LEDS_RED);*/
/* XXX for now, just tx one size packets */
/*    packetbuf_copyfrom(msg, 9);   */
/*    packetbuf_copyfrom(msg, msglen);*/
/*    broadcast_send(&bc);*/
    if(rimeaddr_node_addr.u8[0] == 2) {
      cc2500_send(msg, 10);
    }
/*    cc2500_send(msg, msglen);*/
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
static struct etimer et;
PROCESS_THREAD(blink_process, ev, data)
{
  PROCESS_POLLHANDLER();
  PROCESS_EXITHANDLER();
  PROCESS_BEGIN();
  while(1) {
/*    leds_toggle(LEDS_ALL);*/
    etimer_set(&et, CLOCK_SECOND/8);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
  }
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

