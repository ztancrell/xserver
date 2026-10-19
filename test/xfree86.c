/**
 * Copyright © 2011 Red Hat, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice (including the next
 *  paragraph) shall be included in all copies or substantial portions of the
 *  Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 */

/* Test relies on assert() */
#undef NDEBUG

#include <dix-config.h>

#include <assert.h>

#include "config/hotplug_priv.h"

#include "xf86.h"
#include "xf86Bus.h"
#include "xf86Parser.h"

#include "tests-common.h"

static void
xfree86_option_list_duplicate(void)
{
    XF86OptionPtr options;
    XF86OptionPtr duplicate;
    const char *o1 = "foo", *o2 = "bar", *v1 = "one", *v2 = "two";
    const char *o_null = "NULL";
    char *val1, *val2;
    XF86OptionPtr a, b;

    duplicate = xf86OptionListDuplicate(NULL);
    assert(!duplicate);

    options = xf86AddNewOption(NULL, o1, v1);
    assert(options);
    options = xf86AddNewOption(options, o2, v2);
    assert(options);
    options = xf86AddNewOption(options, o_null, NULL);
    assert(options);

    duplicate = xf86OptionListDuplicate(options);
    assert(duplicate);

    val1 = xf86CheckStrOption(options, o1, "1");
    val2 = xf86CheckStrOption(duplicate, o1, "2");

    assert(strcmp(val1, v1) == 0);
    assert(strcmp(val1, val2) == 0);
    free(val1);
    free(val2);

    val1 = xf86CheckStrOption(options, o2, "1");
    val2 = xf86CheckStrOption(duplicate, o2, "2");

    assert(strcmp(val1, v2) == 0);
    assert(strcmp(val1, val2) == 0);
    free(val1);
    free(val2);

    a = xf86FindOption(options, o_null);
    b = xf86FindOption(duplicate, o_null);
    assert(a);
    assert(b);

    xf86OptionListFree(duplicate);
    xf86OptionListFree(options);
}

static void
xfree86_add_comment(void)
{
    char *current = NULL;
    const char *comment;
    char compare[1024] = { 0 };

    comment = "# foo";
    current = xf86addComment(current, comment);
    strcpy(compare, comment);
    strcat(compare, "\n");

    assert(!strcmp(current, compare));

    /* this used to overflow */
    strcpy(current, "\n");
    comment = "foobar\n";
    current = xf86addComment(current, comment);
    strcpy(compare, "\n#");
    strcat(compare, comment);
    assert(!strcmp(current, compare));

    free(current);
}

/*
 * A GPU screen added by xf86platformAddGPUDevices() claims its platform
 * slot without a Device section, so the entity has no instances. Another
 * driver must still see that slot as taken, by platform device and by path.
 */
static void
xfree86_check_slot_deviceless_entity(void)
{
    static struct OdevAttributes attribs = { .path = (char *) "/dev/dri/card1" };
    static struct xf86_platform_device plat = { .attribs = &attribs };
    static DriverRec drv = { .driverName = (char *) "modesetting" };
    static GDevRec gdev = { .identifier = (char *) "test", .driver = (char *) "modesetting" };
    EntityPtr p;
    int num;

    gdev.options = xf86AddNewOption(NULL, "kmsdev", "/dev/dri/card1");

    assert(xf86CheckSlot(&plat, BUS_PLATFORM));
    assert(xf86CheckSlot(&gdev, BUS_NONE));

    num = xf86AllocateEntity();
    p = xf86Entities[num];
    p->driver = &drv;
    p->bus.type = BUS_PLATFORM;
    p->bus.id.plat = &plat;
    assert(p->numInstances == 0);

    assert(!xf86CheckSlot(&plat, BUS_PLATFORM));
    assert(!xf86CheckSlot(&gdev, BUS_NONE));

    /* an unclaimed slot is free again */
    p->bus.type = BUS_NONE;
    assert(xf86CheckSlot(&plat, BUS_PLATFORM));
    assert(xf86CheckSlot(&gdev, BUS_NONE));

    xf86OptionListFree(gdev.options);
}

const testfunc_t*
xfree86_test(void)
{
    static const testfunc_t testfuncs[] = {
        xfree86_option_list_duplicate,
        xfree86_add_comment,
        xfree86_check_slot_deviceless_entity,
        NULL,
    };
    return testfuncs;
}
