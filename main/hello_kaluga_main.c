/*
 * Hello Kaluga — Clase 1 (Sistemas Embebidos, UCU 2026)
 *
 * Firmware base de la App 1: hace parpadear/ciclar el LED RGB direccionable
 * (WS2812) de la placa ESP32-S2-Kaluga-1 y publica el estado por el monitor
 * serie.
 *
 * Hardware:
 *   - LED RGB direccionable en GPIO45 (requiere el JUMPER RGB colocado).
 *   - Target: esp32s2.
 *
 * Componente: espressif/led_strip (backend RMT), declarado en
 * main/idf_component.yml.
 */

#include <stdio.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h"

/* LED RGB direccionable de la Kaluga-1 (ver User Guide del kit). */

/* GPIO (General Purpose Input/Output): pin genérico del micro, sin función
 * fija, cuyo modo (entrada/salida) y nivel se deciden por software. El 45 solo
 * llega al LED si el jumper RGB está colocado.
 * Además es un pin de "strapping": el chip lee su nivel durante el reset para
 * fijar el ajuste de arranque para la tensión de VDD_SPI (memorias flash y PSRAM). 
 * Después del arranque, el Pin se puede usar para otra cosa, por ejemplo, conectar
 * el dispositivo WS2812. */
#define RGB_LED_GPIO      45
/* La placa lleva un único LED; este valor también dimensiona el framebuffer del driver. */
#define RGB_LED_COUNT     1
/* Tiempo de permanencia en cada color de la secuencia. */
#define STEP_DELAY_MS     500

static const char *TAG = "hello_kaluga";

/* Un color RGB de 8 bits por componente. */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    const char *nombre;
} color_t;

/* Secuencia de colores que recorreremos en bucle. */
static const color_t secuencia[] = {
    { 32,  0,  0, "ROJO"    },
    {  0, 32,  0, "VERDE"   },
    {  0,  0, 32, "AZUL"    },
    { 32, 24,  0, "AMARILLO"},
    {  0,  0,  0, "APAGADO" },
};

static led_strip_handle_t configurar_led(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_LED_GPIO,
        .max_leds = RGB_LED_COUNT,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, /* 10 MHz */
        .flags.with_dma = false,
    };

    led_strip_handle_t led_strip;
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    return led_strip;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Hola desde la ESP32-S2-Kaluga-1");
    ESP_LOGI(TAG, "Controlando el LED RGB en GPIO%d (jumper RGB requerido)", RGB_LED_GPIO);

    led_strip_handle_t led_strip = configurar_led();

    const size_t n = sizeof(secuencia) / sizeof(secuencia[0]);
    uint32_t iteracion = 0;

    while (1) {
        const color_t *c = &secuencia[iteracion % n];

        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, c->r, c->g, c->b));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));

        ESP_LOGI(TAG, "[%" PRIu32 "] LED -> %-8s (R=%3u G=%3u B=%3u)",
                 iteracion, c->nombre, c->r, c->g, c->b);

        iteracion++;
        vTaskDelay(pdMS_TO_TICKS(STEP_DELAY_MS));
    }
}
