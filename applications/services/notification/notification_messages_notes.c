#include "notification.h"

/*
Python script for note messages generation

# coding: utf-8
# Python script for note messages generation
from typing import List

note_names: List = ['c', 'cs', 'd', 'ds', 'e', 'f', 'fs', 'g', 'gs', 'a', 'as', 'b']
base_note: float = 16.3515979
cf: float = 2 ** (1.0 / 12)

note: float = base_note
for octave in range(9):
    for name in note_names:
        print(f"const NotificationMessage message_note_{name}{octave}" + " = {\n"
              "\t.type = NotificationMessageTypeSoundOn,\n"
              f"\t.data.sound.frequency = {round(note, 2)}f,\n"
              "\t.data.sound.volume = 1.0f,\n"
              "};")
        note = note * cf

for octave in range(9):
    for name in note_names:
        print(f"extern const NotificationMessage message_note_{name}{octave};")
*/

const NotificationMessage message_click = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_c0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 16.35f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_cs0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 17.32f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_d0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 18.35f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_ds0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 19.45f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_e0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 20.6f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_f0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 21.83f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_fs0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 23.12f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_g0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 24.5f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_gs0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 25.96f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_a0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 27.5f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_as0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 29.14f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_b0 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 30.87f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_c1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 32.7f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_cs1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 34.65f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_d1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 36.71f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_ds1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 38.89f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_e1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 41.2f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_f1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 43.65f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_fs1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 46.25f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_g1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 49.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_gs1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 51.91f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_a1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 55.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_as1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 58.27f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_b1 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 61.74f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_c2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 65.41f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_cs2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 69.3f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_d2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 73.42f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_ds2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 77.78f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_e2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 82.41f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_f2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 87.31f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_fs2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 92.5f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_g2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 98.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_gs2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 103.83f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_a2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 110.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_as2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 116.54f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_b2 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 123.47f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_c3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 130.81f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_cs3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 138.59f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_d3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 146.83f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_ds3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 155.56f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_e3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 164.81f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_f3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 174.61f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_fs3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 185.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_g3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 196.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_gs3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 207.65f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_a3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 220.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_as3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 233.08f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_b3 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 246.94f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_c4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 261.63f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_cs4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 277.18f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_d4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 293.66f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_ds4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 311.13f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_e4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 329.63f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_f4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 349.23f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_fs4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 369.99f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_g4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 392.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_gs4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 415.3f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_a4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 440.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_as4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 466.16f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_b4 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 493.88f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_c5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 523.25f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_cs5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 554.37f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_d5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 587.33f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_ds5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 622.25f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_e5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 659.26f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_f5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 698.46f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_fs5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 739.99f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_g5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 783.99f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_gs5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 830.61f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_a5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 880.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_as5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 932.33f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_b5 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 987.77f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_c6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1046.5f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_cs6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1108.73f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_d6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1174.66f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_ds6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1244.51f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_e6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1318.51f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_f6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1396.91f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_fs6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1479.98f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_g6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1567.98f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_gs6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1661.22f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_a6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1760.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_as6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1864.66f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_b6 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 1975.53f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_c7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 2093.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_cs7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 2217.46f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_d7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 2349.32f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_ds7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 2489.02f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_e7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 2637.02f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_f7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 2793.83f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_fs7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 2959.96f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_g7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 3135.96f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_gs7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 3322.44f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_a7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 3520.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_as7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 3729.31f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_b7 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 3951.07f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_c8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 4186.01f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_cs8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 4434.92f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_d8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 4698.64f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_ds8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 4978.03f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_e8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 5274.04f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_f8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 5587.65f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_fs8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 5919.91f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_g8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 6271.93f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_gs8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 6644.88f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_a8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 7040.0f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_as8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 7458.62f,
    .data.sound.volume = 1.0f,
};
const NotificationMessage message_note_b8 = {
    .type = NotificationMessageTypeSoundOn,
    .data.sound.frequency = 7902.13f,
    .data.sound.volume = 1.0f,
};
