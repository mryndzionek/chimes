import math
import numpy as np

import scipy.signal as sig
from scipy.fft import fft
import scipy.io.wavfile as wv

import matplotlib.pyplot as plt

fs, y = wv.read('wav/bowl.wav')
print("samplerate: {} Hz".format(fs))
y = 4 * y.astype(np.float32) / np.iinfo(type(y[0])).max
FFT_LEN = 16384
init_offset = 6500
Y = fft(y[init_offset:FFT_LEN+init_offset])

f = np.array(range(FFT_LEN))*fs/FFT_LEN
plt.figure(figsize=(20, 10))
plt.plot(f, 20*np.log10(np.abs(Y)), alpha=0.5, label="WAV spectrum")
plt.xlim(0, fs/2)
plt.grid(True)
plt.xlabel('Frequency (Hz)')
plt.ylabel('Magnitude (dB)')

Y = np.abs(Y[0:math.ceil(FFT_LEN/2)])
Y_filt = sig.filtfilt(np.ones(10) / 10, 1.0, Y)
plt.plot(f[0:math.ceil(FFT_LEN/2)], 20*np.log10(Y_filt), color="black",
         label="WAV spectrum (filtered)")

pk = sig.find_peaks(20*np.log10(Y_filt), height=10, distance=500)[0]
Yi = Y[pk]
Xi = f[pk]
plt.plot(Xi, 20*np.log10(Yi), marker="x",
         linestyle='None', color="red", linewidth=5, label="peaks (modes)")

K = len(Xi)

B = 0.2*np.array([0.5, 2.4, 2.0, 6.5, 5.5] + (K * [1.5]))[0:K]
r = np.exp(-math.pi*B/fs)
b0 = Yi
a1 = -2*r*np.cos(2.0*math.pi*Xi/fs)
a2 = r * r

filterbank = np.transpose(np.stack((b0, a1, a2)))
print(filterbank)

N = len(y)
x = np.concatenate((np.ones(1), np.zeros(N-1)))
y2 = sig.lfilter([b0[0]], [1.0, a1[0], a2[0]], x)
for n in range(1, K):
    y2 = y2 + sig.lfilter([b0[n]], [1.0, a1[n], a2[n]], x)
y2 = 0.5 * y2 / max(abs(y2))

Y2 = fft(y2[init_offset:FFT_LEN+init_offset])
plt.plot(f, 20*np.log10(np.abs(Y2)), label="synthesized spectrum")

x = (np.power(0.97, np.array(range(N)))) * \
    ((np.random.random(N) * 2) - 1)
y2 = sig.lfilter([b0[0]], [1.0, a1[0], a2[0]], x)
for n in range(1, K):
    y2 = y2 + sig.lfilter([b0[n]], [1.0, a1[n], a2[n]], x)
y2 = 0.5 * y2 / max(abs(y2))

Y3 = fft(y2[init_offset:FFT_LEN+init_offset])
plt.plot(f, 20*np.log10(np.abs(Y3)),
         label="synthesized spectrum (2)")
plt.legend()
plt.savefig("bowl_synth.svg", format="svg", dpi=1200, transparent=True)

wv.write("wav/bowl_synth.wav", fs, y2)
