import math

import numpy as np
import matplotlib.pyplot as plt
import scipy.signal as sig
from scipy.io.wavfile import write as wav_write

# An attempt at recreating the results from the paper:
# "Modal Synthesis of Wind Chime Sounds with Stochastic
#  Event Triggering" by Teemu Lukkari and Vesa Välimäki

FS = 11025   # [Hz]
TS = 1 / FS  # [s]
END_TIME = 25.0  # [s]
N_BELLS = 5

T_60 = [40, 7, 2, 1, 0.5]
assert(len(T_60) == N_BELLS)
Rs = list(map(lambda t: math.pow(1e-6, 1 / (t * FS)), T_60))
Gs = [0.0787, 0.1849, 1.0000, 0.0136, 0.0275]

# modes for 5 tubes
MODES = [[219.8, 590.2, 1115.4, 1766.2, 2513.9],
         [245.8, 657.6, 1239.2, 1955.3, 2773.1],
         [293.9, 782.4, 1465.2, 2311.8, 3278.8],
         [331.6, 875.5, 1633.0, 2576.5, 3654.2],
         [366.1, 967.5, 1794.2, 2831.0, 4015.1]]

assert(len(MODES) == N_BELLS)
assert(len(Rs) == N_BELLS)
assert(len(Gs) == N_BELLS)


class Trigger:
    def __init__(self, n_bells):
        self.n_bells = n_bells
        self.s_low = round(0.03 / TS)
        self.s_high = round(0.05 / TS)
        self.next_s = 0
        self.state = 0

    def update(self, n):
        p = hit_prob(energy(n))
        if n == self.next_s:
            if self.state == 0:
                ps = [1 - p] + ([p / self.n_bells] * self.n_bells)
                self.state = np.random.choice(np.arange(0, self.n_bells + 1),
                                              p=ps)
            else:
                cs = [0, self.state - 1 if self.state > 1 else self.n_bells,
                      self.state + 1 if self.state < self.n_bells else 1]
                ps = [1 - p, p / 2, p / 2]
                self.state = np.random.choice(cs, p=ps)
            self.next_s = n + \
                np.random.choice(range(self.s_low, self.s_high + 1, 1))
        return self.state


class BurstGen:
    def __init__(self):
        self.R_input = 0.97
        self.E = 0.0
        self.start = 0
        self.n = 0
        self.active = False

    def update(self, n):
        self.n = n
        a = math.sqrt(self.E + 0.1)
        if self.active:
            if (n - self.start) <= (0.02 / TS):
                d = a * math.pow(self.R_input, n - self.start)
                s = np.random.uniform(-1, 1, 1)[0]
                return s * d
            else:
                self.active = False
                return 0.0
        else:
            return 0.0

    def reset(self, E):
        self.E = E
        self.start = self.n
        self.active = True


def wind(t):
    if t < 2:
        return 0.0
    elif t < 10:
        return 2 * (t - 2) / 8
    elif t < 20:
        return 2.0
    else:
        return 0.0


def energy(n):
    Rdec = 0.9999
    dn = TS * wind((n - 1) * TS)
    En = Rdec * (energy.En_prev + dn)
    energy.En_prev = En
    return En


def hit_prob(e):
    return 1.0 / (1 + (99 * math.exp(-2 * e)))


def resonator_coefs(modes):
    fs = []
    for i, f in enumerate(modes):
        R = Rs[i]
        G = Gs[i]
        cos_theta = math.cos(2 * math.pi * f / FS) * (2 * R) / (1 + (R * R))
        sin_theta = math.sqrt(1 - (cos_theta * cos_theta))
        A_zero = (1 + (R * R)) * sin_theta
        b_s = [A_zero * G, 0.0, -A_zero * G]
        a_s = [1.0, -2 * R * cos_theta, R * R]
        fs.append((b_s, a_s))
    return fs


time = np.arange(0, END_TIME, TS)
wnd_s = np.array(list(map(wind, time)))
energy.En_prev = 0
en = list(map(energy, range(len(time))))
h_prob = list(map(hit_prob, en))

trigger = Trigger(N_BELLS)
strikes = []
energy.En_prev = 0
for i in range(len(time)):
    s = trigger.update(i)
    strikes.append(s)

fig, (ax1, ax2, ax3, ax4) = plt.subplots(4)
ax1.grid(True)
ax2.grid(True)
ax3.grid(True)
ax4.grid(True)

ax1.plot(time, wnd_s)
ax2.plot(time, en)
ax3.plot(time, h_prob)
ax4.plot(time, strikes)
plt.xlabel("time [s]")
plt.show()

for i, m in enumerate(MODES):
    n = round(1.0 / TS)
    bg = BurstGen()
    bg.reset(0.2)
    brst = []
    for j in range(n):
        brst.append(bg.update(j))
    output = np.zeros(n)
    fs = resonator_coefs(m)

    for f in fs:
        output += sig.lfilter(f[0], f[1], brst)
    output /= len(fs)

    # save the synthesized tube samples
    wav_write("wav/tube_{}.wav".format(i + 1), FS, output)

chimes = np.zeros(len(strikes))

for n in range(N_BELLS):
    bell_n = list(map(lambda s: s == n + 1, strikes))
    bg = BurstGen()
    brsts = []
    prev_s = False
    for i, s in enumerate(bell_n):
        if s and (prev_s != s):
            bg.reset(en[i])
        brsts.append(bg.update(i))
        prev_s = s

    output = np.zeros(len(bell_n))
    fs = resonator_coefs(MODES[n])

    for i, f in enumerate(fs):
        output += sig.lfilter(f[0], f[1], brsts)
        # print("static const float T{}_B{}[{}] = {{ {} }};".format(
        #     n + 1, i + 1, len(f[0]), ", ".join(list(map(str, f[0])))))
        # print("static const float T{}_A{}[{}] = {{ {} }};".format(
        #     n + 1, i + 1, len(f[1]), ", ".join(list(map(str, f[1])))))
        # print("")

    chimes += output / (N_BELLS * len(fs))

wav_write("wav/chimes.wav", FS, chimes * 0.9)