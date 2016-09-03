/*
 * copyright 2016 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

//#define NDEBUG

#include <ac_inet_link.h>
#include <ac_inet_hton.h>
#include <ac_inet_ntoh.h>
#include <ac_arp.h>
#include <ac_ether.h>

#include <ac_assert.h>
#include <ac_comp_mgr.h>
#include <ac_debug_printf.h>
#include <ac_printf.h>
#include <ac_memset.h>
#include <ac_memcpy.h>
#include <ac_msg.h>
#include <ac_msg_pool.h>
#include <ac_string.h>
#include <ac_status.h>

#define _DEFAULT_SOURCE // Needed for struct ether_arp

#include <errno.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>

#define __USE_GNU // Be sure we get TEMP_FAILURE_RETRY defined
#include <unistd.h>

typedef struct {
  AcComp comp;
  ac_u32 a_u32;
  AcInt fd;                           ///< File descriptor for the interface
  AcU32 ifname_idx;                   ///< Index to the current ifname[ifname_idx]
  char* ifname[2];                    ///< Name of interfaces to try
  AcUint ifindex;                     ///< Index of the interface
  AcU8 ifmac_addr[AC_ETHER_ADDR_LEN]; ///< Mac address for this interface
  struct sockaddr_in ifipv4_addr;     ///< IPV4 address for this interface
  AcU8 padding[AC_ETHER_MIN_LEN];     ///< Padding for a packet contents are zero's
} AcCompIpv4LinkLayer;

/**
 * Display memory
 */
void ac_print_mem(char* leader, void *mem, int len, char* format, char sep, char* trailer) {
  if (leader != AC_NULL) {
    ac_printf(leader);
  }
  unsigned char* p = (unsigned char*)mem;

  for (int i = 0; i < len; i++) {
    if (i != 0) ac_printf("%c", sep);
    ac_printf(format, p[i]);
  }

  if (trailer != AC_NULL) {
    ac_printf(trailer);
  }
}

/**
 * Display hex memory
 */
void ac_println_hex(char* leader, void *mem, int len, char sep) {
  ac_print_mem(leader, mem, len, "%x", sep, "\n");
}

/**
 * Display hex memory
 */
void ac_println_dec(char* leader, void *mem, int len, char sep) {
  ac_print_mem(leader, mem, len, "%d", sep, "\n");
}

void  ac_println_sockaddr_ll(char* leader, struct sockaddr_ll* addr) {
  if (leader != AC_NULL) {
    ac_printf(leader);
  }
  ac_printf("family=%d protocol=0x%x ifindex=%d hatype=%d pkttype=%d halen=%d addr=",
      addr->sll_family, AC_NTOH_U16(addr->sll_protocol), addr->sll_ifindex,
      addr->sll_hatype, addr->sll_pkttype, addr->sll_halen);
  ac_println_hex(AC_NULL, addr->sll_addr, addr->sll_halen, ':');
}

/**
 * Set ifr_name
 *
 * @return 0 if OK
 */
AcStatus set_ifname(struct ifreq* ifr, const char* ifname) {
  AcStatus status;

  // Copy ifname to ifr.ifr_name if it fits
  if (ac_strlen(ifname) >= (sizeof(ifr->ifr_name) - 1)) {
    status = AC_STATUS_BAD_PARAM;
    goto done;
  }
  ac_strcpy(ifr->ifr_name, ifname);

  status = AC_STATUS_OK;

done:
  return status;
}

/**
 * Get index for ifname
 *
 * @return 0 if OK
 */
AcStatus get_ifindex(AcInt fd, const char* ifname, AcUint* ifindex) {
  AcStatus status;
  struct ifreq ifr;

  status = set_ifname(&ifr, ifname);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Issue iotcl to get the index
  if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
    status = AC_STATUS_ERR;
    goto done;
  }

  *ifindex = ifr.ifr_ifindex;

  status = AC_STATUS_OK;

done:
  return status;
}

/**
 * Get MAC address of NIC in network order
 *
 * @return 0 if OK
 */
AcStatus get_ethernet_mac_addr(int fd, const char* ifname, AcU8 mac_addr[AC_ETHER_ADDR_LEN]) {
  AcStatus status;
  struct ifreq ifr;

  status = set_ifname(&ifr, ifname);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Issue iotcl to get the index
  if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
    status = AC_STATUS_ERR;
    goto done;
  }

#if 0
  ac_println_hex("ifr.ifr_hwaddr.sa_data=", &ifr.ifr_hwaddr.sa_data, sizeof(ifr.ifr_hwaddr.sa_data), ':');
#endif

  ac_memcpy(mac_addr, &ifr.ifr_hwaddr.sa_data, ETH_ALEN);

  status = AC_STATUS_OK;

done:
  return status;
}

/**
 * Get IPV4 address of NIC in network order
 *
 * @return 0 if OK
 */
AcStatus get_ethernet_ipv4_addr(int fd, const char* ifname, struct sockaddr_in* ipv4_addr) {
  AcStatus status;
  struct ifreq ifr;

  status = set_ifname(&ifr, ifname);
  if (status != AC_STATUS_OK) {
    goto done;
  }

  // Issue iotcl to get the index
  if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
    status = AC_STATUS_ERR;
    goto done;
  }

  // Be sure address fits
  ac_static_assert(sizeof(*ipv4_addr) <= sizeof(ifr.ifr_addr),
    "struct sockaddr_in larger than ifr.ifr_addr");

  // Get ipv4 address
  ac_memcpy(ipv4_addr, &ifr.ifr_addr, sizeof(*ipv4_addr));

  status = AC_STATUS_OK;

done:
  return status;
}



/**
 * Initialize a struct ether_arp
 *
 * @param fd is the socket file descriptro
 * @param ifname is the name of the interface
 * @param ipv4_addr_str is a ipv4 dotted decimal address
 * @param pArpReq is the ether_arp to initialize
 *
 * @return 0 if OK
 */
void init_ether_arp(AcCompIpv4LinkLayer* this, struct ether_arp* pArpReq,
    AcU16 protocol, AcU32 proto_addr_len, AcU8* proto_addr) {

  // Initialize ethernet arp request
  pArpReq->arp_hrd = AC_HTON_U16(ARPHRD_ETHER);
  pArpReq->arp_pro = AC_HTON_U16(protocol);
  pArpReq->arp_hln = AC_ETHER_ADDR_LEN;
  pArpReq->arp_pln = proto_addr_len;
  pArpReq->arp_op = AC_HTON_U16(ARPOP_REQUEST);

  // Copy the proto_addr to target protocol address (tpa)
  ac_memcpy(pArpReq->arp_tpa, proto_addr, proto_addr_len);

  // Zero the target hardware address (arp_tha)
  ac_memset(&pArpReq->arp_tha, 0, sizeof(pArpReq->arp_tha));

  // Get Source hardware address to arp source hardware address (arp_sha)
  ac_memcpy(pArpReq->arp_sha, this->ifmac_addr, pArpReq->arp_hln);

  // Get source ipv4 address to arp source protocol address (arp_spa)
  ac_memcpy(pArpReq->arp_spa, &this->ifipv4_addr.sin_addr, AC_IPV4_ADDR_LEN);
}

/**
 * init a struct ethhdr
 *
 * @param dst_addr
 */
void init_ethhdr(struct ethhdr* pEthHdr, const void *dst_addr, const void* src_addr, int ll_protocol) {
  ac_memcpy(pEthHdr->h_dest, dst_addr, AC_ETHER_ADDR_LEN);
  ac_memcpy(pEthHdr->h_source, src_addr, AC_ETHER_ADDR_LEN);
  pEthHdr->h_proto = ll_protocol;
}

/**
 * Init sockaddr_ll
 */
void init_sockaddr_ll(struct sockaddr_ll* pSockAddrLl,
    const int addr_len,
    const unsigned char* addr,
    const int ifindex,
    const int protocol) {
  ac_memset(pSockAddrLl, 0, sizeof(struct sockaddr_ll));

  pSockAddrLl->sll_family = AF_PACKET;
  pSockAddrLl->sll_protocol = AC_HTON_U16(protocol);
  pSockAddrLl->sll_ifindex = ifindex;
  pSockAddrLl->sll_halen = addr_len;
  ac_memcpy(pSockAddrLl->sll_addr, addr, addr_len);
}

/**
 * Init ethernet broadcast sockaddr_ll
 */
void init_ether_broadcast_sockaddr_ll(struct sockaddr_ll* pSockAddrLl, const int ifindex, const AcU16 protocol) {
  unsigned char ethernet_broadcast_addr[AC_ETHER_ADDR_LEN] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff
  };

  init_sockaddr_ll(pSockAddrLl, AC_ETHER_ADDR_LEN, ethernet_broadcast_addr, ifindex, protocol);
}

AcStatus send_arp(AcCompIpv4LinkLayer* this, AcU16 protocol, AcU32 proto_addr_len, AcU8* proto_addr) {
  ac_printf("%s:+send_arp proto=%u proto_addr_len=%u", this->comp.name, protocol, proto_addr_len);
  ac_println_dec(" proto_addr=", proto_addr, proto_addr_len, '.');

  AcStatus status;
  struct sockaddr_ll dst_addr;

  // The destination is an ethenet broadcast address
  init_ether_broadcast_sockaddr_ll(&dst_addr, this->ifindex, AC_ETHER_PROTO_ARP);
  ac_println_sockaddr_ll("send_ethernet_arp_ipv4: ", &dst_addr);

  // Initialize ethernet arp request
  struct ether_arp arp_req;
  init_ether_arp(this, &arp_req, protocol, proto_addr_len, proto_addr);

  // Initialize ethernet header
  struct ethhdr ether_hdr;
  init_ethhdr(&ether_hdr, &dst_addr, &arp_req.arp_sha, dst_addr.sll_protocol);

  // Initialize iovec for msghdr
  struct iovec iov[3];
  int iovlen = 2;
  iov[0].iov_base = &ether_hdr;
  iov[0].iov_len = sizeof(ether_hdr);
  iov[1].iov_base = &arp_req;
  iov[1].iov_len = sizeof(arp_req);

  int len = sizeof(ether_hdr) + sizeof(arp_req);
  if (len < AC_ETHER_MIN_LEN) {
    // We need to padd out the frame to 60 bytes
    // so add an extr iov;
    iov[iovlen].iov_base = this->padding;
    iov[iovlen].iov_len = AC_ETHER_MIN_LEN - len;
    iovlen += 1;
  }

  // Initialize msghdr
  struct msghdr mh;
  mh.msg_name = &dst_addr;
  mh.msg_namelen = sizeof(dst_addr);
  mh.msg_iov = iov;
  mh.msg_iovlen = iovlen;
  mh.msg_control = NULL;
  mh.msg_controllen = 0;
  mh.msg_flags = 0;

  //println_hex("send_ethernet_arp_ipv4: mh=", &packet, len, ' ');
  int count = sendmsg(this->fd, &mh, 0);
  if (count < 0) {
    status = AC_STATUS_ERR;
  } else {
    status = AC_STATUS_OK;
  }
  ac_printf("%s:-send_arp sent count=%d status=%u\n", this->comp.name, count, status);
  return status;
}

static void send_error_rsp(AcComp* comp, AcMsg* msg, AcStatus status) {
}

static ac_bool comp_ipv4_ll_process_msg(AcComp* comp, AcMsg* msg) {
  AcStatus status;
  AcCompIpv4LinkLayer* this = (AcCompIpv4LinkLayer*)comp;
  AC_UNUSED(this);

  ac_debug_printf("%s:+msg->op=%lx\n", this->comp.name, msg->op);

  switch (msg->op) {
    case (AC_INIT_CMD): {
      ac_debug_printf("%s: AC_INIT_CMD ifname=%s\n", this->comp.name, this->ifname[0]);
      // Open an AF_PACKET socket
      this->fd = socket(AF_PACKET, SOCK_RAW, AC_HTON_U16(ETH_P_ALL));
      if (this->fd < 0) {
        AcU8 str[256];
        ac_snprintf(str, sizeof(str), "%s: Could not open SOCK_RAW errno=%u\n", this->comp.name, errno);
        ac_fail((char*)str);
      }

      // Get the interface index
      status = get_ifindex(this->fd, this->ifname[0], &this->ifindex);
      if (status != AC_STATUS_OK) {
        ac_printf("%s: Could not get interface index for ifname=%s errno=%d\n", this->comp.name, this->ifname[0], errno);
        status = get_ifindex(this->fd, this->ifname[1], &this->ifindex);
        if (status != AC_STATUS_OK) {
          AcU8 str[256];
          ac_snprintf(str, sizeof(str),
                "%s: Could not get interface index for ifname=%s errno=%d\n", this->comp.name, this->ifname[1], errno);
          ac_fail((char*)str);
        }
      }

      // Get the mac address for our interface
      status = get_ethernet_mac_addr(this->fd, this->ifname[this->ifname_idx], this->ifmac_addr);
      if (status != AC_STATUS_OK) {
        AcU8 str[256];
        ac_snprintf(str, sizeof(str), "%s: Could get interface mac address status=%u\n", this->comp.name, status);
        ac_fail((char*)str);
      }
      ac_printf("%s: ifmac_addr=", this->comp.name);
      ac_println_hex("send_ethernet_arp_ipv4: pArpReq->arp_sha=",
          this->ifmac_addr, sizeof(this->ifmac_addr), ':');

      // Get the ipv4 address for our interface
      status = get_ethernet_ipv4_addr(this->fd, this->ifname[this->ifname_idx], &this->ifipv4_addr);
      if (status != AC_STATUS_OK) {
        AcU8 str[256];
        ac_snprintf(str, sizeof(str), "%s: Could get interface ipv4 address status=%u\n", this->comp.name, status);
        ac_fail((char*)str);
      }
      ac_printf("%s: ", this->comp.name);
      ac_println_dec("ifipv4_addr.sin_addr=", &this->ifipv4_addr.sin_addr, AC_IPV4_ADDR_LEN, '.');

      // Initialize padding array
      ac_memset(this->padding, 0, sizeof(this->padding));

      break;
    }
    case (AC_DEINIT_CMD): {
      TEMP_FAILURE_RETRY (close(this->fd));
      ac_debug_printf("%s: AC_DEINIT_CMD\n", this->comp.name);
      break;
    }
    case (AC_INET_SEND_ARP_CMD): {
      AcInetSendArpExtra* send_arp_extra = (AcInetSendArpExtra*)msg->extra;
      ac_printf("%s: AC_INET_SEND_ARP_CMD proto=%x", this->comp.name, send_arp_extra->proto);
      ac_println_dec(" proto_addr=", send_arp_extra->proto_addr, send_arp_extra->proto_addr_len, '.');

      send_arp(this, send_arp_extra->proto, send_arp_extra->proto_addr_len, send_arp_extra-> proto_addr);

      break;
    }
    case (AC_INET_SEND_PACKET_CMD): {
      ac_debug_printf("%s: AC_INET_SEND_PACKET_CMD\n", this->comp.name);
      break;
    }
    default: {
      ac_debug_printf("%s: AC_STATUS_UNRECOGNIZED_PROTOCOL send error rsp\n", this->comp.name);
      send_error_rsp(comp, msg, AC_STATUS_UNRECOGNIZED_PROTOCOL);
      break;
    }
  }

  AcMsgPool_ret_msg(msg);

  ac_debug_printf("%s:-msg->op=%lx\n", this->comp.name, msg->op);
  return AC_TRUE;
}

static AcCompIpv4LinkLayer comp_ipv4_ll = {
  .comp.name=(ac_u8*)INET_LINK_COMP_IPV4_NAME,
  .comp.process_msg = comp_ipv4_ll_process_msg,
  .ifname_idx = 0,
  .ifname[0] = "eno1",
  .ifname[1] = "eth0"
};

/**
 * see ac_inet_link_internal.h
 */
void AcInetLink_deinit(AcCompMgr* cm) {
  ac_debug_printf("AcInetLink_deinit:+cm=%p\n", cm);

  ac_assert(AcCompMgr_rmv_comp(&comp_ipv4_ll.comp) == AC_STATUS_OK);

  ac_debug_printf("AcInetLink_deinit:-cm=%p\n", cm);
}

/**
 * see ac_inet_link.h
 */
void AcInetLink_init(AcCompMgr* cm) {
  ac_debug_printf("AcInetLink_init:+cm=%p\n", cm);

  ac_assert(AcCompMgr_add_comp(cm, &comp_ipv4_ll.comp) == AC_STATUS_OK);

  ac_debug_printf("AcInetLink_init:-cm=%p\n", cm);
}
