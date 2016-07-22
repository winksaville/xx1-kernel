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

#ifndef SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_LINK_H
#define SADIE_LIBS_AC_INET_LINK_INCS_AC_INET_LINK_H

#include <ac_inttypes.h>

/**
 * See (RFC-2460 Internet Protocol, V6)[https://tools.ietf.org/html/rfc2460]
 */
typedef struct __attribute__ ((__packed__)) AcInetIpv6Hdr {
  ac_u8 version:4;        // Version = 6
  ac_u8 traffic_class:8;  // Traffic class see (section 7)[https://tools.ietf.org/html/rfc2460#section-7]
  ac_u32 flow_label:20;   // Flow label set (section 6)[https://tools.ietf.org/html/rfc2460#section-6]
  ac_u16 payload_length;  // Includes extension headers, if any, and payload that follow this header
  ac_u8 next_header;      // 59 == no next_header, IP Protocol see (www.iana.org Protocol Numbers)[http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml]
  ac_u8 hop_limit;        // If arrives as 0 discard. Before forwarding decrement
                          // and if it becomes 0 discard and do not forward.
  ac_u8 source[16];       // 16 byte source address
  ac_u8 destination[16];  // 16 byte destination address
} AcInetIpv6Hdr;

/**
 * See (RFC-791 Internet Protocol)[https://tools.ietf.org/html/rfc791]
 */
typedef struct __attribute__ ((__packed__)) AcInetIpv4FragmentOffset {
  union {
    ac_u8 raw_u8;
    ac_u16 raw_u16;
    struct {
      ac_u16 offset_in_u64s:13; // Bits 0-12 this fragments byte offset == offset_in_u64s * 8
      ac_bool more_fragments:1; // 0 = last fragment, 1 = more fragments
      ac_bool dont_fragment:1;  // 0 = May fragment,  1 = dont' fragment
      ac_bool zero:1;           // always 0
    };
  };
} AcInetIpv4FragmentOffset;

/**
 * See (RFC-791 Internet Protocol)[https://tools.ietf.org/html/rfc791]
 */
typedef struct __attribute__((__packed__)) AcInetIpv4Hdr {
  ac_u8 version:4;         // Version = 4
  ac_u8 ihl:4;             // Interent Header Lenth
  ac_u8 tos;               // Type of service
  ac_u16 total_length;     // Total length of the datagram
  ac_u16 identification;   // sequence number
  AcInetIpv4FragmentOffset fragment_offset; // bits 0-12 are offset
                                            // bit 13 == 0 = Last Fragment, 1 = More Fragments
                                            // bit 15 == 0 always
                                            // bit 14 == 0 = May Fragment,  1 = Don't Fragment
  ac_u8 time_to_live;      // 0 == time has expired and this packet is not forwarded
  ac_u8 protocol;          // IP Protocol see (www.iana.org Protocol Numbers)[http://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml]
  ac_u16 header_checksum;  // Checksum for the header
  ac_u8 source[4];         // 4 byte source address
  ac_u8 destination[4];    // 4 byte destination address
} AcInetIpv4Hdr;

/**
 * Initialize this module
 */
void AcInetLink_init(void);

#include <ac_inet_link_impl.h>

#endif
