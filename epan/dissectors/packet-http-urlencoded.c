/* packet-http-urlencoded.c
 * Routines for dissection of HTTP urlecncoded form, based on packet-text-media.c (C) Olivier Biot, 2004.
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "config.h"

#include <epan/packet.h>
#include <epan/charsets.h>
#include <epan/strutil.h>
#include <wsutil/str_util.h>

#include "packet-http.h"

void proto_register_http_urlencoded(void);
void proto_reg_handoff_http_urlencoded(void);

static dissector_handle_t form_urlencoded_handle;

static int proto_urlencoded = -1;

static int hf_form_key = -1;
static int hf_form_value = -1;

static gint ett_form_urlencoded = -1;
static gint ett_form_keyvalue = -1;

static int
get_form_key_value(tvbuff_t *tvb, char **ptr, int offset, char stop)
{
	const int orig_offset = offset;
	char *tmp;
	int len;

	len = 0;
	while (tvb_reported_length_remaining(tvb, offset) > 0) {
		guint8 ch;

		ch = tvb_get_guint8(tvb, offset);
		if (!ch)
			return -1;
		if (ch == stop)
			break;
		if (ch == '%') {
			offset++;
			ch = tvb_get_guint8(tvb, offset);
			if (ws_xton(ch) == -1)
				return -1;

			offset++;
			ch = tvb_get_guint8(tvb, offset);
			if (ws_xton(ch) == -1)
				return -1;
		}

		len++;
		offset++;
	}

	*ptr = tmp = (char*)wmem_alloc(wmem_packet_scope(), len + 1);
	tmp[len] = '\0';

	len = 0;
	offset = orig_offset;
	while (tvb_reported_length_remaining(tvb, offset) > 0) {
		guint8 ch;

		ch = tvb_get_guint8(tvb, offset);
		if (!ch)
			return -1;
		if (ch == stop)
			break;

		if (ch == '%') {
			guint8 ch1, ch2;

			offset++;
			ch1 = tvb_get_guint8(tvb, offset);

			offset++;
			ch2 = tvb_get_guint8(tvb, offset);

			tmp[len] = ws_xton(ch1) << 4 | ws_xton(ch2);

		} else if (ch == '+')
			tmp[len] = ' ';
		else
			tmp[len] = ch;

		len++;
		offset++;
	}

	return offset;
}


static int
dissect_form_urlencoded(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void* data)
{
	proto_tree	*url_tree;
	proto_tree	*sub;
	proto_item	*ti;
	gint		offset = 0, next_offset;
	const char	*data_name;
	http_message_info_t *message_info;

	data_name = pinfo->match_string;
	if (! (data_name && data_name[0])) {
		/*
		 * No information from "match_string"
		 */
		message_info = (http_message_info_t *)data;
		if (message_info == NULL) {
			/*
			 * No information from dissector data
			 */
			data_name = NULL;
		} else {
			data_name = message_info->media_str;
			if (! (data_name && data_name[0])) {
				/*
				 * No information from dissector data
				 */
				data_name = NULL;
			}
		}
	}

	if (data_name)
		col_append_sep_fstr(pinfo->cinfo, COL_INFO, " ", "(%s)", data_name);

	ti = proto_tree_add_item(tree, proto_urlencoded, tvb, 0, -1, ENC_NA);
	if (data_name)
		proto_item_append_text(ti, ": %s", data_name);
	url_tree = proto_item_add_subtree(ti, ett_form_urlencoded);

	while (tvb_reported_length_remaining(tvb, offset) > 0) {
		const int start_offset = offset;
		char *key, *value;
		char *key_decoded, *value_decoded;

		sub = proto_tree_add_subtree(url_tree, tvb, offset, 0, ett_form_keyvalue, &ti, "Form item");

		next_offset = get_form_key_value(tvb, &key, offset, '=');
		if (next_offset == -1)
			break;
		/* XXX: Only UTF-8 is conforming according to WHATWG, though we
		 * ought to look for a "charset" parameter in media_str
		 * to handle other encodings.
		 * Our charset functions should probably return a boolean
		 * indicating that replacement characters had to be used,
		 * and that the string was not the expected encoding.
		 */
		key_decoded = get_utf_8_string(pinfo->pool, key, (int)strlen(key));
		proto_tree_add_string(sub, hf_form_key, tvb, offset, next_offset - offset, key_decoded);
		proto_item_append_text(sub, ": \"%s\"", format_text(pinfo->pool, key, strlen(key)));

		offset = next_offset+1;

		next_offset = get_form_key_value(tvb, &value, offset, '&');
		if (next_offset == -1)
			break;
		value_decoded = get_utf_8_string(pinfo->pool, value, (int)strlen(value));
		proto_tree_add_string(sub, hf_form_value, tvb, offset, next_offset - offset, value_decoded);
		proto_item_append_text(sub, " = \"%s\"", format_text(pinfo->pool, value, strlen(value)));

		offset = next_offset+1;

		proto_item_set_len(ti, offset - start_offset);
	}

	return tvb_captured_length(tvb);
}

void
proto_register_http_urlencoded(void)
{
	static hf_register_info hf[] = {
		{ &hf_form_key,
			{ "Key", "urlencoded-form.key",
			  FT_STRINGZ, BASE_NONE, NULL, 0x0,
			  NULL, HFILL }
		},
		{ &hf_form_value,
			{ "Value", "urlencoded-form.value",
			  FT_STRINGZ, BASE_NONE, NULL, 0x0,
			  NULL, HFILL }
		},
	};

	static gint *ett[] = {
		&ett_form_urlencoded,
		&ett_form_keyvalue
	};

	proto_urlencoded = proto_register_protocol("HTML Form URL Encoded", "URL Encoded Form Data", "urlencoded-form");

	form_urlencoded_handle = register_dissector("urlencoded-form", dissect_form_urlencoded, proto_urlencoded);

	proto_register_field_array(proto_urlencoded, hf, array_length(hf));
	proto_register_subtree_array(ett, array_length(ett));
}

void
proto_reg_handoff_http_urlencoded(void)
{
	dissector_add_string("media_type", "application/x-www-form-urlencoded", form_urlencoded_handle);
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: t
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 noexpandtab:
 * :indentSize=8:tabSize=8:noTabs=false:
 */
