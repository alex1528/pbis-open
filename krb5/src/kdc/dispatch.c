/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * kdc/dispatch.c
 *
 * Copyright 1990, 2009 by the Massachusetts Institute of Technology.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 *
 * Dispatch an incoming packet.
 */

#include "k5-int.h"
#include <syslog.h>
#include "kdc_util.h"
#include "extern.h"
#include "adm_proto.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

static krb5_int32 last_usec = 0, last_os_random = 0;

static krb5_error_code make_too_big_error (krb5_data **out);

krb5_error_code
dispatch(void *cb, struct sockaddr *local_saddr, const krb5_fulladdr *from,
         krb5_data *pkt, krb5_data **response, int is_tcp)
{

    krb5_error_code retval;
    krb5_kdc_req *as_req;
    krb5_int32 now, now_usec;

    /* decode incoming packet, and dispatch */

#ifndef NOCACHE
    /* try the replay lookaside buffer */
    if (kdc_check_lookaside(pkt, response)) {
        /* a hit! */
        const char *name = 0;
        char buf[46];

        if (is_tcp == 0 && (*response)->length > max_dgram_reply_size)
            goto too_big_for_udp;

        name = inet_ntop (ADDRTYPE2FAMILY (from->address->addrtype),
                          from->address->contents, buf, sizeof (buf));
        if (name == 0)
            name = "[unknown address type]";
        krb5_klog_syslog(LOG_INFO,
                         "DISPATCH: repeated (retransmitted?) request from %s, resending previous response",
                         name);
        return 0;
    }
#endif
    retval = krb5_crypto_us_timeofday(&now, &now_usec);
    if (retval == 0) {
        krb5_int32 usec_difference = now_usec-last_usec;
        krb5_data data;
        if(last_os_random == 0)
            last_os_random = now;
        /* Grab random data from OS every hour*/
        if(now-last_os_random >= 60*60) {
            krb5_c_random_os_entropy(kdc_context, 0, NULL);
            last_os_random = now;
        }

        data.length = sizeof(krb5_int32);
        data.data = (void *) &usec_difference;

        krb5_c_random_add_entropy(kdc_context,
                                  KRB5_C_RANDSOURCE_TIMING, &data);
        last_usec = now_usec;
    }
    /* try TGS_REQ first; they are more common! */

    if (krb5_is_tgs_req(pkt)) {
        retval = process_tgs_req(pkt, from, response);
    } else if (krb5_is_as_req(pkt)) {
        if (!(retval = decode_krb5_as_req(pkt, &as_req))) {
            /*
             * setup_server_realm() sets up the global realm-specific data
             * pointer.
             * process_as_req frees the request if it is called
             */
            if (!(retval = setup_server_realm(as_req->server))) {
                retval = process_as_req(as_req, pkt, from, response);
            }
            else            krb5_free_kdc_req(kdc_context, as_req);
        }
    }
    else
        retval = KRB5KRB_AP_ERR_MSG_TYPE;
#ifndef NOCACHE
    /* put the response into the lookaside buffer */
    if (!retval)
        kdc_insert_lookaside(pkt, *response);
#endif

    if (is_tcp == 0 && *response != NULL &&
        (*response)->length > max_dgram_reply_size) {
    too_big_for_udp:
        krb5_free_data(kdc_context, *response);
        retval = make_too_big_error(response);
        if (retval) {
            krb5_klog_syslog(LOG_ERR,
                             "error constructing KRB_ERR_RESPONSE_TOO_BIG error: %s",
                             error_message(retval));
        }
    }

    return retval;
}

static krb5_error_code
make_too_big_error (krb5_data **out)
{
    krb5_error errpkt;
    krb5_error_code retval;
    krb5_data *scratch;

    *out = NULL;
    memset(&errpkt, 0, sizeof(errpkt));

    retval = krb5_us_timeofday(kdc_context, &errpkt.stime, &errpkt.susec);
    if (retval)
        return retval;
    errpkt.error = KRB_ERR_RESPONSE_TOO_BIG;
    errpkt.server = tgs_server;
    errpkt.client = NULL;
    errpkt.text.length = 0;
    errpkt.text.data = 0;
    errpkt.e_data.length = 0;
    errpkt.e_data.data = 0;
    scratch = malloc(sizeof(*scratch));
    if (scratch == NULL)
        return ENOMEM;
    retval = krb5_mk_error(kdc_context, &errpkt, scratch);
    if (retval) {
        free(scratch);
        return retval;
    }

    *out = scratch;
    return 0;
}
