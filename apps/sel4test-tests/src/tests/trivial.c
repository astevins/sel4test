/*
 * Copyright 2017, Data61, CSIRO (ABN 41 687 119 230)
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <sel4test/test.h>
#include <vka/capops.h>
#include "../test.h"
#include "../helpers.h"

#define MIN_EXPECTED_ALLOCATIONS 100

int test_trivial(env_t env)
{
    test_geq(2, 1);
    return sel4test_get_result();
}
DEFINE_TEST(TRIVIAL0000, "Ensure the test framework functions", test_trivial, true)

int test_allocator(env_t env)
{
    /* Perform a bunch of allocations and frees */
    vka_object_t endpoint;
    int error;

    for (int i = 0; i < MIN_EXPECTED_ALLOCATIONS; i++)
    {
        error = vka_alloc_endpoint(&env->vka, &endpoint);
        test_error_eq(error, 0);
        test_assert(endpoint.cptr != 0);
        vka_free_object(&env->vka, &endpoint);
    }

    return sel4test_get_result();
}

int test_debug_cap_identify(env_t env)
{
    vka_object_t untyped;
    int error;

    /* Get the paddr of an untyped cap */
    error = vka_alloc_untyped(&env->vka, seL4_PageBits, &untyped);
    test_error_eq(error, 0);

    uint64_t untyped_paddr = seL4_DebugCapPaddr(untyped.cptr);
    printf("Untyped has paddr 0x%" PRIx64 "\n", untyped_paddr);
    test_assert(untyped_paddr != 0);

    /* Retype the untyped to frame and get paddr */
    seL4_CPtr frame = get_free_slot(env);
    test_assert(frame != seL4_CapNull);

    cspacepath_t dest;
    vka_cspace_make_path(&env->vka, frame, &dest);

    error = seL4_Untyped_Retype(untyped.cptr, kobject_get_type(KOBJECT_FRAME, seL4_PageBits), 
                                seL4_PageBits, dest.root, dest.dest, dest.destDepth, 
                                dest.offset, 1);
    test_error_eq(error, seL4_NoError);

    uint64_t frame_paddr = seL4_DebugCapPaddr(frame);
    printf("Frame has paddr 0x%" PRIx64 "\n", frame_paddr);
    test_assert(untyped_paddr == frame_paddr);

    /* Cleanup */
    vka_cnode_delete(&dest);
    vka_cspace_free(&env->vka, frame);
    vka_free_object(&env->vka, &untyped);
}

DEFINE_TEST(TRIVIAL0001, "Ensure the allocator works", test_allocator, true)
DEFINE_TEST(TRIVIAL0002, "Ensure the allocator works more than once", test_allocator, true)
DEFINE_TEST(TRIVIAL0003, "Ensure seL4_DebugCapIdentify works", test_debug_cap_identify, true)
