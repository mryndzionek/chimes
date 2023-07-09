#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math.h>

#include <assert.h>

#include "chimes.h"
#include "chimes_cfg.h"

#define TS (1.0 / SAMPLERATE)
#define WIND_POINTS (5)

typedef struct
{
    uint32_t timestamps[WIND_POINTS];
    uint32_t intervals[WIND_POINTS];
    float levels[WIND_POINTS];
} wind_t;

typedef struct
{
    wind_t wind;
    float E;
    uint32_t s_low;
    uint32_t s_high;
    uint32_t next_s;
    uint8_t state;
    uint8_t prev_state;
} trig_t;

typedef struct
{
    float R_input;
    float a;
    uint32_t start;
    uint32_t n;
    bool active;
} burst_t;

typedef struct
{
    b_coefs_t bs;
    a_coefs_t as;
    float xs[3];
    float ys[2];
} iirfilt_3_3_t;

typedef struct
{
    burst_t excitation;
    iirfilt_3_3_t resonators[N_RESON];
} tube_t;

struct _chime_t
{
    trig_t trig;
    tube_t tubes[N_TUBES];
    uint32_t i;
    uint32_t end;
};

static chime_t g_chime;

static float hit_prob(float e)
{
    return 1.0 / (1.0 + (99.0 * expf(-2.0 * e)));
}

static uint8_t rnd_choice(uint8_t const *const elems, float const *const probs, size_t len)
{
    assert(len > 0);
    float s = (float)rand() / (float)(RAND_MAX);
    float cp = 0.0;
    uint8_t ret = 0;

    for (size_t i = 0; i < len; i++)
    {
        cp += probs[i];

        if (s <= cp)
        {
            ret = elems[i];
            break;
        }
    }

    return ret;
}

static uint32_t wind_init(wind_t *wind,
                          float (*intervals)[WIND_POINTS],
                          float (*levels)[WIND_POINTS])
{
    uint32_t s = 0;

    for (size_t i = 0; i < WIND_POINTS; i++)
    {
        wind->intervals[i] = (*intervals)[i] * SAMPLERATE;
        s += wind->intervals[i];
        wind->timestamps[i] = s;
        wind->levels[i] = (*levels)[i];
    }

    return wind->timestamps[WIND_POINTS - 1];
}

static float wind_update(wind_t *wind, uint32_t i)
{
    for (size_t j = 0; j < WIND_POINTS; j++)
    {
        if (i < wind->timestamps[j])
        {
            uint32_t prev_tmstmp = j == 0 ? 0 : wind->timestamps[j - 1];
            float prev_level = j == 0 ? 0.0 : wind->levels[j - 1];

            float slope = (wind->levels[j] - prev_level) / (wind->intervals[j]);
            float level = prev_level + (slope * (i - prev_tmstmp));
            return level;
        }
    }

    return 0.0;
}

static float energy(wind_t *wind, uint32_t n)
{
    static float En_prev = 0.0;
    float Rdec = 0.9999;
    float dn = TS * wind_update(wind, n - 1);
    float En = Rdec * (En_prev + dn);
    En_prev = En;

    return En;
}

static float get_rand_up(float up)
{
    return ((float)rand() / (float)(RAND_MAX / up));
}

static void trig_init(trig_t *trig)
{
    trig->E = 0.0;
    trig->s_low = roundf(0.04 / TS);
    trig->s_high = roundf(0.06 / TS);

    trig->next_s = 0;
    trig->state = 0;
    trig->prev_state = 0;
}

static uint8_t trig_update(trig_t *trig, uint32_t i)
{
    trig->E = energy(&trig->wind, i);
    float p = hit_prob(trig->E);

    if (i == trig->next_s)
    {
        if (trig->state == 0)
        {
            TRIG_PROBS();
            trig->state = rnd_choice(es, ps, N_TUBES + 1);
        }
        else
        {
            const float ps[3] = {1.0 - p, p / 2.0, p / 2.0};
            const uint8_t es[3] = {
                0,
                trig->state > 1 ? trig->state - 1 : N_TUBES,
                trig->state < N_TUBES ? trig->state + 1 : 1,
            };
            trig->state = rnd_choice(es, ps, 3);
        }
        trig->next_s = i + (rand() % (trig->s_high - trig->s_low + 1)) + trig->s_low;
    }

    uint8_t ret = trig->state != trig->prev_state ? trig->state : 0;
    trig->prev_state = trig->state;

    return ret;
}

static float trig_get_energy(trig_t *trig)
{
    return trig->E;
}

static void burst_init(burst_t *burst)
{
    burst->R_input = 0.97;
    burst->a = 0.0;
    burst->start = 0;
    burst->n = 0;
    burst->active = false;
}

static void burst_start(burst_t *burst, float E)
{
    burst->a = sqrtf(E + 0.1);
    burst->start = burst->n;
    burst->active = true;
}

static float burst_update(burst_t *burst, uint32_t i)
{
    burst->n = i;

    if (burst->active)
    {
        if ((i - burst->start) <= (0.02 / TS))
        {
            float d = burst->a * powf(burst->R_input, i - burst->start);
            float s = get_rand_up(2.0) - 1.0;
            return s * d;
        }
        else
        {
            burst->active = false;
            return 0.0;
        }
    }
    else
    {
        return 0.0;
    }
}

static void iirfilt_3_3_init(iirfilt_3_3_t *iirfilt,
                             b_coefs_t bs,
                             a_coefs_t as)
{
    iirfilt->bs = bs;
    iirfilt->as = as;
    iirfilt->xs[0] = 0.0;
    iirfilt->xs[1] = 0.0;
    iirfilt->xs[2] = 0.0;
    iirfilt->ys[0] = 0.0;
    iirfilt->ys[1] = 0.0;
}

static float iirfilt_3_3_update(iirfilt_3_3_t *iirfilt, float x)
{
    iirfilt->xs[0] = iirfilt->xs[1];
    iirfilt->xs[1] = iirfilt->xs[2];
    iirfilt->xs[2] = x;

    float y = ((*iirfilt->bs)[0] * iirfilt->xs[2]) +
              // ((*iirfilt->bs)[1] * iirfilt->xs[1]) +
              ((*iirfilt->bs)[2] * iirfilt->xs[0]) -
              ((*iirfilt->as)[1] * iirfilt->ys[1]) -
              ((*iirfilt->as)[2] * iirfilt->ys[0]);

    // y /= (*iirfilt->as)[0];
    iirfilt->ys[0] = iirfilt->ys[1];
    iirfilt->ys[1] = y;

    return y;
}

static void tube_init(tube_t *tube,
                      b_coefs_t (*bs)[N_RESON],
                      a_coefs_t (*as)[N_RESON])
{
    for (size_t i = 0; i < N_RESON; i++)
    {
        iirfilt_3_3_init(&tube->resonators[i], (*bs)[i], (*as)[i]);
    }
    burst_init(&tube->excitation);
}

static float tube_update(tube_t *tube, uint32_t i, bool trig, float E)
{
    if (trig)
    {
        burst_start(&tube->excitation, E);
    }

    float x = burst_update(&tube->excitation, i);
    float y = 0.0;

    for (size_t i = 0; i < N_RESON; i++)
    {
        y += iirfilt_3_3_update(&tube->resonators[i], x);
    }

    return y / N_RESON;
}

chime_t *chime_init(void)
{
    chime_t *chime = &g_chime;
    chime->i = 0;
    trig_init(&chime->trig);
    chime->end = wind_init(&chime->trig.wind,
                           &(float[WIND_POINTS]){1.0 + get_rand_up(1.0),
                                                 2.0 + get_rand_up(2.0),
                                                 8.0 + get_rand_up(4.0),
                                                 2.0 + get_rand_up(3.0),
                                                 4.0 + get_rand_up(1.0)},
                           &(float[WIND_POINTS]){0.0, 2.0, 2.0, 0.0, 0.0});

    TUBES_INIT();

    return chime;
}

float chime_update(chime_t *chime)
{
    float y = 0.0;
    uint8_t s = trig_update(&chime->trig, chime->i);

    for (size_t i = 0; i < N_TUBES; i++)
    {
        y += tube_update(&chime->tubes[i], chime->i, s == (i + 1), trig_get_energy(&chime->trig));
    }
    chime->i++;

    return (y / N_TUBES) * 0.9;
}

bool chime_is_finished(chime_t *chime)
{
    return chime->i >= chime->end;
}
