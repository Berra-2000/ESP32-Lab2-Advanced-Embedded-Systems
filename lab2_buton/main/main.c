#include <stdint.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define BUTON 0
#define LED 4
#define KUYRUK_BOYU 64
#define SESSIZLIK_MS 50

static QueueHandle_t kenar_kuyrugu;
static const char *TAG = "LAB2_BUTON";

static void IRAM_ATTR buton_kesmesi(void *arg)
{
    int64_t zaman = esp_timer_get_time();
    BaseType_t uyandir = pdFALSE;

    xQueueSendFromISR(kenar_kuyrugu, &zaman, &uyandir);

    if (uyandir) {
        portYIELD_FROM_ISR();
    }
}

void app_main(void)
{
    gpio_config_t led_ayar = {
        .pin_bit_mask = 1ULL << LED,
        .mode = GPIO_MODE_OUTPUT
    };
    ESP_ERROR_CHECK(gpio_config(&led_ayar));

    gpio_config_t buton_ayar = {
        .pin_bit_mask = 1ULL << BUTON,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&buton_ayar));

    kenar_kuyrugu = xQueueCreate(KUYRUK_BOYU, sizeof(int64_t));
    configASSERT(kenar_kuyrugu != NULL);

    ESP_ERROR_CHECK(gpio_install_isr_service(0));
    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTON, buton_kesmesi, NULL));

    ESP_LOGI(TAG, "Hazir! BOOT butonuna bas.");

    int64_t zaman, ilk = 0, son = 0;
    int kenar_sayisi = 0;
    int led_durum = 0;    while (1) {
        if (xQueueReceive(kenar_kuyrugu, &zaman, pdMS_TO_TICKS(SESSIZLIK_MS) + 1) == pdTRUE) {
            if (kenar_sayisi == 0) {
                ilk = zaman;
            }
            son = zaman;
            kenar_sayisi++;
        }
        else if (kenar_sayisi > 0) {
            int64_t sure = son - ilk;

            ESP_LOGI(TAG,
                     "Kenar sayisi: %d | Sicrama: %" PRId64 " us",
                     kenar_sayisi, sure);

            if (gpio_get_level(BUTON) == 0) {
                led_durum = !led_durum;
                gpio_set_level(LED, led_durum);

                ESP_LOGI(TAG, "LED: %s",
                         led_durum ? "ACIK" : "KAPALI");
            }

            kenar_sayisi = 0;
        }
    }
}
