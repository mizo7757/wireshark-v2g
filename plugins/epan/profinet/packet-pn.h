/* packet-pn.h
 * Common functions for other PROFINET protocols like DCP, MRP, ...
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1999 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/*
 * Cyclic PNIO RTC1 Data Dissection:
 *
 * Added new structures to packet-pn.h to transfer the gained data of
 * packet-pn-dcp.c and packet-dcerpc-pn-io.c to packet-pn-rtc-one.c for
 * detailled dissection of cyclic PNIO RTC1 dataframes.
 *
 */

#define FRAME_ID_DCP_HELLO      0xfefc
#define FRAME_ID_DCP_GETORSET   0xfefd
#define FRAME_ID_DCP_IDENT_REQ  0xfefe
#define FRAME_ID_DCP_IDENT_RES  0xfeff


/* ---- Structures for pnio_rtc1 ---- */
extern int       proto_pn_dcp;
extern int proto_pn_io_apdu_status;
extern int proto_pn_io_time_aware_status;

extern gboolean  pnio_ps_selection;  /* given by pnio preferences */

/* Structure for general station information */
typedef struct tagStationInfo {
    /* general information */
    gchar    *typeofstation;
    gchar    *nameofstation;
    guint16   u16Vendor_id;
    guint16   u16Device_id;
    /* frame structure */
    guint16   ioDataObjectNr;
    guint16   iocsNr_in;
    guint16   iocsNr_out;
    /* GSDfile station information */
    gboolean  gsdFound;
    gboolean  gsdPathLength;
    gchar    *gsdLocation;
    /* IOCS object data */
    wmem_list_t *iocs_data_in;
    wmem_list_t *iocs_data_out;
    /* IOData object data */
    wmem_list_t *ioobject_data_in;
    wmem_list_t *ioobject_data_out;
    /* Different ModuleIdentnumber */
    wmem_list_t *diff_module;
} stationInfo;

typedef struct tagApduStatusSwitch
{
    gboolean isRedundancyActive;
    address dl_dst;
    address dl_src;
}apduStatusSwitch;

/* Structure for IOCS Frames */
typedef struct tagIocsObject {
    guint16    slotNr;
    guint16    subSlotNr;
    guint16    frameOffset;
} iocsObject;

/* Structure for IO Data Objects  */
typedef struct tagIoDataObject {
    guint16     slotNr;
    guint16     subSlotNr;
    guint32     moduleIdentNr;
    guint32     subModuleIdentNr;
    guint16     frameOffset;
    guint16     length;
    guint16     amountInGSDML;
    guint32     fParameterIndexNr;
    guint16     f_par_crc1;
    guint16     f_src_adr;
    guint16     f_dest_adr;
    gboolean    f_crc_seed;
    guint8      f_crc_len;
    address     srcAddr;
    address     dstAddr;
    gboolean    profisafeSupported;
    gboolean    discardIOXS;
    gchar      *moduleNameStr;
    tvbuff_t   *tvb_slot;
    tvbuff_t   *tvb_subslot;
    /* Status- or Controlbyte data*/
    guint8     last_sb_cb;
    guint8     lastToggleBit;
} ioDataObject;

/* Structure for Modules with different ModuleIdentnumber */
typedef struct tagModuleDiffInfo {
    guint16    slotNr;
    guint32    modulID;
} moduleDiffInfo;

typedef struct tagARUUIDFrame {
    e_guid_t aruuid;
    guint32  setupframe;
    guint32  releaseframe;
    guint16  outputframe;
    guint16  inputframe;
} ARUUIDFrame;

extern wmem_list_t *aruuid_frame_setup_list;

extern void init_pn(int proto);
extern void init_pn_io_rtc1(int proto);
extern void init_pn_rsi(int proto);
extern void pn_rsi_reassemble_init(void);

extern void init_pnio_rtc1_station(stationInfo *station_info);

extern int dissect_pn_uint8(tvbuff_t *tvb, gint offset, packet_info *pinfo,
                  proto_tree *tree, int hfindex, guint8 *pdata);

extern int dissect_pn_uint16_ret_item(tvbuff_t *tvb, gint offset, packet_info *pinfo _U_,
                       proto_tree *tree, int hfindex, guint16 *pdata, proto_item ** new_item);
extern int dissect_pn_uint16(tvbuff_t *tvb, gint offset, packet_info *pinfo,
                       proto_tree *tree, int hfindex, guint16 *pdata);

extern int dissect_pn_int16(tvbuff_t *tvb, gint offset, packet_info *pinfo,
                       proto_tree *tree, int hfindex, gint16 *pdata);

extern int dissect_pn_oid(tvbuff_t *tvb, int offset, packet_info *pinfo,
                    proto_tree *tree, int hfindex, guint32 *pdata);

extern int dissect_pn_mac(tvbuff_t *tvb, int offset, packet_info *pinfo,
                    proto_tree *tree, int hfindex, guint8 *pdata);

extern int dissect_pn_ipv4(tvbuff_t *tvb, int offset, packet_info *pinfo,
                    proto_tree *tree, int hfindex, guint32 *pdata);

extern int dissect_pn_uuid(tvbuff_t *tvb, int offset, packet_info *pinfo,
                    proto_tree *tree, int hfindex, e_guid_t *uuid);

extern int dissect_pn_undecoded(tvbuff_t *tvb, int offset, packet_info *pinfo,
                    proto_tree *tree, guint32 length);

extern int dissect_pn_user_data(tvbuff_t *tvb, int offset, packet_info *pinfo _U_,
                    proto_tree *tree, guint32 length, const char *text);

extern int dissect_blocks(tvbuff_t *tvb, int offset,
                    packet_info *pinfo, proto_tree *tree, guint8 *drep);

#define PDU_TYPE_REQ 0x05
#define PDU_TYPE_RSP 0x06

extern int dissect_rsi_blocks(tvbuff_t* tvb, int offset, packet_info* pinfo, proto_tree* tree, guint8* drep, guint32 u32FOpnumOffsetOpnum, int type);

#define SUBST_DATA  1
#define FRAG_DATA   2

extern int dissect_pn_user_data_bytes(tvbuff_t *tvb, int offset, packet_info *pinfo _U_,
                    proto_tree *tree, guint32 length, int iSelect);

extern int dissect_pn_malformed(tvbuff_t *tvb, int offset, packet_info *pinfo,
                    proto_tree *tree, guint32 length);

extern int dissect_pn_padding(tvbuff_t *tvb, int offset, packet_info *pinfo,
                    proto_tree *tree, int length);

extern int dissect_pn_align4(tvbuff_t *tvb, int offset, packet_info *pinfo, proto_tree *tree);

extern int dissect_PNIO_status(tvbuff_t *tvb, int offset, packet_info *pinfo,
                    proto_tree *tree, guint8 *drep);

extern int dissect_PNIO_C_SDU_RTC1(tvbuff_t* tvb, int offset, packet_info* pinfo,
                    proto_tree* tree, guint8* drep _U_, guint16 frameid);

extern int dissect_PNIO_RSI(tvbuff_t *tvb, int offset, packet_info *pinfo,
                    proto_tree *tree, guint8 *drep);

extern int dissect_PDRsiInstances_block(tvbuff_t *tvb, int offset,
                    packet_info *pinfo, proto_tree *tree, proto_item *item _U_, guint8 *drep, guint8 u8BlockVersionHigh, guint8 u8BlockVersionLow);

extern void pn_append_info(packet_info *pinfo, proto_item *dcp_item, const char *text);

extern void pn_init_append_aruuid_frame_setup_list(e_guid_t aruuid, guint32 setup);

extern ARUUIDFrame* pn_find_aruuid_frame_setup(packet_info* pinfo);

extern void pn_find_dcp_station_info(stationInfo* station_info, conversation_t* conversation);

extern gboolean dissect_CSF_SDU_heur(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void *data);

#define MAX_LINE_LENGTH          1024   /* used for fgets() */

/* Read a string from an "xml" file, dropping xml comment blocks */
#include <stdio.h>
extern char *pn_fgets(char *str, int n, FILE *stream, wmem_allocator_t *scope);
