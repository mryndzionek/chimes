#ifndef _CHIMES_H_
#define _CHIMES_H_

#define SAMPLERATE (11025)

typedef struct _chime_t chime_t;

chime_t *chime_init(void);
float chime_update(chime_t *chime);
bool chime_is_finished(chime_t *chime);

#endif // _CHIMES_H_
