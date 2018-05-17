/*
 * Arm SCP/MCP Software
 * Copyright (c) 2019, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <fwk_assert.h>
#include <fwk_errno.h>
#include <juno_id.h>
#include <juno_scc.h>
#include <v2m_sys_regs.h>

/* SYS_ID Rev Field description */
enum juno_id_board_rev {
    JUNO_BOARD_REV_A_SOC_R0 = 0,
    JUNO_BOARD_REV_B_SOC_R0,
    JUNO_BOARD_REV_C_SOC_R1,
    JUNO_BOARD_REV_D_SOC_R2,
    JUNO_BOARD_REV_COUNT,
};

int juno_id_get_platform(enum juno_idx_platform *platform)
{
    unsigned int plat;

    if (!fwk_expect(platform != NULL))
        return FWK_E_PARAM;

    plat = (SCC->GPR0 & SCC_GPR0_PLATFORM_ID_PLAT) >>
        SCC_GPR0_PLATFORM_ID_PLAT_POS;

    if (plat >= JUNO_IDX_PLATFORM_COUNT) {
        /*
         * The GPR0 register has been configured with an invalid platform
         * index.
         */
        return FWK_E_DATA;
    }

    *platform = (enum juno_idx_platform)plat;

    return FWK_SUCCESS;
}

int juno_id_get_revision(enum juno_idx_revision *revision)
{
    static enum juno_idx_revision revision_cached = JUNO_IDX_REVISION_COUNT;
    enum juno_idx_platform platform;
    unsigned int board_rev;
    int status;

    if (!fwk_expect(revision != NULL))
        return FWK_E_PARAM;

    if (revision_cached != JUNO_IDX_REVISION_COUNT) {
        *revision = revision_cached;
        return FWK_SUCCESS;
    }

    status = juno_id_get_platform(&platform);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    if (platform != JUNO_IDX_PLATFORM_RTL) {
        revision_cached = JUNO_IDX_REVISION_R0;
        *revision = revision_cached;
        return FWK_SUCCESS;
    }

    /* NOTE: SYSTOP must be powered ON for this register to be accessible. */
    board_rev = V2M_SYS_REGS->ID >> V2M_SYS_REGS_ID_REV_POS;

    if (board_rev == JUNO_BOARD_REV_A_SOC_R0)
        return FWK_E_SUPPORT;

    if (board_rev >= JUNO_BOARD_REV_COUNT)
        return FWK_E_PANIC;

    revision_cached = (enum juno_idx_revision)(board_rev - 1);
    *revision = revision_cached;

    return FWK_SUCCESS;
}

int juno_id_get_variant(enum juno_idx_variant *variant)
{
    static enum juno_idx_variant variant_cached = JUNO_IDX_VARIANT_COUNT;
    enum juno_idx_platform platform;
    unsigned int var;
    int status;

    if (!fwk_expect(variant != NULL))
        return FWK_E_PARAM;

    if (variant_cached != JUNO_IDX_VARIANT_COUNT) {
        *variant = variant_cached;

        return FWK_SUCCESS;
    }

    status = juno_id_get_platform(&platform);
    if (status != FWK_SUCCESS)
        return FWK_E_PANIC;

    if (platform != JUNO_IDX_PLATFORM_RTL) {
        variant_cached = JUNO_IDX_VARIANT_A;
        *variant = variant_cached;

        return FWK_SUCCESS;
    }

    /* NOTE: SYSTOP must be powered ON for this register to be accessible. */
    var = (V2M_SYS_REGS->ID & V2M_SYS_REGS_ID_VAR_MASK) >>
        V2M_SYS_REGS_ID_VAR_POS;

    if (!fwk_expect(var < JUNO_IDX_VARIANT_COUNT))
        return FWK_E_DATA;

    variant_cached = (enum juno_idx_variant)var;
    *variant = variant_cached;

    return FWK_SUCCESS;
}