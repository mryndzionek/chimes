#ifndef __CHIMES_CFG__
#define __CHIMES_CFG__

#define N_TUBES (5)
#define N_RESON (5)

typedef const float (*a_coefs_t)[3];
typedef const float (*b_coefs_t)[3];

static const float T1_B1[3] = { 0.019664537132491156, 0.0, -0.019664537132491156 };
static const float T1_A1[3] = { 1.0, -1.9842670792615222, 0.9999373465861271 };

static const float T1_B2[3] = { 0.12203082156949098, 0.0, -0.12203082156949098 };
static const float T1_A2[3] = { 1.0, -1.8875886304768807, 0.9996420333614648 };

static const float T1_B3[3] = { 1.1866909195812432, 0.0, -1.1866909195812432 };
static const float T1_A3[3] = { 1.0, -1.6083394925589745, 0.9987476772777912 };

static const float T1_B4[3] = { 0.02295521671812924, 0.0, -0.02295521671812924 };
static const float T1_A4[3] = { 1.0, -1.0681952901977536, 0.997496922867783 };

static const float T1_B5[3] = { 0.054340057577871186, 0.0, -0.054340057577871186 };
static const float T1_A5[3] = { 1.0, -0.27466469574917346, 0.9950001111306961 };

static const float T2_B1[3] = { 0.02197621974477674, 0.0, -0.02197621974477674 };
static const float T2_A1[3] = { 1.0, -1.9803469856441656, 0.9999373465861271 };

static const float T2_B2[3] = { 0.13534370179483976, 0.0, -0.13534370179483976 };
static const float T2_A2[3] = { 1.0, -1.860851601091276, 0.9996420333614648 };

static const float T2_B3[3] = { 1.2971192494894297, 0.0, -1.2971192494894297 };
static const float T2_A3[3] = { 1.0, -1.5206817623094464, 0.9987476772777912 };

static const float T2_B4[3] = { 0.024384620317483067, 0.0, -0.024384620317483067 };
static const float T2_A4[3] = { 1.0, -0.8804493838245461, 0.997496922867783 };

static const float T2_B5[3] = { 0.05485997351401727, 0.0, -0.05485997351401727 };
static const float T2_A5[3] = { 1.0, 0.019157317500966536, 0.9950001111306961 };

static const float T3_B1[3] = { 0.026239734450185515, 0.0, -0.026239734450185515 };
static const float T3_A1[3] = { 1.0, -1.971949300806235, 0.9999373465861271 };

static const float T3_B2[3] = { 0.15945261189524396, 0.0, -0.15945261189524396 };
static const float T3_A2[3] = { 1.0, -1.8041292690642838, 0.9996420333614648 };

static const float T3_B3[3] = { 1.48169510912585, 0.0, -1.48169510912585 };
static const float T3_A3[3] = { 1.0, -1.3414809631742195, 0.9987476772777912 };

static const float T3_B4[3] = { 0.026299156928883467, 0.0, -0.026299156928883467 };
static const float T3_A4[3] = { 1.0, -0.5005595610243526, 0.997496922867783 };

static const float T3_B5[3] = { 0.05244765936959565, 0.0, -0.05244765936959565 };
static const float T3_A5[3] = { 1.0, 0.5853705649161809, 0.9950001111306961 };

static const float T4_B1[3] = { 0.029567786387665298, 0.0, -0.029567786387665298 };
static const float T4_A1[3] = { 1.0, -1.964331189345691, 0.9999373465861271 };

static const float T4_B2[3] = { 0.17691919513192592, 0.0, -0.17691919513192592 };
static const float T4_A2[3] = { 1.0, -1.755856135354962, 0.9996420333614648 };

static const float T4_B3[3] = { 1.6030152742559023, 0.0, -1.6030152742559023 };
static const float T4_A3[3] = { 1.0, -1.193873422261688, 0.9987476772777912 };

static const float T4_B4[3] = { 0.02702354343657772, 0.0, -0.02702354343657772 };
static const float T4_A4[3] = { 1.0, -0.20426584766221142, 0.997496922867783 };

static const float T4_B5[3] = { 0.047834192845185315, 0.0, -0.047834192845185315 };
static const float T4_A5[3] = { 1.0, 0.976943929696704, 0.9950001111306961 };

static const float T5_B1[3] = { 0.03260142915125318, 0.0, -0.03260142915125318 };
static const float T5_A1[3] = { 1.0, -1.956565053014329, 0.9999373465861271 };

static const float T5_B2[3] = { 0.19369044527902135, 0.0, -0.19369044527902135 };
static const float T5_A2[3] = { 1.0, -1.7032981905450706, 0.9996420333614648 };

static const float T5_B3[3] = { 1.7057805199744118, 0.0, -1.7057805199744118 };
static const float T5_A3[3] = { 1.0, -1.0417795683547892, 0.9987476772777912 };

static const float T5_B4[3] = { 0.027141311744702582, 0.0, -0.027141311744702582 };
static const float T5_A4[3] = { 1.0, 0.08506806178318344, 0.997496922867783 };

static const float T5_B5[3] = { 0.04133913259364663, 0.0, -0.04133913259364663 };
static const float T5_A5[3] = { 1.0, 1.3115946695371703, 0.9950001111306961 };

#define TUBE_INIT(_x) \
    do \
    { \
        tube_init(&chime ->tubes[_x - 1], \
                  &(b_coefs_t[N_RESON]){&T##_x##_B1, &T##_x##_B2, &T##_x##_B3, &T##_x##_B4, &T##_x##_B5},  \
                  &(a_coefs_t[N_RESON]){&T##_x##_A1, &T##_x##_A2, &T##_x##_A3, &T##_x##_A4, &T##_x##_A5}); \
    } while (0)

#define TUBES_INIT() \
    do \
    { \
        TUBE_INIT(1); \
        TUBE_INIT(2); \
        TUBE_INIT(3); \
        TUBE_INIT(4); \
        TUBE_INIT(5); \
    } while (0)

#define TRIG_PROBS() \
    const float ps[N_TUBES + 1] = {1.0 - p, p / N_TUBES, p / N_TUBES, p / N_TUBES, p / N_TUBES, p / N_TUBES}; \
    const uint8_t es[N_TUBES + 1] = {0, 1, 2, 3, 4, 5}; \

#endif // __CHIMES_CFG__
