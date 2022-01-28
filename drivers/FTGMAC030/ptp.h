#ifndef __PTP_H__
#define __PTP_H__


/* define Primitive PTP data types CH5.2 */
typedef int    Boolean;
typedef uint8_t  Enumeration8;
typedef uint16_t Enumeration16;
typedef int8_t   Integer8;
typedef uint8_t  UInteger8;
typedef int16_t  Integer16;
typedef uint16_t UInteger16;
typedef int  Integer32;
typedef uint32_t UInteger32;
typedef int64_t  Integer64;
typedef uint64_t UInteger64;
typedef uint8_t  Octet;

typedef Integer64 TimeInterval;

/* define Derived data types specifications CH5.3 */
struct Timestamp {
	uint16_t seconds_msb;
	uint32_t seconds_lsb;
	UInteger32 nanoseconds;
}__attribute__((packed));

struct ClockIdentity {
	Octet id[8];
}__attribute__((packed));

struct PortIdentity {
	struct ClockIdentity clockIdentity;
	UInteger16 portNumber;
}__attribute__((packed));

struct PortAddress {
	Enumeration16 networkProtocol;
	UInteger16 addressLength;
	Octet address[0];
}__attribute__((packed));

struct PhysicalAddress {
	UInteger16 length;
	Octet address[0];
}__attribute__((packed));

struct ClockQuality {
    UInteger8     clockClass;
    Enumeration8  clockAccuracy;
    UInteger16    offsetScaledLogVariance;
}__attribute__((packed));

struct TLV {
    Enumeration16 type;
    UInteger16    length; /* must be even */
    Octet         value[0];
}__attribute__((packed));

struct PTPText {
    UInteger8 length;
    Octet     text[0];
}__attribute__((packed));

#define MAX_PTP_OCTETS 255
struct static_ptp_text {
    /* null-terminated array of UTF-8 symbols */
    Octet text[MAX_PTP_OCTETS + 1];
    /* number of used bytes in text, not including trailing null */
    int length;
    /* max number of UTF-8 symbols that can be in text */
    int max_symbols;
};

struct FaultRecord {
    UInteger16       faultRecordLength;
    struct Timestamp faultTime;
    Enumeration8     severityCode;
    struct PTPText   faultName;
    struct PTPText   faultValue;
    struct PTPText   faultDescription;
};

struct ptp_header {
    uint8_t             
		messageType:4,
		transportSpecific:4;
    uint8_t           
		versionPTP:4,
		reserved:4;
    UInteger16          messageLength;
    UInteger8           domainNumber;
    Octet               reserved1;
    Octet               flagField[2];
    Integer64           correction;
    UInteger32          reserved2;
    struct PortIdentity sourcePortIdentity;
    UInteger16          sequenceId;
    UInteger8           control;
    Integer8            logMessageInterval;
}__attribute__((packed));

struct sync_msg {
	struct ptp_header hdr;
	struct Timestamp originTimestamp;
};

struct delay_req_msg {
	struct ptp_header hdr;
	struct Timestamp originTimestamp;
};

struct pdelay_req_msg {
	struct ptp_header hdr;
	struct Timestamp originTimestamp;
	struct PortIdentity reserved;
};

struct pdelay_resp_msg {
	struct ptp_header hdr;
	struct Timestamp originTimestamp;
	struct PortIdentity requestingPortIdentity;
};

struct ptp_message {
	union {
		struct ptp_header header;
		struct sync_msg sync;
		struct delay_req_msg delay_req;
		struct pdelay_req_msg pdelay_req;
		struct pdelay_resp_msg pdelay_resp;
	};
};

#define SYNC                  0x0
#define DELAY_REQ             0x1
#define PDELAY_REQ            0x2
#define PDELAY_RESP           0x3
#define FOLLOW_UP             0x8
#define DELAY_RESP            0x9
#define PDELAY_RESP_FOLLOW_UP 0xA
#define ANNOUNCE              0xB
#define SIGNALING             0xC
#define MANAGEMENT            0xD

#endif
