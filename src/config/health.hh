#pragma once

#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "events/proxy_send.hh"
#include "config/config.hh"
#include "error/not-implemented.hh"
#include "events/event-loop.hh"
#include "events/register.hh"
#include "events/server.hh"
#include "misc/openssl/ssl.hh"
#include "request/response.hh"
#include "vhost/apm.hh"
#include "vhost/vhost-factory.hh"

static void check_alive(std::shared_ptr<VHost> vhost);
