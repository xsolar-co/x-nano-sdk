//
// Copyright 2024 NanoMQ Team, Inc. <wangwei@emqx.io>
//
// This software is supplied under the terms of the MIT License, a
// copy of which should be located in the distribution where this
// file was obtained (LICENSE.txt).  A copy of the license may also be
// found online at https://opensource.org/licenses/MIT.
//

#include "iceoryx_api.h"

#include "nng/nng.h"
#include "core/nng_impl.h"

#include "iceoryx_binding_c/listener.h"
#include "iceoryx_binding_c/runtime.h"
#include "iceoryx_binding_c/subscriber.h"
#include "iceoryx_binding_c/publisher.h"
#include "iceoryx_binding_c/types.h"
#include "iceoryx_binding_c/user_trigger.h"

struct nano_iceoryx_suber {
	iox_listener_t listener;
	iox_sub_t      suber;
};

struct nano_iceoryx_puber {
	iox_pub_t      puber;
};

// Event is the topic you wanna read
int
nano_iceoryx_init(const char *const name)
{
    iox_runtime_init(name); // No related to subscriber or publisher. just a runtime name
	return 0;
}

int
nano_iceoryx_fini()
{
	iox_runtime_shutdown();
	return 0;
}

void
nano_iceoryx_listener_alloc(nano_iceoryx_listener **listenerp)
{
    iox_listener_storage_t listener_storage;
    iox_listener_t listener = iox_listener_init(&listener_storage);

	*listenerp = listener;
}

void
nano_iceoryx_listener_free(nano_iceoryx_listener *listener)
{
    iox_listener_deinit((iox_listener_t)listener);
}

static void
suber_recv_cb(iox_sub_t subscriber)
{
	NNI_ARG_UNUSED(subscriber);
}

nano_iceoryx_suber *
nano_iceoryx_suber_alloc(const char *subername, const char *const service_name,
    const char *const instance_name, const char *const event,
    nano_iceoryx_listener *lstner)
{
	iox_listener_t listener = lstner;
	nano_iceoryx_suber *suber = nng_alloc(sizeof(*suber));
	if (!suber)
		return NULL;

    iox_sub_options_t options;
    iox_sub_options_init(&options);
    options.historyRequest = 10U;
    options.queueCapacity = 50U;
    options.nodeName = subername;

    iox_sub_storage_t subscriber_storage;
    iox_sub_t subscriber = iox_sub_init(&subscriber_storage, service_name,
	    instance_name, event, &options);

    iox_listener_attach_subscriber_event(
        (iox_listener_t)listener, subscriber, SubscriberEvent_DATA_RECEIVED, &suber_recv_cb);

	suber->listener = listener;
	suber->suber    = subscriber;

	return suber;
}

void
nano_iceoryx_suber_free(nano_iceoryx_suber *suber)
{
    iox_listener_detach_subscriber_event(suber->listener, suber->suber,
	        SubscriberEvent_DATA_RECEIVED);
    iox_sub_deinit(suber->suber);
	nng_free(suber, sizeof(*suber));
}

nano_iceoryx_puber *
nano_iceoryx_puber_alloc(const char *pubername, const char *const service_name,
    const char *const instance_name, const char *const event)
{
	nano_iceoryx_puber *puber = nng_alloc(sizeof(*puber));
	if (!puber)
		return NULL;

    iox_pub_options_t options;
    iox_pub_options_init(&options);
    options.historyCapacity = 10U;
    options.nodeName = pubername;

    iox_pub_storage_t publisher_storage;

    iox_pub_t publisher = iox_pub_init(&publisher_storage, service_name,
		instance_name, event, &options);

	puber->puber = publisher;
	return puber;
}

void
nano_iceoryx_puber_free(nano_iceoryx_puber *puber)
{
	nng_free(puber, sizeof(*puber));
}


