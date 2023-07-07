#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "pico/sem.h"

#include "hardware/irq.h"
#include "hardware/pwm.h"
#include "hardware/sync.h"

#include "chimes.h"

#define BUFF_SIZE (SAMPLERATE)
#define AUDIO_PIN (28)
#define FCPU (266000000UL)
#define PWM_MULT (8)

static queue_t results_queue;
static struct semaphore pwm_end_sem;

static uint16_t outputBuffer[2][BUFF_SIZE];
static uint16_t *volatile buf_p;

static void pwm_interrupt_handler()
{
    static size_t pos = 0;
    static uint8_t div = PWM_MULT;

    pwm_clear_irq(pwm_gpio_to_slice_num(AUDIO_PIN));

    pwm_set_gpio_level(AUDIO_PIN, buf_p[pos / PWM_MULT]);
    pos++;
    if (pos >= (BUFF_SIZE * PWM_MULT))
    {
        pos = 0;
        sem_release(&pwm_end_sem);
    }
}

static void core1_entry()
{
    float sample;
    size_t n = 0;

    chime_t *chime = chime_init();
    assert(chime);

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    while (1)
    {
        gpio_put(LED_PIN, 1);
        for (size_t i = 0; i < BUFF_SIZE; i++)
        {
            sample = chime_update(chime);
            if (chime_is_finished(chime))
            {
                chime = chime_init();
            }
            outputBuffer[n][i] = (sample + 1.0) * 80;
        }
        uint16_t *data = outputBuffer[n];
        gpio_put(LED_PIN, 0);
        queue_add_blocking(&results_queue, &data);
        n ^= 1;
    }
}

int main()
{
    uint16_t *data;

    srand(time(NULL));
    set_sys_clock_khz(FCPU / 1000, true);

    queue_init(&results_queue, sizeof(uint16_t *), 1);
    sem_init(&pwm_end_sem, 0, 1);

    gpio_set_function(AUDIO_PIN, GPIO_FUNC_PWM);

    int audio_pin_slice = pwm_gpio_to_slice_num(AUDIO_PIN);

    // Setup PWM interrupt to fire when PWM cycle is complete
    pwm_clear_irq(audio_pin_slice);
    pwm_set_irq_enabled(audio_pin_slice, true);
    // set the handle function above
    irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_interrupt_handler);
    irq_set_enabled(PWM_IRQ_WRAP, true);

    pwm_config config = pwm_get_default_config();

    pwm_config_set_clkdiv(&config, (1.0 * FCPU / (250 * (PWM_MULT - 1) * SAMPLERATE)));
    pwm_config_set_wrap(&config, 250);

    pwm_set_gpio_level(AUDIO_PIN, 0);

    multicore_launch_core1(core1_entry);

    queue_peek_blocking(&results_queue, &data);
    buf_p = data;
    pwm_init(audio_pin_slice, &config, true);

    while (1)
    {
        sem_acquire_blocking(&pwm_end_sem);
        queue_remove_blocking(&results_queue, &data);
        queue_peek_blocking(&results_queue, &data);
        buf_p = data;
    }
}
