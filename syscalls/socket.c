/*
 * SYSCALL_DEFINE3(socket, int, family, int, type, int, protocol)
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "compat.h"
#include "log.h"
#include "net.h"
#include "random.h"
#include "sanitise.h"
#include "shm.h"
#include "config.h"
#include "params.h"
#include "trinity.h"

struct socket_ptr {
	unsigned int family;
	void (*func)(struct proto_type *pt);
};
static const struct socket_ptr socketptrs[] = {
	{ .family = AF_APPLETALK, .func = &atalk_rand_socket },
	{ .family = AF_AX25, .func = &ax25_rand_socket },
#ifdef USE_CAIF
	{ .family = AF_CAIF, .func = &caif_rand_socket },
#endif
	{ .family = AF_CAN, .func = &can_rand_socket },
	{ .family = AF_DECnet, .func = &decnet_rand_socket },
	{ .family = AF_INET, .func = &inet_rand_socket },
	{ .family = AF_INET6, .func = &inet6_rand_socket },
	{ .family = AF_IPX, .func = &ipx_rand_socket },
	{ .family = AF_IRDA, .func = &irda_rand_socket },
	{ .family = AF_LLC, .func = &llc_rand_socket },
	{ .family = AF_NETLINK, .func = &netlink_rand_socket },
	{ .family = AF_NFC, .func = &nfc_rand_socket },
//TODO	{ .family = AF_IB, .func = &ib_rand_socket },
	{ .family = AF_PACKET, .func = &packet_rand_socket },
	{ .family = AF_PHONET, .func = &phonet_rand_socket },
	{ .family = AF_RDS, .func = &rds_rand_socket },
	{ .family = AF_TIPC, .func = &tipc_rand_socket },
	{ .family = AF_UNIX, .func = &unix_rand_socket },
	{ .family = AF_X25, .func = &x25_rand_socket },
};

/* note: also called from generate_sockets() & sanitise_socketcall() */
void sanitise_socket(int childno)
{
	unsigned int family;
	struct proto_type pt = { .protocol = 0, .type = 0 };

	if (do_specific_proto == TRUE)
		family = specific_proto;
	else
		family = rand() % TRINITY_PF_MAX;

	if (rand() % 100 > 0) {
		unsigned int i;
		for (i = 0; i < ARRAY_SIZE(socketptrs); i++) {
			if (socketptrs[i].family == family)
				socketptrs[i].func(&pt);
		}

	} else {
		pt.protocol = rand() % PROTO_MAX;

		switch (rand() % 6) {
		case 0:	pt.type = SOCK_DGRAM;	break;
		case 1:	pt.type = SOCK_STREAM;	break;
		case 2:	pt.type = SOCK_SEQPACKET;	break;
		case 3:	pt.type = SOCK_RAW;	break;
		case 4:	pt.type = SOCK_RDM;	break;
		case 5:	pt.type = SOCK_PACKET;	break;
		default: break;
		}
	}

	if ((rand() % 100) < 25)
		pt.type |= SOCK_CLOEXEC;
	if ((rand() % 100) < 25)
		pt.type |= SOCK_NONBLOCK;

	shm->a1[childno] = family;
	shm->a2[childno] = pt.type;
	shm->a3[childno] = pt.protocol;
}

struct syscall syscall_socket = {
	.name = "socket",
	.num_args = 3,
	.arg1name = "family",
	.arg2name = "type",
	.arg3name = "protocol",
	.sanitise = sanitise_socket,
};
