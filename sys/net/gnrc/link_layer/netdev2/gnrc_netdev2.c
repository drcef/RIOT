/*
 * Copyright (C) 2015 Freie Universität Berlin
 *               2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 * @ingroup     net
 * @file
 * @brief       Glue for netdev2 devices to netapi
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @}
 */

#include <errno.h>

#include "msg.h"
#include "thread.h"
#include "xtimer.h"

#include "net/gnrc.h"
#include "net/gnrc/nettype.h"
#include "net/netdev2.h"

#include "net/gnrc/netdev2.h"
#include "net/ethernet/hdr.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

#if defined(MODULE_OD) && ENABLE_DEBUG
#include "od.h"
#endif

#define CCA_COUNT_MAX           2
#define RADIO_WAKEUP_DELAY      300

#define CCA_INTERVAL            690
#define CCA_MEASUREMENT_TIME    135

#define MAX_NONACTIVITY_PERIODS 10

/* The max listen time after a CCA has detected a packet
 * will be twice the time it takes to send the largest 802.15.4 packet
 * (127 bytes * 32 us per byte = 4064 us) plus t_i, the interval
 * between packets we have chosen, 600 us
 */
#define MAX_LISTEN_TIME_AFTER_PACKET_DETECTED   8728

#define NETDEV2_NETAPI_MSG_QUEUE_SIZE 8

static volatile uint8_t pending_packet;

static void _pass_on_packet(gnrc_pktsnip_t *pkt);

/**
 * @brief   Function called by the device driver on device events
 *
 * @param[in] event     type of event
 */
static void _event_cb(netdev2_t *dev, netdev2_event_t event)
{
    gnrc_netdev2_t *gnrc_netdev2 = (gnrc_netdev2_t*) dev->context;

    if (event == NETDEV2_EVENT_ISR) {
        msg_t msg;

        msg.type = NETDEV2_MSG_TYPE_EVENT;
        msg.content.ptr = gnrc_netdev2;

        if (msg_send(&msg, gnrc_netdev2->pid) <= 0) {
            puts("gnrc_netdev2: possibly lost interrupt.");
        }
    }
    else {
        DEBUG("gnrc_netdev2: event triggered -> %i\n", event);
        switch(event) {
            case NETDEV2_EVENT_RX_COMPLETE:
                {
                    // pending_packet = 1;
                    gnrc_pktsnip_t *pkt = gnrc_netdev2->recv(gnrc_netdev2);

                    if (pkt) {
                        _pass_on_packet(pkt);
                    }
                    
                    pending_packet = 1;
                    
                    break;
                }
#ifdef MODULE_NETSTATS_L2
            case NETDEV2_EVENT_TX_MEDIUM_BUSY:
                dev->stats.tx_failed++;
                break;
            case NETDEV2_EVENT_TX_COMPLETE:
                dev->stats.tx_success++;
                break;
#endif
            default:
                DEBUG("gnrc_netdev2: warning: unhandled event %u.\n", event);
        }
    }
}

static void _pass_on_packet(gnrc_pktsnip_t *pkt)
{
    /* throw away packet if no one is interested */
    if (!gnrc_netapi_dispatch_receive(pkt->type, GNRC_NETREG_DEMUX_CTX_ALL, pkt)) {
        DEBUG("gnrc_netdev2: unable to forward packet of type %i\n", pkt->type);
        gnrc_pktbuf_release(pkt);
        return;
    }
}

/**
 * @brief   Startup code and event loop of the gnrc_netdev2 layer
 *
 * @param[in] args  expects a pointer to the underlying netdev device
 *
 * @return          never returns
 */
static void *_gnrc_netdev2_thread(void *args)
{
    DEBUG("gnrc_netdev2: starting thread\n");

    gnrc_netdev2_t *gnrc_netdev2 = (gnrc_netdev2_t*) args;
    netdev2_t *dev = gnrc_netdev2->dev;

    gnrc_netdev2->pid = thread_getpid();

    gnrc_netapi_opt_t *opt;
    int res;
    msg_t msg, reply, msg_queue[NETDEV2_NETAPI_MSG_QUEUE_SIZE];

    /* setup the MAC layers message queue */
    msg_init_queue(msg_queue, NETDEV2_NETAPI_MSG_QUEUE_SIZE);

    /* register the event callback with the device driver */
    dev->event_callback = _event_cb;
    dev->context = (void*) gnrc_netdev2;

    /* register the device to the network stack*/
    gnrc_netif_add(thread_getpid());

    /* initialize low-level driver */
    dev->driver->init(dev);

    /* start the event loop */
    while (1) {
        DEBUG("gnrc_netdev2: waiting for incoming messages\n");
        msg_receive(&msg);
        /* dispatch NETDEV and NETAPI messages */
        switch (msg.type) {
            case NETDEV2_MSG_TYPE_EVENT:
                DEBUG("gnrc_netdev2: GNRC_NETDEV_MSG_TYPE_EVENT received\n");
                dev->driver->isr(dev);
                break;
            case GNRC_NETAPI_MSG_TYPE_SND:
                DEBUG("gnrc_netdev2: GNRC_NETAPI_MSG_TYPE_SND received\n");
                gnrc_pktsnip_t *pkt = msg.content.ptr;
                gnrc_netdev2->send(gnrc_netdev2, pkt);
                break;
            case GNRC_NETAPI_MSG_TYPE_SET:
                /* read incoming options */
                opt = msg.content.ptr;
                DEBUG("gnrc_netdev2: GNRC_NETAPI_MSG_TYPE_SET received. opt=%s\n",
                        netopt2str(opt->opt));
                /* set option for device driver */
                res = dev->driver->set(dev, opt->opt, opt->data, opt->data_len);
                DEBUG("gnrc_netdev2: response of netdev->set: %i\n", res);
                /* send reply to calling thread */
                reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            case GNRC_NETAPI_MSG_TYPE_GET:
                /* read incoming options */
                opt = msg.content.ptr;
                DEBUG("gnrc_netdev2: GNRC_NETAPI_MSG_TYPE_GET received. opt=%s\n",
                        netopt2str(opt->opt));
                /* get option from device driver */
                res = dev->driver->get(dev, opt->opt, opt->data, opt->data_len);
                DEBUG("gnrc_netdev2: response of netdev->get: %i\n", res);
                /* send reply to calling thread */
                reply.type = GNRC_NETAPI_MSG_TYPE_ACK;
                reply.content.value = (uint32_t)res;
                msg_reply(&msg, &reply);
                break;
            default:
                DEBUG("gnrc_netdev2: Unknown command %" PRIu16 "\n", msg.type);
                break;
        }
    }
    /* never reached */
    return NULL;
}

static void radio_state_sleep(netdev2_t *dev)
{
    if(we_are_sending == 0 && we_are_receiving_burst == 0) {
        netopt_state_t* state = NETOPT_STATE_SLEEP;
        dev->driver->set(dev, NETOPT_STATE, state, sizeof(netopt_state_t));
    }
}

static void radio_state_rxaack(netdev2_t *dev)
{
    if(we_are_sending == 0 && we_are_receiving_burst == 0) {
        netopt_state_t* state = NETOPT_STATE_IDLE;
        dev->driver->set(dev, NETOPT_STATE, state, sizeof(netopt_state_t));
    }
}

static void radio_state_rxon(netdev2_t *dev)
{
    if(we_are_sending == 0 && we_are_receiving_burst == 0) {
        netopt_state_t* state = NETOPT_STATE_IDLE_BASIC;
        dev->driver->set(dev, NETOPT_STATE, state, sizeof(netopt_state_t));
    }
}

// Don't do this if the radio is operating in extended mode
static netopt_cca_state_t cca_check(netdev2_t *dev)
{
    netopt_cca_state_t cca_state;
    dev->driver->get(dev, NETOPT_CCA_STATE, &cca_state, sizeof(netopt_cca_state_t));
    return cca_state;
}

static void *_gnrc_contikiMAC_thread(void *args)
{
    DEBUG("gnrc_contikiMAC: starting thread\n");

    gnrc_netdev2_t *gnrc_netdev2 = (gnrc_netdev2_t*) args;
    netdev2_t *dev = gnrc_netdev2->dev;

    uint64_t cycle_start = xtimer_now_usec64();
    while(1) {
        netopt_state_t radio_state;
        uint8_t packet_seen;
        uint8_t count;

        packet_seen = 0;

        for (count = 0; count < CCA_COUNT_MAX; count++) {
            dev->driver->get(dev, NETOPT_STATE, &radio_state, sizeof(netopt_state_t));
            if ((radio_state != NETOPT_STATE_RX) && (radio_state != NETOPT_STATE_TX)) {
                radio_state_rxon(dev); // waking the radio from sleep takes ~300 us
                if (cca_check(dev) == NETOPT_CCA_STATE_BUSY) { // cca takes ~135 us
                    packet_seen = 1;
                    break;
                }
                radio_state_sleep(dev);
            }
            xtimer_usleep(CCA_INTERVAL - RADIO_WAKEUP_DELAY);
        }

        if (packet_seen) {
            uint8_t periods = 0;
            uint64_t start_time = xtimer_now_usec64();

            pending_packet = 0;

            // switch radio to RX_AACK mode
            radio_state_rxaack(dev);

            dev->driver->get(dev, NETOPT_STATE, &radio_state, sizeof(netopt_state_t));
            while ((radio_state != NETOPT_STATE_TX)
                && (radio_state != NETOPT_STATE_SLEEP)
                && (xtimer_now_usec64() < start_time + MAX_LISTEN_TIME_AFTER_PACKET_DETECTED)) {

                /* TODO fast sleep optimisations */

                if (pending_packet) {
                    // the packet will have been passed on in the ISR
                    // by the time we read this so we should clear it
                    pending_packet = 0;
                    break;
                }

                xtimer_usleep(CCA_INTERVAL + CCA_MEASUREMENT_TIME);
                dev->driver->get(dev, NETOPT_STATE, &radio_state, sizeof(netopt_state_t));
            }

            // if radio is on; sleep optimisations above might turn it off, once implemented
            if (radio_state != NETOPT_STATE_SLEEP) {
                if ((radio_state != NETOPT_STATE_RX)
                    && (xtimer_now_usec64() < start_time + MAX_LISTEN_TIME_AFTER_PACKET_DETECTED))
            }
        }
    }
}

kernel_pid_t gnrc_netdev2_init(char *stack, int stacksize, char priority,
                        const char *name, gnrc_netdev2_t *gnrc_netdev2)
{
    kernel_pid_t res;

    /* check if given netdev device is defined and the driver is set */
    if (gnrc_netdev2 == NULL || gnrc_netdev2->dev == NULL) {
        return -ENODEV;
    }

    /* create new gnrc_netdev2 thread */
    res = thread_create(stack, stacksize, priority, THREAD_CREATE_STACKTEST,
                         _gnrc_netdev2_thread, (void *)gnrc_netdev2, name);
    if (res <= 0) {
        return -EINVAL;
    }

    return res;
}
