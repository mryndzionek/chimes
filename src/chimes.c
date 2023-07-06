#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

#include <math.h>

#include <assert.h>

#include "chimes.h"

#define TS (1.0 / SAMPLERATE)
#define N_TUBES (5)
#define N_RESON (5)
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

typedef const float (*a_coefs_t)[3];
typedef const float (*b_coefs_t)[3];

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

static const float T1_B1[] = {0.019664537132491156, 0.0, -0.019664537132491156};
static const float T1_A1[] = {1.0, -1.9842670792615222, 0.9999373465861271};

static const float T1_B2[] = {0.12203082156949098, 0.0, -0.12203082156949098};
static const float T1_A2[] = {1.0, -1.8875886304768807, 0.9996420333614648};

static const float T1_B3[] = {1.1866909195812432, 0.0, -1.1866909195812432};
static const float T1_A3[] = {1.0, -1.6083394925589745, 0.9987476772777912};

static const float T1_B4[] = {0.02295521671812924, 0.0, -0.02295521671812924};
static const float T1_A4[] = {1.0, -1.0681952901977536, 0.997496922867783};

static const float T1_B5[] = {0.054340057577871186, 0.0, -0.054340057577871186};
static const float T1_A5[] = {1.0, -0.27466469574917346, 0.9950001111306961};

static const float T2_B1[] = {0.02197621974477674, 0.0, -0.02197621974477674};
static const float T2_A1[] = {1.0, -1.9803469856441656, 0.9999373465861271};

static const float T2_B2[] = {0.13534370179483976, 0.0, -0.13534370179483976};
static const float T2_A2[] = {1.0, -1.860851601091276, 0.9996420333614648};

static const float T2_B3[] = {1.2971192494894297, 0.0, -1.2971192494894297};
static const float T2_A3[] = {1.0, -1.5206817623094464, 0.9987476772777912};

static const float T2_B4[] = {0.024384620317483067, 0.0, -0.024384620317483067};
static const float T2_A4[] = {1.0, -0.8804493838245461, 0.997496922867783};

static const float T2_B5[] = {0.05485997351401727, 0.0, -0.05485997351401727};
static const float T2_A5[] = {1.0, 0.019157317500966536, 0.9950001111306961};

static const float T3_B1[] = {0.026239734450185515, 0.0, -0.026239734450185515};
static const float T3_A1[] = {1.0, -1.971949300806235, 0.9999373465861271};

static const float T3_B2[] = {0.15945261189524396, 0.0, -0.15945261189524396};
static const float T3_A2[] = {1.0, -1.8041292690642838, 0.9996420333614648};

static const float T3_B3[] = {1.48169510912585, 0.0, -1.48169510912585};
static const float T3_A3[] = {1.0, -1.3414809631742195, 0.9987476772777912};

static const float T3_B4[] = {0.026299156928883467, 0.0, -0.026299156928883467};
static const float T3_A4[] = {1.0, -0.5005595610243526, 0.997496922867783};

static const float T3_B5[] = {0.05244765936959565, 0.0, -0.05244765936959565};
static const float T3_A5[] = {1.0, 0.5853705649161809, 0.9950001111306961};

static const float T4_B1[] = {0.029567786387665298, 0.0, -0.029567786387665298};
static const float T4_A1[] = {1.0, -1.964331189345691, 0.9999373465861271};

static const float T4_B2[] = {0.17691919513192592, 0.0, -0.17691919513192592};
static const float T4_A2[] = {1.0, -1.755856135354962, 0.9996420333614648};

static const float T4_B3[] = {1.603015274255902, 0.0, -1.603015274255902};
static const float T4_A3[] = {1.0, -1.1938734222616882, 0.9987476772777912};

static const float T4_B4[] = {0.02702354343657772, 0.0, -0.02702354343657772};
static const float T4_A4[] = {1.0, -0.20426584766221142, 0.997496922867783};

static const float T4_B5[] = {0.047834192845185315, 0.0, -0.047834192845185315};
static const float T4_A5[] = {1.0, 0.976943929696704, 0.9950001111306961};

static const float T5_B1[] = {0.03260142915125318, 0.0, -0.03260142915125318};
static const float T5_A1[] = {1.0, -1.956565053014329, 0.9999373465861271};

static const float T5_B2[] = {0.19369044527902135, 0.0, -0.19369044527902135};
static const float T5_A2[] = {1.0, -1.7032981905450706, 0.9996420333614648};

static const float T5_B3[] = {1.7057805199744118, 0.0, -1.7057805199744118};
static const float T5_A3[] = {1.0, -1.0417795683547892, 0.9987476772777912};

static const float T5_B4[] = {0.027141311744702582, 0.0, -0.027141311744702582};
static const float T5_A4[] = {1.0, 0.08506806178318299, 0.997496922867783};

static const float T5_B5[] = {0.04133913259364663, 0.0, -0.04133913259364663};
static const float T5_A5[] = {1.0, 1.3115946695371703, 0.9950001111306961};

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
            float ps[N_TUBES + 1] = {1.0 - p, p / N_TUBES, p / N_TUBES, p / N_TUBES, p / N_TUBES, p / N_TUBES};
            uint8_t es[N_TUBES + 1] = {0, 1, 2, 3, 4, 5};
            trig->state = rnd_choice(es, ps, N_TUBES + 1);
        }
        else
        {
            float ps[3] = {1.0 - p, p / 2.0, p / 2.0};
            uint8_t es[3] = {
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
              ((*iirfilt->bs)[1] * iirfilt->xs[1]) +
              ((*iirfilt->bs)[2] * iirfilt->xs[0]) -
              ((*iirfilt->as)[1] * iirfilt->ys[1]) -
              ((*iirfilt->as)[2] * iirfilt->ys[0]);

    y /= (*iirfilt->as)[0];
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

    tube_init(&chime->tubes[0],
              &(b_coefs_t[N_RESON]){&T1_B1, &T1_B2, &T1_B3, &T1_B4, &T1_B5},
              &(a_coefs_t[N_RESON]){&T1_A1, &T1_A2, &T1_A3, &T1_A4, &T1_A5});

    tube_init(&chime->tubes[1],
              &(b_coefs_t[N_RESON]){&T2_B1, &T2_B2, &T2_B3, &T2_B4, &T2_B5},
              &(a_coefs_t[N_RESON]){&T2_A1, &T2_A2, &T2_A3, &T2_A4, &T2_A5});

    tube_init(&chime->tubes[2],
              &(b_coefs_t[N_RESON]){&T3_B1, &T3_B2, &T3_B3, &T3_B4, &T3_B5},
              &(a_coefs_t[N_RESON]){&T3_A1, &T3_A2, &T3_A3, &T3_A4, &T3_A5});

    tube_init(&chime->tubes[3],
              &(b_coefs_t[N_RESON]){&T4_B1, &T4_B2, &T4_B3, &T4_B4, &T4_B5},
              &(a_coefs_t[N_RESON]){&T4_A1, &T4_A2, &T4_A3, &T4_A4, &T4_A5});

    tube_init(&chime->tubes[4],
              &(b_coefs_t[N_RESON]){&T5_B1, &T5_B2, &T5_B3, &T5_B4, &T5_B5},
              &(a_coefs_t[N_RESON]){&T5_A1, &T5_A2, &T5_A3, &T5_A4, &T5_A5});

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
