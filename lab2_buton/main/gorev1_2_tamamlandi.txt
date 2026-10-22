#include <stdint.h>
#include <inttypes.h>
#include "driver/gpio.h"
#include "esp_cpu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/gpio_struct.h"
#include "esp_log.h"
#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "soc/io_mux_reg.h"
#include "../ortak/lab_reg.h"

#define LED_PIN 4
#define LED_MASK (1U << LED_PIN)

static const char *TAG = "LAB2_GOREV1";
#define TEKRAR 10000
static portMUX_TYPE olcum_kilidi = portMUX_INITIALIZER_UNLOCKED;

__attribute__((noinline))
static void degistir_surucu(void)
{
    for (int i = 0; i < TEKRAR; i++) {
        gpio_set_level(LED_PIN, 1);
        gpio_set_level(LED_PIN, 0);
    }
}

__attribute__((noinline))
static void degistir_yapi(void)
{
    for (int i = 0; i < TEKRAR; i++) {
        GPIO.out_w1ts = LED_MASK;
        GPIO.out_w1tc = LED_MASK;
    }
}

__attribute__((noinline))
static void degistir_register(void)
{
    for (int i = 0; i < TEKRAR; i++) {
        REG_WRITE(GPIO_OUT_W1TS_REG, LED_MASK);
        REG_WRITE(GPIO_OUT_W1TC_REG, LED_MASK);
    }
}

static void hiz_olc(const char *ad, void (*fonksiyon)(void))
{
    uint32_t baslangic, bitis;

    portENTER_CRITICAL(&olcum_kilidi);
    baslangic = esp_cpu_get_cycle_count();
    fonksiyon();
    bitis = esp_cpu_get_cycle_count();
    portEXIT_CRITICAL(&olcum_kilidi);

    double cevrim = (double)(uint32_t)(bitis - baslangic) / TEKRAR;
    double frekans = (double)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ * 1000000.0 / cevrim;

    ESP_LOGI("LAB2_GOREV2", "%s: %.2f cevrim/periyot, f = %.0f Hz",
             ad, cevrim, frekans);
}


static void register_yazdir(const char *baslik)
{
    ESP_LOGI(TAG, "--- %s ---", baslik);

    ESP_LOGI(TAG, "IO_MUX_GPIO4 = 0x%08" PRIx32,
             REG_READ(IO_MUX_GPIO4_REG));

    ESP_LOGI(TAG, "GPIO_PIN4 = 0x%08" PRIx32,
             REG_READ(GPIO_PIN4_REG));

    ESP_LOGI(TAG, "FUNC4_OUT_SEL = 0x%08" PRIx32,
             REG_READ(GPIO_FUNC0_OUT_SEL_CFG_REG + LED_PIN * 4));

    ESP_LOGI(TAG, "ENABLE bit 4 = %" PRIu32,
             (REG_READ(GPIO_ENABLE_REG) >> LED_PIN) & 1U);
}

void app_main(void)
{
    // Yöntem 1: Register ve düşük seviye pin ayarları
    gpio_cikis_ayarla(LED_PIN);

    register_yazdir("Register ile ayar");

    // Yöntem 2: ESP-IDF sürücüsü
    gpio_config_t io = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(gpio_config(&io));

    register_yazdir("gpio_config() ile ayar");

    gpio_set_level(LED_PIN, 0);

    ESP_LOGI("LAB2_GOREV2", "CPU saati: %d MHz",
             CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ);

    for (int tekrar = 0; tekrar < 5; tekrar++) {
        ESP_LOGI("LAB2_GOREV2", "Olcum tekrari: %d", tekrar + 1);

        hiz_olc("surucu", degistir_surucu);
        vTaskDelay(pdMS_TO_TICKS(200));

        hiz_olc("yapi", degistir_yapi);
        vTaskDelay(pdMS_TO_TICKS(200));

        hiz_olc("register", degistir_register);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
