/*
 * Copyright (C) 2021 by Cornelis Networks.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the
 * BSD license below:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <ofi.h>

#include "rdma/opx/fi_opx_fabric.h"
#include "rdma/opx/fi_opx_eq.h"

#include <ofi_enosys.h>

struct fi_opx_eq {
	struct fid_eq eq_fid;
};

static int fi_opx_close_eq(struct fid *fid)
{
	struct fi_opx_eq *opx_eq = container_of(fid, struct fi_opx_eq, eq_fid.fid);

	free(opx_eq);
	return FI_SUCCESS;
}

static ssize_t fi_opx_eq_read(struct fid_eq *eq, uint32_t *event, void *buf, size_t len, uint64_t flags)
{
	return -FI_EAGAIN;
}

static ssize_t fi_opx_eq_readerr(struct fid_eq *eq, struct fi_eq_err_entry *buf, uint64_t flags)
{
	return -FI_EAGAIN;
}

static const char *fi_opx_eq_strerror(struct fid_eq *eq, int prov_errno, const void *err_data, char *buf, size_t len)
{
	return fi_strerror(prov_errno);
}

static struct fi_ops fi_opx_eq_fi_ops = {.size	    = sizeof(struct fi_ops),
					.close	    = fi_opx_close_eq,
					.bind	    = fi_no_bind,
					.control   = fi_no_control,
					.ops_open = fi_no_ops_open};

static struct fi_ops_eq fi_opx_eq_ops = {.size     = sizeof(struct fi_ops_eq),
					 .read     = fi_opx_eq_read,
					 .readerr  = fi_opx_eq_readerr,
					 .write    = fi_no_eq_write,
					 .sread    = fi_no_eq_sread,
					 .strerror = fi_opx_eq_strerror};

static int fi_opx_eq_check_attr(struct fi_eq_attr *attr)
{
	if (!attr)
		return FI_SUCCESS;

	if (attr->flags & ~(FI_AFFINITY | FI_PEER))
		return -FI_ENOSYS;

	switch (attr->wait_obj) {
	case FI_WAIT_NONE:
	case FI_WAIT_UNSPEC:
		return FI_SUCCESS;
	default:
		return -FI_ENOSYS;
	}
}

int fi_opx_eq_open(struct fid_fabric *fabric, struct fi_eq_attr *attr, struct fid_eq **eq, void *context)
{
	struct fi_opx_eq *opx_eq;
	int ret;

	ret = fi_opx_eq_check_attr(attr);
	if (ret)
		return ret;

	opx_eq = calloc(1, sizeof(*opx_eq));
	if (!opx_eq)
		return -FI_ENOMEM;

	opx_eq->eq_fid.fid.fclass = FI_CLASS_EQ;
	opx_eq->eq_fid.fid.context = context;
	opx_eq->eq_fid.fid.ops = &fi_opx_eq_fi_ops;
	opx_eq->eq_fid.ops = &fi_opx_eq_ops;

	*eq = &opx_eq->eq_fid;
	return FI_SUCCESS;
}
