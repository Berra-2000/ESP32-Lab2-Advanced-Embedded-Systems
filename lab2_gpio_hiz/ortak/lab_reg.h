#pragma once

#include <stdint.h>
#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"

static inline void gpio_cikis_ayarla(uint32_t pin)
{
    uint32_t mask = 1U << pin;

    // 1. Çıkışı kapat, başlangıç seviyesini sıfırla
    REG_WRITE(GPIO_ENABLE_W1TC_REG, mask);
    REG_WRITE(GPIO_OUT_W1TC_REG, mask);

    // 2. IO_MUX: GPIO işlevini seç
    if (pin == 4) {
        PIN_FUNC_SELECT(IO_MUX_GPIO4_REG, PIN_FUNC_GPIO);
        REG_SET_BIT(IO_MUX_GPIO4_REG, FUN_IE);
    }

    // 3. Push-pull çıkış, kesmeler kapalı
    if (pin == 4) {
        REG_WRITE(GPIO_PIN4_REG, 0);
    }

    // 4. GPIO çıkış matrisini ayarla
    REG_WRITE(GPIO_FUNC0_OUT_SEL_CFG_REG + pin * 4, 0x100);

    // 5. Çıkışı etkinleştir
    REG_WRITE(GPIO_ENABLE_W1TS_REG, mask);
}
