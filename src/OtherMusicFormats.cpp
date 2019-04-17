#include "OtherMusicFormats.h"

#include <stdbool.h>
#include <stddef.h>

extern "C" {
#include "audio_lib/mixer.h"
}

#define MIN(a, b) (a < b ? a : b)

typedef struct Song
{
	Mixer_Sound *sound;
	Mixer_SoundInstanceID instance;
} Song;

static Song song;
static Song previous_song;
static bool playing;

void OtherMusic_Init(unsigned int sample_rate, unsigned int channel_count)
{
	Mixer_Init(sample_rate, channel_count);
}

void OtherMusic_Deinit(void)
{
	Mixer_Deinit();
}

void OtherMusic_Play(void)
{
	playing = true;
}

void OtherMusic_Stop(void)
{
	playing = false;
}

void OtherMusic_Load(const char *path, bool loop)
{
	if (previous_song.sound)
		Mixer_UnloadSound(previous_song.sound);

	Mixer_PauseSound(song.instance);
	previous_song = song;

	song.sound = Mixer_LoadSound(path, loop, false);
	song.instance = Mixer_PlaySound(song.sound);
	Mixer_UnpauseSound(song.instance);
}

void OtherMusic_LoadPrevious(void)
{
	if (song.sound)
		Mixer_UnloadSound(song.sound);

	song = previous_song;
	Mixer_UnpauseSound(song.instance);

	previous_song.sound = NULL;
}

void OtherMusic_Mix(float (*buffer)[2], unsigned long frames)
{
	if (playing)
	{
		unsigned long frames_done_total = 0;

		for (unsigned long frames_done; frames_done_total != frames; frames_done_total += frames_done)
		{
			float read_buffer[4096][2];

			frames_done = MIN(4096, frames - frames_done_total);

			Mixer_GetSamples(read_buffer, MIN(4096, frames_done));

			for (unsigned long i = 0; i < frames_done; ++i)
			{
				buffer[frames_done_total + i][0] += read_buffer[i][0];
				buffer[frames_done_total + i][1] += read_buffer[i][1];
			}

			if (frames_done < frames - frames_done_total)
				break;
		}
	}
}