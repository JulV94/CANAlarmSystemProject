#ifndef SPEECH_H
#define SPEECH_H

void decodeSoundFrame(int* buf, int size);

void sendToDac(int echant);

#endif
