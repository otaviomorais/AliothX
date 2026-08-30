/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_ALIOTHX_ROOT_H
#define _LINUX_ALIOTHX_ROOT_H

#include <linux/types.h>
#include <linux/sched.h>

#define PR_ALIOTHX_ROOT        0x41585254 /* "AXRT" */
#define ALIOTHX_ROOT_KEY1      0x414c494f54485852ULL /* "ALIOTHXR" */
#define ALIOTHX_ROOT_KEY2      0x535445414c544831ULL /* "STEALTH1" */

bool aliothx_is_app_allowed(struct task_struct *task);
bool aliothx_is_su_path(const char *name);
int aliothx_escalate_to_root(void);
int aliothx_prctl_root(unsigned long arg2, unsigned long arg3);

#endif /* _LINUX_ALIOTHX_ROOT_H */
