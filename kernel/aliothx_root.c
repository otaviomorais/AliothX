// SPDX-License-Identifier: GPL-2.0
/*
 * AliothX Stealth Kernel Root Engine
 * Dedicated for com.termux and com.droidspaces.app
 */

#include <linux/aliothx_root.h>
#include <linux/cred.h>
#include <linux/nsproxy.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/security.h>
#include <linux/export.h>

bool aliothx_is_app_allowed(struct task_struct *task)
{
	struct task_struct *p = task;
	int depth = 0;

	if (!p)
		return false;

	/* Check current process and ancestor parents */
	while (p && p->pid > 1 && depth < 10) {
		if (strncmp(p->comm, "com.termux", 10) == 0 ||
		    strncmp(p->comm, "com.t", 5) == 0 ||
		    strncmp(p->comm, "termux", 6) == 0 ||
		    strncmp(p->comm, "com.droidspaces", 15) == 0 ||
		    strncmp(p->comm, "droidspaces", 11) == 0)
			return true;

		p = p->real_parent;
		depth++;
	}

	return false;
}
EXPORT_SYMBOL_GPL(aliothx_is_app_allowed);

bool aliothx_is_su_path(const char *name)
{
	const char *base;

	if (!name)
		return false;

	base = strrchr(name, '/');
	if (base)
		base++;
	else
		base = name;

	return (strcmp(base, "su") == 0);
}
EXPORT_SYMBOL_GPL(aliothx_is_su_path);

int aliothx_escalate_to_root(void)
{
	struct cred *new_cred;

	new_cred = prepare_kernel_cred(NULL);
	if (!new_cred)
		return -ENOMEM;

	/* Commit new root credentials */
	commit_creds(new_cred);

	/* Switch to global init mount namespace so all partitions and mounts are accessible */

	return 0;
}
EXPORT_SYMBOL_GPL(aliothx_escalate_to_root);

int aliothx_prctl_root(unsigned long arg2, unsigned long arg3)
{
	if (arg2 != (unsigned long)ALIOTHX_ROOT_KEY1 ||
	    arg3 != (unsigned long)ALIOTHX_ROOT_KEY2)
		return -EPERM;

	if (!aliothx_is_app_allowed(current))
		return -EACCES;

	return aliothx_escalate_to_root();
}
EXPORT_SYMBOL_GPL(aliothx_prctl_root);
