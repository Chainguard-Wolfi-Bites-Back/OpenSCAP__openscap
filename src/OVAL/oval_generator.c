/**
 * @file oval_generator.c
 * @brief OVAL generator data type implementation
 * @author "Tomas Heinrich" <theinric@redhat.com>
 */

/*
 * Copyright 2010 Red Hat Inc., Durham, North Carolina.
 * All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors:
 *      "Tomas Heinrich" <theinric@redhat.com>
 */

#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "common/util.h"
#include "common/debug_priv.h"
#include "oval_agent_api_impl.h"

struct oval_generator {
	char *product_name;
	char *product_version;
	char *schema_version;
	char *timestamp;
};

struct oval_generator *oval_generator_new(void)
{
	struct oval_generator *gen;
	time_t et;
	struct tm *lt;
	char timestamp[] = "yyyy-mm-ddThh:mm:ss";

	gen = oscap_alloc(sizeof(struct oval_generator));
	gen->product_name = oscap_strdup("OpenSCAP");
	gen->product_version = NULL;
	gen->schema_version = oscap_strdup(OVAL_DEFAULT_VERSION);

	time(&et);
	lt = localtime(&et);
	snprintf(timestamp, sizeof(timestamp), "%4d-%02d-%02dT%02d:%02d:%02d",
		 1900 + lt->tm_year, 1 + lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
	gen->timestamp = oscap_strdup(timestamp);

	return gen;
}

void oval_generator_free(struct oval_generator *generator)
{
	oscap_free(generator->product_name);
	oscap_free(generator->product_version);
	oscap_free(generator->schema_version);
	oscap_free(generator->timestamp);
	oscap_free(generator);
}

struct oval_generator *oval_genrator_clone(struct oval_generator *old_generator)
{
	struct oval_generator *new_gen;

	new_gen = oval_generator_new();
	new_gen->product_name = oscap_strdup(old_generator->product_name);
	new_gen->product_version = oscap_strdup(old_generator->product_version);
	new_gen->schema_version = oscap_strdup(old_generator->schema_version);
	new_gen->timestamp = oscap_strdup(old_generator->timestamp);

	return new_gen;
}

char *oval_generator_get_product_name(struct oval_generator *generator)
{
	return generator->product_name;
}

char *oval_generator_get_product_version(struct oval_generator *generator)
{
	return generator->product_version;
}

char *oval_generator_get_schema_version(struct oval_generator *generator)
{
	return generator->schema_version;
}

char *oval_generator_get_timestamp(struct oval_generator *generator)
{
	return generator->timestamp;
}

bool oval_generator_is_valid(struct oval_generator *generator)
{
	// todo
	return true;
}

void oval_generator_set_product_name(struct oval_generator *generator, char *product_name)
{
	oscap_free(generator->product_name);
	generator->product_name = oscap_strdup(product_name);
}

void oval_generator_set_product_version(struct oval_generator *generator, char *product_version)
{
	oscap_free(generator->product_version);
	generator->product_version = oscap_strdup(product_version);
}

void oval_generator_set_schema_version(struct oval_generator *generator, char *schema_version)
{
	oscap_free(generator->schema_version);
	generator->schema_version = oscap_strdup(schema_version);
}

void oval_generator_set_timestamp(struct oval_generator *generator, char *timestamp)
{
	oscap_free(generator->timestamp);
	generator->timestamp = oscap_strdup(timestamp);
}

xmlNode *oval_generator_to_dom(struct oval_generator *generator, xmlDocPtr doc, xmlNode *parent)
{
	xmlNode *gen_node;
	xmlNs *ns_common;

	gen_node = xmlNewTextChild(parent, NULL, BAD_CAST "generator", NULL);
	ns_common = xmlSearchNsByHref(doc, parent, OVAL_COMMON_NAMESPACE);
	if (generator->product_name)
		xmlNewTextChild(parent, ns_common, BAD_CAST "product_name", BAD_CAST generator->product_name);
	if (generator->product_version)
		xmlNewTextChild(parent, ns_common, BAD_CAST "product_version", BAD_CAST generator->product_version);
	if (generator->schema_version)
		xmlNewTextChild(parent, ns_common, BAD_CAST "schema_version", BAD_CAST generator->schema_version);
	if (generator->timestamp)
		xmlNewTextChild(parent, ns_common, BAD_CAST "timestamp", BAD_CAST generator->timestamp);

	return gen_node;
}

static int _oval_generator_parse_tag(xmlTextReader *reader, struct oval_parser_context *context, void *user)
{
	char *tagname, *namespace, *val = NULL;
	struct oval_generator *gen = user;
	int ret;

	tagname = (char *) xmlTextReaderLocalName(reader);
	namespace = (char *) xmlTextReaderNamespaceUri(reader);

	if (!strcmp("product_name", tagname)) {
		ret = xmlTextReaderRead(reader);
		val = (char *) xmlTextReaderValue(reader);
		dI("product_name: %s.\n", val);
		oval_generator_set_product_name(gen, val);
	} else if (!strcmp("product_version", tagname)) {
		ret = xmlTextReaderRead(reader);
		val = (char *) xmlTextReaderValue(reader);
		dI("product_version: %s.\n", val);
		oval_generator_set_product_version(gen, val);
	} else if (!strcmp("schema_version", tagname)) {
		ret = xmlTextReaderRead(reader);
		val = (char *) xmlTextReaderValue(reader);
		dI("schema_version: %s.\n", val);
		oval_generator_set_schema_version(gen, val);
	} else if (!strcmp("timestamp", tagname)) {
		ret = xmlTextReaderRead(reader);
		val = (char *) xmlTextReaderValue(reader);
		dI("timestamp: %s.\n", val);
		oval_generator_set_timestamp(gen, val);
	} else {
		dW("Unprocessed tag: <%s:%s>.\n", namespace, tagname);
		oval_parser_skip_tag(reader, context);
		ret = 0;
	}

	oscap_free(tagname);
	oscap_free(namespace);
	oscap_free(val);

	return ret;
}

int oval_generator_parse_tag(xmlTextReader *reader, struct oval_parser_context *context, struct oval_generator *generator)
{
	return oval_parser_parse_tag(reader, context, &_oval_generator_parse_tag, generator);
}